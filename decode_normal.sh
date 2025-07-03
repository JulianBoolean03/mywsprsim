#!/usr/bin/env bash
#
# Sweep wspr_normal.wav (or wspr_altered.wav) through offsets until wsprd decodes,
# then stop immediately.

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <normal|altered> [start_offset end_offset step]"
  exit 2
fi

MODE="$1"
case "$MODE" in
  normal)  WAV=wspr_normal.wav ;;
  altered) WAV=wspr_altered.wav ;;
  *)
    echo "Error: first arg must be 'normal' or 'altered'"
    exit 2
    ;;
esac

# Optional override of sweep range
START=${2:--2.0}
END  ${3:=4.0}
STEP ${4:=0.1}

echo "─── Sweeping offsets for '$MODE' signal ($WAV) ───"
for offset in $(seq "$START" "$STEP" "$END"); do
  echo "--- Trying offset ${offset}s ---"
  length=$(echo "110.592 - $offset" | bc -l)

  # Trim down to exactly 110.592 s starting at $offset
  sox "$WAV" temp.wav trim "$offset" "$length" 2>/dev/null
  [[ -s temp.wav ]] || { echo "⚠️  sox failed at offset $offset"; continue; }

  # Decode with wsprd (deep search), filter out the <DecodeFinished> marker
  result=$(./wspr-cui/wsprd/wsprd -d -f 14.097100 temp.wav \
           | grep -v "<DecodeFinished>")

  if [[ -n "$result" ]]; then
    echo "✅ Decoded at offset ${offset}s:"
    echo "$result"
    rm temp.wav
    exit 0
  else
    echo "no decode"
  fi
done

echo "❌ No decode found in range."
rm -f temp.wav
exit 1

