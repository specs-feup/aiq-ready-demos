#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <dirent.h>

#include <math.h>
#include <float.h>

extern void entry(const int8_t input[1][640], int8_t output[1][640]);

typedef struct {
  char filename[512];
  int label; // 0 normal, 1 anomaly
  int8_t qfeat[640];
  float anomaly_score;
} Sample;

float compute_mse_floats(const float a[640], const float b[640]) {
  double s = 0.0;
  for (int i = 0; i < 640; i++) {
    double d = a[i] - b[i];
    s += d * d;
  }
  return (float)(s / 640.0);
}

int read_stats(const char *stats_path, float mean[640], float std[640], float *scale) {
  FILE *f = fopen(stats_path, "r");
  if (!f) {
    printf("Failed to read stats from %s\n", stats_path);
    return 1;
  }

  // read mean (first 640 floats)
  for (int i = 0; i < 640; i++) {
    if (fscanf(f, "%f", &mean[i]) != 1) {
      printf("Failed to read mean[%d]\n", i);
      return 1;
    }
  }

  // read std (next 640 floats)
  for (int i = 0; i < 640; i++) {
    if (fscanf(f, "%f", &std[i]) != 1) {
      printf("Failed to read std[%d]\n", i);
      return 1;
    }
  }

  // read scale (last float)
  if (fscanf(f, "%f", scale) != 1) {
    printf("Failed to read scale\n");
    return 1;
  }
  printf("DEBUG: scale = %f\n", *scale);

  fclose(f);
  return 0;
}

int load_bin_feature(const char *path, int8_t out[640]) {
  FILE *f = fopen(path, "rb");
  if (!f) return -1;
  size_t r = fread(out, 1, 640, f);
  fclose(f);
  if (r != 640) return -1;
  return 0;
}

int8_t get_label_from_filename(const char *filename) {
  if (strstr(filename, "normal") != NULL) return 0;
  if (strstr(filename, "anomaly") != NULL) return 1;
  return -1;
}

// Simple confusion matrix
typedef struct { int tp, tn, fp, fn; } CM;

void update_cm(CM *cm, int true_label, int pred) {
  if (true_label == 1 && pred == 1) cm->tp++;
  else if (true_label == 0 && pred == 0) cm->tn++;
  else if (true_label == 0 && pred == 1) cm->fp++;
  else if (true_label == 1 && pred == 0) cm->fn++;
}

void calc_metrics(CM *cm, float *accuracy, float *precision, float *recall, float *f1, float *specificity) {
  int total = cm->tp + cm->tn + cm->fp + cm->fn;
  if (total == 0) { *accuracy = *precision = *recall = *f1 = *specificity = 0.0f; return; }
  *accuracy = (float)(cm->tp + cm->tn) / (float)total;
  *precision = (cm->tp + cm->fp > 0) ? (float)cm->tp / (cm->tp + cm->fp) : 0.0f;
  *recall = (cm->tp + cm->fn > 0) ? (float)cm->tp / (cm->tp + cm->fn) : 0.0f;
  *f1 = (*precision + *recall > 0) ? 2.0f * (*precision) * (*recall) / (*precision + *recall) : 0.0f;
  *specificity = (cm->tn + cm->fp > 0) ? (float)cm->tn / (cm->tn + cm->fp) : 0.0f;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <features_csv> <stats_txt>\n", argv[0]);
    return 1;
  }
  const char *csv = argv[1];
  const char *stats = argv[2];

  float mean[640], std[640], scale;
  if (read_stats(stats, mean, std, &scale) != 0) {
    fprintf(stderr, "Failed to read stats from %s\n", stats);
    return 1;
  }

  // load mapping CSV
  FILE *f = fopen(csv, "r");
  if (!f) { perror("open csv"); return 1; }
  char line[1024];
  if (!fgets(line, sizeof(line), f)) { fclose(f); return 1; } // skip header

  int capacity = 4096;
  Sample *samples = malloc(capacity * sizeof(Sample));
  int ns = 0;
  while (fgets(line, sizeof(line), f)) {
    char filename[512], binpath[512];
    int label;
    if (sscanf(line, " %511[^,],%511[^,],%d", filename, binpath, &label) < 3) continue;
    if (ns >= capacity) { capacity *= 2; samples = realloc(samples, capacity * sizeof(Sample)); }
    strncpy(samples[ns].filename, binpath, sizeof(samples[ns].filename)-1);
    samples[ns].label = label;
    if (load_bin_feature(binpath, samples[ns].qfeat) != 0) {
      fprintf(stderr, "Warning: failed to load %s\n", binpath);
      continue;
    }
    ns++;
  }
  fclose(f);
  if (ns == 0) {
    fprintf(stderr, "No samples found in %s\n", csv);
    free(samples);
    return 1;
  }

  printf("Loaded %d samples\n", ns);

  float *scores = malloc(ns * sizeof(float));
  for (int i = 0; i < ns; i++) {
    int8_t input_q[640];
    int8_t output_q[640];
    memcpy(input_q, samples[i].qfeat, 640);

    entry((const int8_t (*)[640])&input_q, (int8_t (*)[640])&output_q);

    float in_f[640], out_f[640];
    for (int k = 0; k < 640; k++) {
      float z_in = ((float)input_q[k]) / scale;
      float z_out = ((float)output_q[k]) / scale;
      in_f[k] = z_in * std[k] + mean[k];
      out_f[k] = z_out * std[k] + mean[k];
    }

    float mse = compute_mse_floats(in_f, out_f);
    samples[i].anomaly_score = mse;
    scores[i] = mse;
  }

  // --- Score summary ---
  float min_n=FLT_MAX, max_n=-FLT_MAX, sum_n=0.0f;
  float min_a=FLT_MAX, max_a=-FLT_MAX, sum_a=0.0f;
  int cnt_n=0, cnt_a=0;
  for(int i=0;i<ns;i++){
    if(samples[i].label==0){ // normal
      if(samples[i].anomaly_score < min_n) min_n = samples[i].anomaly_score;
      if(samples[i].anomaly_score > max_n) max_n = samples[i].anomaly_score;
      sum_n += samples[i].anomaly_score; cnt_n++;
    } else { // anomaly
      if(samples[i].anomaly_score < min_a) min_a = samples[i].anomaly_score;
      if(samples[i].anomaly_score > max_a) max_a = samples[i].anomaly_score;
      sum_a += samples[i].anomaly_score; cnt_a++;
    }
  }
  printf("Normal:  count=%d, min=%.4f, max=%.4f, avg=%.4f\n", cnt_n, min_n, max_n, sum_n/cnt_n);
  printf("Anomaly: count=%d, min=%.4f, max=%.4f, avg=%.4f\n", cnt_a, min_a, max_a, sum_a/cnt_a);

  // Save anomaly scores csv
  FILE *out = fopen("anomaly_scores.csv", "w");
  if (out) {
    fprintf(out, "filename,anomaly_score,label\n");
    for (int i = 0; i < ns; i++) {
      fprintf(out, "%s,%.6f,%d\n", samples[i].filename, samples[i].anomaly_score, samples[i].label);
    }
    fclose(out);
    printf("Wrote anomaly_scores.csv\n");
  }

  // Find threshold
  float min_s = scores[0], max_s = scores[0];
  for (int i = 1; i < ns; i++) {
    if (scores[i] < min_s)
      min_s = scores[i];

    if (scores[i] > max_s)
      max_s = scores[i];
  }

  float best_thr = min_s;
  float best_f1 = 0.0f;

  for (int step = 0; step <= 100; step++) {
    float thr = min_s + (max_s - min_s) * (float)step / 100.0f;
    CM cm = { 0, 0, 0, 0 };

    for (int i = 0; i < ns; i++) {
      int pred = (samples[i].anomaly_score > thr) ? 1 : 0;
      update_cm(&cm, samples[i].label, pred);
    }

    float acc, prec, rec, f1, spec;
    calc_metrics(&cm, &acc, &prec, &rec, &f1, &spec);

    if (f1 > best_f1) {
      best_f1 = f1;
      best_thr = thr;
    }
  }

  // Final metrics
  // CM cm = { 0, 0, 0, 0 };
  // for (int i = 0; i < ns; i++) {
  //   int pred = (samples[i].anomaly_score > best_thr) ? 1 : 0;
  //   update_cm(&cm, samples[i].label, pred);
  // }
// 
//   float acc, prec, rec, f1, spec;
//   calc_metrics(&cm, &acc, &prec, &rec, &f1, &spec);
// 
//   printf("\n=== Classification Metrics (Threshold: %.6f) ===\n", best_thr);
//   printf("Confusion Matrix:\n");
//   printf("     Predicted\n");
//   printf("     Normal  Anomaly\n");
//   printf("Actual Normal  %4d  %4d\n", cm.tn, cm.fp);
//   printf("   Anomaly   %4d  %4d\n", cm.fn, cm.tp);
//   printf("\nMetrics:\n");
//   printf("  Accuracy:  %.4f (%.2f%%)\n", acc, acc*100.0f);
//   printf("  Precision:   %.4f\n", prec);
//   printf("  Recall:  %.4f\n", rec);
//   printf("  F1-Score:  %.4f\n", f1);
//   printf("  Specificity: %.4f\n", spec);
//   printf("==============================\n");

  free(samples);
  free(scores);
  return 0;
}
