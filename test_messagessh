#!/usr/bin/env bash
set -euo pipefail

# test_messages.sh: regenerate & sweep each test in turn

# list your test vectors here, one per line:
# format: CALL GRID POWER
read -r -d '' TESTS << 'EOF'
K1ABC FN31PR 10
K9XYZ EM73   15
N0CALL FN32   37
W1AW  FN42    30
EOF

# path to your sim + decoder
SIM=./wsprsim
DECODE=./wspr-cui/wsprd/wsprd

while read -r CALL GRID PWR; do
  echo
  echo "=== Testing $CALL $GRID $PWR ==="

  # 1) regenerate wav
  rm -f wspr_normal.wav temp.wav
  $SIM "$CALL" "$GRID" "$PWR"

  # 2) do the offset sweep
  for off in $(seq -1.0 0.1 1.0); do
    sox wspr_normal.wav temp.wav trim "$off" 110.592 2>/dev/null
    [[ -s temp.wav ]] || continue

    out=$($DECODE -d -f 14.0971 temp.wav | grep -v "<DecodeFinished>")
    if [[ -n "$out" ]]; then
      echo "Decoded at offset $off:"
      echo "  $out"
      break
    fi
  done

done <<< "$TESTS"

rm -f temp.wav

