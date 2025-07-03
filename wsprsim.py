#!/usr/bin/env python3
"""
Pure-Python WSPR simulator: encodes, corrupts sync, and writes WAV/BIT files.
Requires: pip install wsprpy numpy
"""

import argparse
import numpy as np
import wave
import wsprpy  # pip install wsprpy

# Constants
SAMPLE_RATE    = 48000
SYMBOL_RATE    = 162.0 / 110.612   # ≈1.464817 symbols/sec
BIT_PERIOD     = 1.0 / SYMBOL_RATE
CENTER_FREQ    = 1420.0            # Center frequency (Hz)
FREQ_OFFSETS   = np.array([-2.197265625, -0.732421875, 0.732421875, 2.197265625])
WSPR_SYM_COUNT = 162

def write_wav(path, samples):
    """Write 16-bit PCM WAV (mono)."""
    with wave.open(path, 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SAMPLE_RATE)
        w.writeframes(samples.astype(np.int16).tobytes())

def write_bits(path, syms):
    """Save raw symbol bytes."""
    with open(path, 'wb') as f:
        f.write(bytes(syms.tolist()))

def gen_audio(symbols):
    """Generate PCM samples for symbol sequence."""
    spb = int(round(BIT_PERIOD * SAMPLE_RATE))
    total = np.zeros(len(symbols)*spb, dtype=np.int16)
    for i, sym in enumerate(symbols):
        if sym < 0 or sym > 3:
            raise ValueError(f"Invalid symbol value: {sym}")
        freq = CENTER_FREQ + FREQ_OFFSETS[sym]
        t = (np.arange(spb) + i*spb) / SAMPLE_RATE
        wave_data = np.sin(2*np.pi*freq*t) * 32767
        total[i*spb:(i+1)*spb] = wave_data.astype(np.int16)
    return total

def main():
    parser = argparse.ArgumentParser(description="Pure-Python WSPR simulator")
    parser.add_argument("callsign", help="WSPR callsign")
    parser.add_argument("grid",     help="Maidenhead grid locator")
    parser.add_argument("power",    type=int, help="Power in dBm (0–60)")
    args = parser.parse_args()

    if not (0 <= args.power <= 60):
        parser.error("Power must be 0–60 dBm")

    # 1) Encode normal message
    msg = wsprpy.WSPRMessage(args.callsign, args.grid, args.power)
    normal_syms = np.array(msg.symbols, dtype=np.uint8)

    # 2) Invert sync bit (LSB)
    altered_syms = normal_syms ^ 0b01

    # 3) Bit difference report
    delta = normal_syms ^ altered_syms
    bit_diff = int(((delta & 1) != 0).sum() + (((delta >> 1) & 1) != 0).sum())
    print(f"Bit differences: {bit_diff} / {WSPR_SYM_COUNT*2} "
          f"({100*bit_diff/(WSPR_SYM_COUNT*2):.1f}%)")

    # 4) Write bits and WAVs
    write_bits("wspr_normal.bits", normal_syms)
    normal_audio = gen_audio(normal_syms)
    write_wav("wspr_normal.wav", normal_audio)
    print("→ wspr_normal.wav")

    write_bits("wspr_altered.bits", altered_syms)
    altered_audio = gen_audio(altered_syms)
    write_wav("wspr_altered.wav", altered_audio)
    print("→ wspr_altered.wav")

    print("\nSimulation complete. You now have:")
    print(" - wspr_normal.bits & wspr_normal.wav (should decode)")
    print(" - wspr_altered.bits & wspr_altered.wav (should NOT decode)")

if __name__ == "__main__":
    main()

