#!/bin/bash

echo "Decoding normal sync vector with normal decoder using RF file..."

# Decode RF file directly (no offset processing needed)
result=$(./wspr-cui/wsprd/wsprd -d -f 1.5 wspr_normal.rf | grep -v "<DecodeFinished>")

if [[ -n "$result" ]]; then
  echo "[OK] Decoded:"
  echo "$result"
else
  echo "[FAIL] No decode"
fi

echo "done"

