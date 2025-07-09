#!/bin/bash

echo "Testing altered sync vector with normal decoder using RF file..."

# Decode RF file directly (no offset processing needed)
result=$(./wspr-cui/wsprd/wsprd -d -f 1.5 wspr_altered.rf | grep -v "<DecodeFinished>")

if [[ -n "$result" ]]; then
  echo "[ERROR] Decoded altered RF with normal wsprd:"
  echo "$result"
else
  echo "[OK] Correct: Altered sync vector could not be decoded with normal decoder"
fi

echo "done"
