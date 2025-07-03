#!/usr/bin/env bash
for f in wspr_normal.wav wspr_altered.wav; do
  echo
  echo "=== Decoding $f ==="
  # trim to remove the 0.6s lead‐in you measured
  sox "$f" temp.wav trim -0.6 110.592 2>/dev/null
  # call the local wsprd (either ./wspr-cui/wsprd or just wsprd if you installed it)
  ./wspr-cui/build/bin/wsprd -r 1400:1440 -f temp.wav

done
rm temp.wav


