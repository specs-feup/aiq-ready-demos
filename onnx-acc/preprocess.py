#!/usr/bin/env python3
"""
preprocess_make_features.py

Usage:
  python3 preprocess.py --data_dir dev_data/ToyCar --out_dir features_bin --train_dir dev_data/ToyCar/train

Produces:
 - features_bin/<audiofile>.bin   (640 int8 values)
 - features.csv                   (filename,bin_path,label)
 - stats.json                     (mean, std, scale)
Assumptions:
 - Training normal files are under <train_dir> (only used to compute mean/std).
 - Labels: filenames containing 'normal' -> 0, 'anomaly' -> 1
"""
import os
import argparse
import json
import numpy as np
import librosa
import csv
from tqdm import tqdm

SR = 16000
N_MELS = 64
N_FFT = 1024
HOP_LENGTH = 512
FRAMES = 10                # 64 * 10 = 640 features
EPS = 1e-8
QUANT_SCALE = 16.0         # multiply normalized z-scores by this before rounding to int8 (tunable)

def get_label_from_filename(fn):
    if 'normal' in fn:
        return 0
    if 'anomaly' in fn:
        return 1
    return -1

def extract_640_from_wav(path):
    # load, resample
    y, sr = librosa.load(path, sr=SR, mono=True)
    # compute mel spectrogram (power)
    S = librosa.feature.melspectrogram(y=y, sr=SR, n_fft=N_FFT, hop_length=HOP_LENGTH, n_mels=N_MELS, power=2.0)
    # convert to log-mel (dB)
    log_S = librosa.power_to_db(S, ref=np.max)  # shape (n_mels, n_frames)
    n_frames = log_S.shape[1]
    if n_frames >= FRAMES:
        # center crop the sequence of FRAMES frames
        start = (n_frames - FRAMES) // 2
        block = log_S[:, start:start+FRAMES]
    else:
        # pad with minimum value on time axis
        pad_width = FRAMES - n_frames
        block = np.pad(log_S, ((0,0),(0,pad_width)), mode='constant', constant_values=log_S.min())
    assert block.shape == (N_MELS, FRAMES)
    feats = block.flatten()   # length 640, dtype float
    return feats.astype(np.float32)

def gather_train_stats(train_dir):
    # compute mean and std across all training (normal) samples per-feature
    all_feats = []
    for root,_,files in os.walk(train_dir):
        for f in files:
            if not f.lower().endswith('.wav'):
                continue
            if 'normal' not in f:
                continue
            path = os.path.join(root, f)
            feats = extract_640_from_wav(path)
            all_feats.append(feats)
    if len(all_feats) == 0:
        raise RuntimeError("No training (normal) files found in {}".format(train_dir))
    arr = np.stack(all_feats, axis=0)  # (N, 640)
    mean = arr.mean(axis=0)
    std = arr.std(axis=0)
    # avoid zero std
    std[std < 1e-6] = 1.0
    return mean, std

def quantize_and_save(feats, mean, std, scale, out_path):
    # standardize
    z = (feats - mean) / std
    q = np.round(z * scale).astype(np.int32)
    q = np.clip(q, -127, 127).astype(np.int8)
    q.tofile(out_path)  # raw 640 bytes
    return q

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--data_dir', required=True, help='base dataset dir (with test folder)')
    p.add_argument('--out_dir', default='features_bin', help='output dir for .bin features')
    p.add_argument('--train_dir', required=True, help='directory with training normal wavs to compute stats')
    args = p.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)

    print("Computing mean/std on training normal set...")
    mean, std = gather_train_stats(args.train_dir)
    print("Mean/std computed.")

    mapping_rows = []
    # iterate test folder
    test_dir = os.path.join(args.data_dir, 'test')
    for root,_,files in os.walk(test_dir):
        for f in files:
            if not f.lower().endswith('.wav'):
                continue
            label = get_label_from_filename(f)
            if label == -1:
                continue
            full = os.path.join(root, f)
            feats = extract_640_from_wav(full)
            base = os.path.splitext(f)[0] + '.bin'
            out_path = os.path.join(args.out_dir, base)
            q = quantize_and_save(feats, mean, std, QUANT_SCALE, out_path)
            mapping_rows.append((f, out_path, label))

    with open('features.csv', 'w', newline='') as cf:
        w = csv.writer(cf)
        w.writerow(['filename','bin_path','label'])
        for r in mapping_rows:
            w.writerow(r)

    with open(os.path.join(args.out_dir, 'stats.txt'), 'w') as tf:
        tf.write(" ".join(f"{x:.6f}" for x in mean) + "\n")
        tf.write(" ".join(f"{x:.6f}" for x in std) + "\n")
        tf.write(str(QUANT_SCALE) + "\n")

    print("Done. Wrote {} feature files.".format(len(mapping_rows)))

if __name__ == '__main__':
    main()
