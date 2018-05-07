#!/bin/bash

THREADS=4
JOBS=10
EXEC=./bmi_cli
RESULT_DIR=results
DOC_FEATURES=../data/athome1.bin
QREL=../data/athome1.qrels
QUERIES=../data/athome1.query
MAXEFFORT=0
MAXEFFORTFACTOR=3.1
TRAINITERATIONS=200000

# BMI
echo "Starting BMI..."
SUBRESULT_DIR=$RESULT_DIR/bmi/
mkdir -p "$SUBRESULT_DIR"

./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
    --threads $THREADS --jobs $JOBS --training-iterations "$TRAINITERATIONS" \
    --mode BMI_DOC --max-effort "$MAXEFFORT" --max-effort-factor "$MAXEFFORTFACTOR" &> "$SUBRESULT_DIR/log"



# BMI_STATIC

while read k; do
    echo "Starting BMI_STATIC ($k)..."
    SUBRESULT_DIR=$RESULT_DIR/static_batch/$k
    mkdir -p "$SUBRESULT_DIR"

    ./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
        --threads $THREADS --jobs $JOBS \
        --mode BMI_DOC --max-effort "$MAXEFFORT" --max-effort-factor "$MAXEFFORTFACTOR" \
        --judgments-per-iteration $k --training-iterations "$TRAINITERATIONS" &> "$SUBRESULT_DIR/log"
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
        --mode BMI_PARTIAL_RANKING --max-effort "$MAXEFFORT" --max-effort-factor "$MAXEFFORTFACTOR" \
        --judgments-per-iteration 1 --training-iterations "$TRAINITERATIONS" \
        --partial-ranking-refresh-period $k --partial-ranking-subset-size $s &> "$SUBRESULT_DIR/log"
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
        --mode BMI_PRECISION_DELAY --max-effort "$MAXEFFORT" --max-effort-factor "$MAXEFFORTFACTOR" \
        --judgments-per-iteration 1 --training-iterations "$TRAINITERATIONS" \
        --precision-delay-threshold $p --precision-delay-window $k &> "$SUBRESULT_DIR/log"
done <<EOF
0.4 25
0.6 25
0.8 25
1.0 25
EOF


# TRAINING
while read it; do
    echo "Starting BMI_TRAINING (it=$it)..."
    SUBRESULT_DIR=$RESULT_DIR/training/it=$it
    mkdir -p "$SUBRESULT_DIR"

    ./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
        --threads $THREADS --jobs $JOBS \
        --mode BMI_DOC --max-effort "$MAXEFFORT" --max-effort-factor "$MAXEFFORTFACTOR" \
        --judgments-per-iteration 1 --training-iterations $it &> "$SUBRESULT_DIR/log"
done <<EOF
200
2000
20000
200000
EOF


# RECENCY_WEIGHTING
while read k it; do
    echo "Starting BMI_RECENCY_WEIGHTING (k=$k,it=$it)..."
    SUBRESULT_DIR=$RESULT_DIR/recency/k=$k,it=$it
    mkdir -p "$SUBRESULT_DIR"

    ./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
        --threads $THREADS --jobs $JOBS \
        --mode BMI_RECENCY_WEIGHTING --max-effort "$MAXEFFORT" --max-effort-factor "$MAXEFFORTFACTOR" \
        --judgments-per-iteration 1 \
        --recency-weighting-param $k --training-iterations $it &> "$SUBRESULT_DIR/log"
done <<EOF
2 2000
5 2000
10 2000
EOF

# BMI_FORGET
while read n; do
    echo "Starting BMI_FORGET (n=$n,k=inf)..."
    SUBRESULT_DIR=$RESULT_DIR/forget/n=$n,k=inf
    mkdir -p "$SUBRESULT_DIR"

    ./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
        --threads $THREADS --jobs $JOBS --training-iterations "$TRAINITERATIONS" \
        --mode BMI_RECENCY_WEIGHTING --max-effort "$MAXEFFORT" --max-effort-factor "$MAXEFFORTFACTOR" \
        --judgments-per-iteration 1 --forget-remember-count $n &> "$SUBRESULT_DIR/log"
done <<EOF
25
100
500
EOF

while read n k; do
    echo "Starting BMI_FORGET (n=$n,k=$k)..."
    SUBRESULT_DIR=$RESULT_DIR/forget/n=$n,k=$k
    mkdir -p "$SUBRESULT_DIR"

    ./bmi_cli --doc-features "$DOC_FEATURES" --judgment-logpath "$SUBRESULT_DIR" --qrel "$QREL" --query "$QUERIES" \
        --threads $THREADS --jobs $JOBS \
        --mode BMI_RECENCY_WEIGHTING --max-effort "$MAXEFFORT" --max-effort-factor "$MAXEFFORTFACTOR"\
        --judgments-per-iteration 1 --training-iterations "$TRAINITERATIONS" \
        --forget-remember-count $n --forget-refresh-period $k &> "$SUBRESULT_DIR/log"
done <<EOF
100 10
100 50
100 100
EOF
