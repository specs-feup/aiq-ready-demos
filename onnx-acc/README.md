# Accelaration of Heavy Nodes in a ONNX Model Demo

This document describes how to *accelerate heavy nodes in a ONNX model* on a simple TinyML model:

- Run `onnx2c` on the given `ad01_int8.onnx`
- Run `gprof` on the resulting C file
- Choose the nodes you want to accelerate

---

## 1. Requirements

- onnx2c
- gprof
- python3
    - librosa, numpy, tqdm
- data to run your model (so in this case, the test set for this TinyML model)
- Terminal / shell:
  - Linux/macOS: bash or zsh

---

## ONNX2C

Install onnx2c:

Make sure you have ProtocolBuffers libraries installed, e.g.:
    - Ubuntu: `apt install libprotobuf-dev protobuf-compiler`
    - MacOS:  `brew install protobuf`

```
git clone https://github.com/kraiskil/onnx2c.git
cd onnx2c
git submodule update --init
```

Building:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make onnx2c
```

Usage:
```
./onnx2c [your ONNX model file] > model.c
```

So for this demo, we will do:
```
./onnx2c ad01_int8.onnx > ad01_int8.c
```

---

## Benchmarking

Let's install the dataset for benchmarking first.

```
chmod +x get_dataset.sh
./get_dataset.sh
```

To get `gprof`:
```
sudo apt update
sudo apt install binutils
which gprof
gprof --version
```

Preprocess the dataset data:
    - Note, we could not find how the preprocessing really occured, so this may be wrong however, the
    performance of the network, which is what we want to profile, is not affected by this, only the accuracy
    of the model
```
pip install librosa numpy tqdm
python3 preprocess.py --data_dir dev_data/ToyCar --out_dir features_bin --train_dir dev_data/ToyCar/train
```

Now we compile the benchmark file.
```
gcc -pg -o benchmark benchmark.c ad01_int8.c -lm
./benchmark features.csv features_bin/stats.txt
```
The benchmark file does not have the quantization parameters, so the statistical results are not reliable,
however we just wanted to test the model's nodes performances here.

Now we can run the benchmark with `gprof` to look at the heavy nodes.
```
gprof benchmark gmon.out
```
or
```
gprof benchmark gmon.out > analysis.txt
```
