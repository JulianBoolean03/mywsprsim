#!/usr/bin/env bash
for off in $(seq -1.0 0.1 1.0); do
  sox wspr_normal.wav temp.wav trim "$off" 110.592 2>/dev/null
  out=$(./wspr-cui/wsprd/wsprd -d -f 14.0971 temp.wav | grep -v "<DecodeFinished>")
  if [[ -n "$out" ]]; then
    echo "Decoded at offset $off:"
    echo "  $out"
    break
  fi
done

