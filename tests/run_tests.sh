#!/bin/bash

set -e

EXPR_DIR="tests/cases"
ANS_DIR="tests/answers"
OUT_DIR="tests/outputs"
BIN="./do"

mkdir -p "$OUT_DIR"

pass=0
fail=0

for expr_file in "$EXPR_DIR"/*.txt; do
    base=$(basename "$expr_file" .txt)
    ans_file="$ANS_DIR/$base.ans"
    out_file="$OUT_DIR/$base.out"

    if [[ ! -f "$ans_file" ]]; then
        echo "Skipping $base — missing expected answer"
        continue
    fi

    $BIN eval "$expr_file" "$out_file"

    actual_rounded=$(printf "%.1f" "$actual")
    expected_rounded=$(printf "%.1f" "$expected")

    if [[ "$actual_rounded" == "$expected_rounded" ]]; then
        echo "PASS: $base"
        ((pass++))
    else
        echo "FAIL: $base"
        echo "Expected: $expected_rounded"
        echo "Got     : $actual_rounded"
        ((fail++))
    fi
done

echo "Total: $((pass + fail)) | Passed: $pass | Failed: $fail"
