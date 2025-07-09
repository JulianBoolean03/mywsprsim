#!/bin/bash

echo "Decoding altered sync vector with altered decoder using RF file..."

# Decode RF file directly (no offset processing needed)
result=$(./wspr-cui/wsprd-alt/wsprd -d -f 1.5 wspr_altered.rf | grep -v "<DecodeFinished>")

if [[ -n "$result" ]]; then
  echo "[OK] Decoded:"
  echo "$result"
else
  echo "[FAIL] No decode"
fi

echo "done"

