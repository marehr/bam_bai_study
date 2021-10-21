#!/bin/env bash

set -x

mkdir bin
mkdir data

# clean
rm bin/*
rm data/*

# convert sam to bam
samtools view -bS example_bad.sam > data/example_bad.bam

# un-bgzf bam file to look at it in kaitai.io:
bgzip -c -d data/example_bad.bam > data/example_bad.bam.uncompressed

# create bai index
samtools index -b data/example_bad.bam

# bam_bgzf_split
g++ -std=c++2a bam_bgzf_split.cpp -o bin/bam_bgzf_split
g++ -std=c++2a bai_bin.cpp -o bin/bai_bin
g++ -std=c++2a bai_viewer.cpp -o bin/bai_viewer

# split bgzf blocks
./bin/bam_bgzf_split data/example_bad.bam

# unzip bgzf blocks
gunzip data/example_bad.bam.*.gz

# show bai file
./bin/bai_viewer data/example_bad.bam.bai

# octave -i example_bad.m
