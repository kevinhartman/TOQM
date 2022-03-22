#!/bin/bash

MAPPER_EXECUTABLE="$1"

if [ -z "$MAPPER_EXECUTABLE" ]; then
    echo -e "\nPlease call '$0 <path to mapper executable>' to run this command!\n"
    exit 1
fi

TEST_DIR="$(git rev-parse --show-toplevel)/test"
COUPLINGS_DIR="$TEST_DIR/data/couplings"
SMALL_CIRCUITS_DIR="$TEST_DIR/data/circuits/small"
OSLQ_CIRCUITS_DIR="$TEST_DIR/data/circuits/OLSQ"
LARGE_CIRCUITS_DIR="$TEST_DIR/data/circuits/large"

EXPECTED_SMALL_QX2_DIR="$TEST_DIR/data/expected_output/small_qx2"
EXPECTED_LARGE_TOKYO_DIR="$TEST_DIR/data/expected_output/large_tokyo"

# Generate small QX2
for f in $SMALL_CIRCUITS_DIR/*
do
  echo "Processing $f..."
  "$MAPPER_EXECUTABLE" "$f" "$COUPLINGS_DIR/qx2.txt" \
      -defaults \
      -latency Latency_1_2_6 \
      -filter HashFilter \
      -filter HashFilter2 \
      -pureSwapDiameter \
      > "$EXPECTED_SMALL_QX2_DIR/$(basename $f .qasm)_expected.qasm"
done

# Generate large Tokyo
for f in $LARGE_CIRCUITS_DIR/*
do
  echo "Processing $f..."
  SECONDS=0

  "$MAPPER_EXECUTABLE" "$f" "$COUPLINGS_DIR/tokyo.txt" \
      -defaults \
      -latency Latency_1_2_6 \
      -expander GreedyTopK 10 \
      -queue TrimSlowNodes 2000 1000 \
      -nodeMod GreedyMapper \
      -retain 1 \
      > "$EXPECTED_LARGE_TOKYO_DIR/$(basename $f .qasm)_expected.qasm"

  echo "Done in $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec."
done

