#!/bin/bash

echo "Testing normal sync vector with altered decoder using RF file..."

# Decode RF file directly (no offset processing needed)
result=$(./wspr-cui/wsprd-alt/wsprd -d -f 1.5 wspr_normal.rf | grep -v "<DecodeFinished>")

if [[ -n "$result" ]]; then
  echo "[ERROR] Decoded normal RF with altered wsprd:"
  echo "$result"
else
  echo "[OK] Correct: Normal sync vector could not be decoded with altered decoder"
fi

echo "done"


