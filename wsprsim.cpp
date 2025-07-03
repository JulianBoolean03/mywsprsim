// wsprsim.cpp
//
// Build (assuming JTEncode built into libjtencode.a and JTEncode.h in ../JTEncode/src):
//   g++ wspr_sim.cpp -I../JTEncode/src -L../JTEncode -ljtencode -o wspr_sim
//
// Usage:
//   ./wspr_sim KJ6ABC FN31pr 37
//
// Outputs:
//   wspr_normal.bits    (162 bytes: raw 0/1 symbols)
//   wspr_normal.wav     (48 kHz PCM, 1400/1440 Hz tones)
//   wspr_altered.bits   (162 bytes: inverted symbols)
//   wspr_altered.wav    (same format, inverted sync)

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "src/JTEncode.h"

// Audio parameters (matching wsprsimwav.c)
const int    SAMPLE_RATE   = 48000;             // 48kHz sampling rate
const int    SYMBOL_LENGTH = 32768;             // samples per symbol
const double CENTER_FREQ   = 1500.0;           // center frequency (Hz)
const double FREQ_SPACING  = 48000.0 / 32768;  // = 1.46484375 Hz spacing
const int    DELAY_SAMPLES = 48000;             // 1 second delay

// Write 16-bit PCM WAV (mono)
void write_wav(const char *fn, const std::vector<int16_t>& samples) {
    std::ofstream w(fn, std::ios::binary);
    // RIFF header
    w.write("RIFF",4);
    uint32_t fileSize = 36 + samples.size()*2;
    w.write(reinterpret_cast<const char*>(&fileSize),4);
    w.write("WAVE",4);
    // fmt chunk
    w.write("fmt ",4);
    uint32_t fmtLen = 16; w.write(reinterpret_cast<const char*>(&fmtLen),4);
    uint16_t pcm = 1, mono = 1; w.write(reinterpret_cast<const char*>(&pcm),2);
    w.write(reinterpret_cast<const char*>(&mono),2);
    w.write(reinterpret_cast<const char*>(&SAMPLE_RATE),4);
    uint32_t byteRate = SAMPLE_RATE * 2; w.write(reinterpret_cast<const char*>(&byteRate),4);
    uint16_t blockAlign = 2, bitsPerSample = 16;
    w.write(reinterpret_cast<const char*>(&blockAlign),2);
    w.write(reinterpret_cast<const char*>(&bitsPerSample),2);
    // data chunk
    w.write("data",4);
    uint32_t dataBytes = samples.size() * 2;
    w.write(reinterpret_cast<const char*>(&dataBytes),4);
    // samples
    w.write(reinterpret_cast<const char*>(samples.data()), dataBytes);
}

// Save raw 0/1 bytes
void write_bits(const char *fn, const uint8_t *syms) {
    std::ofstream b(fn, std::ios::binary);
    b.write(reinterpret_cast<const char*>(syms), WSPR_SYMBOL_COUNT);
}

int main(int argc, char** argv) {
      if(argc != 4) {
        std::fprintf(stderr, "Usage: %s CALLSIGN GRID POWER_dBm\n", argv[0]);
        return 1;
    }
    const char* call = argv[1];
    const char* grid = argv[2];
    int dbm = std::atoi(argv[3]);
    if(dbm < 0 || dbm > 60) {
        std::fprintf(stderr,"Power must be 0–60 dBm\n");
        return 2;
    }

    // Buffers for normal and altered symbols
    uint8_t normal_syms[WSPR_SYMBOL_COUNT];
    uint8_t alt_syms   [WSPR_SYMBOL_COUNT];
    JTEncode encoder;
    encoder.wspr_encode(call,
                    grid,
                    static_cast<int8_t>(dbm),
                    normal_syms);


    // 2) Dump normal bits + wav
    write_bits("wspr_normal.bits", normal_syms);
    std::puts("→ wspr_normal.bits");
    // build wav samples (matching wsprsimwav.c algorithm)
    {
        size_t total_samples = DELAY_SAMPLES + (SYMBOL_LENGTH * WSPR_SYMBOL_COUNT);
        std::vector<int16_t> audio(total_samples, 0);
        double phase = 0.0;
        
        for(int i=0; i<WSPR_SYMBOL_COUNT; ++i) {
            // Frequency calculation matching reference: center + (symbol - 1.5) * spacing
            double freq = CENTER_FREQ + ((double)normal_syms[i] - 1.5) * FREQ_SPACING;
            double dphi = 2.0 * M_PI * freq / SAMPLE_RATE;
            
            for(int j=0; j<SYMBOL_LENGTH; ++j) {
                int sample_idx = DELAY_SAMPLES + (SYMBOL_LENGTH * i) + j;
                double s = std::sin(phase);
                audio[sample_idx] = int16_t(s * 32767);
                phase += dphi;
            }
        }
        write_wav("wspr_normal.wav", audio);
        std::puts("→ wspr_normal.wav");
    }

    const uint8_t sync_vector[WSPR_SYMBOL_COUNT] = {
    1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0,
    1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1,
    0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0,
    1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
    0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0
    };

    for (int i = 0; i < WSPR_SYMBOL_COUNT; ++i) {
      uint8_t d = (normal_syms[i] >> 1) & 1;       // extract data bit
      uint8_t s = sync_vector[i] ^ 1;              // invert sync bit
      alt_syms[i] = s + 2 * d;                     // rebuild symbol
    }



    // 4) Dump altered bits + wav
    write_bits("wspr_altered.bits", alt_syms);
    std::puts("→ wspr_altered.bits");
    {
        size_t total_samples = DELAY_SAMPLES + (SYMBOL_LENGTH * WSPR_SYMBOL_COUNT);
        std::vector<int16_t> audio(total_samples, 0);
        double phase = 0.0;
        
        for(int i=0; i<WSPR_SYMBOL_COUNT; ++i) {
            // Frequency calculation matching reference: center + (symbol - 1.5) * spacing
            double freq = CENTER_FREQ + ((double)alt_syms[i] - 1.5) * FREQ_SPACING;
            double dphi = 2.0 * M_PI * freq / SAMPLE_RATE;
            
            for(int j=0; j<SYMBOL_LENGTH; ++j) {
                int sample_idx = DELAY_SAMPLES + (SYMBOL_LENGTH * i) + j;
                double s = std::sin(phase);
                audio[sample_idx] = int16_t(s * 32767);
                phase += dphi;
            }
        }
        write_wav("wspr_altered.wav", audio);
        std::puts("→ wspr_altered.wav");
    }

    std::puts("\nSimulation complete. You now have:");
    std::puts(" - wspr_normal.bits & wspr_normal.wav");
    std::puts(" - wspr_altered.bits & wspr_altered.wav");
    return 0;
}

