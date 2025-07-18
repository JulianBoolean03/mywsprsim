#!/usr/bin/env python3
import numpy as np
from scipy.io import wavfile
import matplotlib.pyplot as plt
import os

# Load WSPR audio file
fs, data = wavfile.read(os.path.expanduser('~/Desktop/wspr_normal.wav'))

# Use first second of data
segment = data[:fs]

# Apply window and compute FFT
w = segment * np.hanning(len(segment))
fft = np.fft.rfft(w)
freqs = np.fft.rfftfreq(len(w), 1/fs)

# Restrict to 1400–1500 Hz
mask = (freqs >= 1400) & (freqs <= 1500)

# Plot and save
plt.plot(freqs[mask], 20 * np.log10(np.abs(fft[mask])))
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude (dB)')
plt.title('WSPR tones in 1400–1500 Hz')
plt.tight_layout()
plt.savefig('wspr_normal_spectrum.png')
print('Saved spectrum plot to wspr_normal_spectrum.png')

