#!/bin/sh

# DCASE 2020 Task 2 - ToyCar dataset download script

URL1="https://zenodo.org/records/3678171/files/dev_data_ToyCar.zip?download=1"
ZIPFILE="dev_data_ToyCar.zip"
URL2="https://zenodo.org/records/3727685/files/eval_data_train_ToyCar.zip?download=1"

mkdir -p dev_data

echo "Downloading development dataset..."
curl -L $URL1 -o $ZIPFILE || wget $URL1 -O $ZIPFILE

echo "Extracting development dataset..."
unzip -q $ZIPFILE -d dev_data
rm $ZIPFILE

echo "Downloading additional training dataset..."
curl -L $URL2 -o $ZIPFILE || wget $URL2 -O $ZIPFILE

echo "Extracting additional training dataset..."
unzip -q $ZIPFILE -d dev_data
rm $ZIPFILE

echo "Dataset download complete!"
echo "Files are located in: ./dev_data"
