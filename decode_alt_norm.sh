#!/bin/bash

echo "Trying offsets from -2.0s to 4.0s in 0.1s steps..."

for offset in $(seq -2 0.1 4.0); do
  echo "--- Trying offset: ${offset}s ---"
  length=$(echo "110.592 - $offset" | bc -l)
  sox wspr_altered.wav temp.wav trim "$offset" "$length" 2>/dev/null

  # capture wsprd's output (minus the <DecodeFinished> line)
  result=$(./wspr-cui/wsprd/wsprd -d -f 1.5 temp.wav | grep -v "<DecodeFinished>")

  if [[ -n "$result" ]]; then
    echo "Error!! Decoded altered wav with normal wsprd at offset ${offset}:"
    echo "$result"
    break
  else
    echo "Normal could not be decoded at this offset using altered decoder"
  fi
done

echo "done"


