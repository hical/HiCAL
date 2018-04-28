#!/bin/bash

THREADS=4
JOBS=10
EXEC=./bmi_cli
RESULT_DIR=results
DOC_FEATURES=../data/athome1.bin
QREL=../data/athome1.qrels
QUERIES=../data/athome1.query
LOG=./log
MAXEFFORT=60000

# BMI
echo "Starting BMI..."
SUBRESULT_DIR=$RESULT_DIR/bmi/
mkdir -p "$SUBRESULT_DIR"

./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
    --threads $THREADS --jobs $JOBS \
    --mode BMI_DOC &> "$LOG"



# BMI_STATIC

while read k; do
    echo "Starting BMI_STATIC ($k)..."
    SUBRESULT_DIR=$RESULT_DIR/static_batch/$k
    mkdir -p "$SUBRESULT_DIR"

    ./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
        --threads $THREADS --jobs $JOBS \
        --mode BMI_DOC --max-effort "$MAXEFFORT" --judgments-per-iteration $k &> "$LOG"
done <<EOF
1
100
EOF



# BMI_PARTIAL_RANKING
while read k s; do
    echo "Starting BMI_PARTIAL_RANKING (k=$k,s=$s)..."
    SUBRESULT_DIR=$RESULT_DIR/partial_batch/k=$k,s=$s
    mkdir -p "$SUBRESULT_DIR"

    ./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
        --threads $THREADS --jobs $JOBS \
        --mode BMI_PARTIAL_RANKING --max-effort "$MAXEFFORT" --judgments-per-iteration 1 \
        --partial-ranking-refresh-period $k --partial-ranking-subset-size $s &> "$LOG"
done <<EOF
100 1000
500 1000
10 1000
100 5000
100 500
100 100
EOF


# BMI_PRECISION_DELAY
while read p k; do
    echo "Starting BMI_PRECISION_DELAY (p=$p,k=$k)..."
    SUBRESULT_DIR=$RESULT_DIR/precision/p=$p,k=$k
    mkdir -p "$SUBRESULT_DIR"

    ./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
        --threads $THREADS --jobs $JOBS \
        --mode BMI_PRECISION_DELAY --max-effort "$MAXEFFORT" --judgments-per-iteration 1 \
        --precision-delay-threshold $p --precision-delay-window $k &> "$LOG"
done <<EOF
0.4 25
0.6 25
0.8 25
1.0 25
EOF


# TRAINING
# TODO

# RECENCY_WEIGHTING
# TODO

# BMI_FORGET
# TODO
