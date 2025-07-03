#!/usr/bin/env bash
set -euo pipefail

if [ $# -ne 3 ]; then
  echo "Usage: $0 <wav_file> <freq_MHz> <offset_s>"
  exit 1
fi

WAV="$1"
FREQ="$2"
OFF="$3"
WSPRD=./wspr-cui/wsprd/wsprd

# 1) chop off the lead-in
sox "$WAV" temp.wav trim "$OFF" 110.592 2>/dev/null

# 2) decode once
"$WSPRD" -d -s -q -f "$FREQ" temp.wav | grep -v "<DecodeFinished>"

rm temp.wav

