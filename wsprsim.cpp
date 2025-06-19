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
 

// Audio parameters
constexpr int    SAMPLE_RATE   = 48000;
constexpr double SYMBOL_RATE   = 162.0 / 110.612;   // ≈1.464817 symbols/sec
constexpr double BIT_PERIOD    = 1.0 / SYMBOL_RATE;
constexpr double FREQ0         = 1400.0;            // “0” tone (Hz)
constexpr double FREQ1         = 1440.0;            // “1” tone (Hz)

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


    // 1) Encode normal
   // if(wspr_encode(call, grid, dbm, normal_syms) != 0) {
     //   std::fprintf(stderr,"wspr_encode() failed\n");
       // return 3;
    //}

    // 2) Dump normal bits + wav
    write_bits("wspr_normal.bits", normal_syms);
    std::puts("→ wspr_normal.bits");
    // build wav samples
    {
        size_t samplesPerBit = size_t(std::round(BIT_PERIOD * SAMPLE_RATE));
        std::vector<int16_t> audio;
        audio.reserve(samplesPerBit * WSPR_SYMBOL_COUNT);
        for(int i=0; i<WSPR_SYMBOL_COUNT; ++i) {
            double freq = normal_syms[i] ? FREQ1 : FREQ0;
            for(size_t n=0; n<samplesPerBit; ++n) {
                double t = double(n + i*samplesPerBit) / SAMPLE_RATE;
                double s = std::sin(2*M_PI*freq*t);
                audio.push_back(int16_t(s * 32767));
            }
        }
        write_wav("wspr_normal.wav", audio);
        std::puts("→ wspr_normal.wav");
    }

   
        // 3) Build altered by inverting only the 11-symbol sync vector
    //    leave the remainder of the message symbols untouched
    //for(int i=0; i<11; ++i) {
       // alt_syms[i] = uint8_t(3 - normal_syms[i]);
    //}
    // copy the rest of the payload unchanged
    //for(int i=11; i<WSPR_SYMBOL_COUNT; ++i) {
      //  alt_syms[i] = normal_syms[i];
    //}
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
        size_t samplesPerBit = size_t(std::round(BIT_PERIOD * SAMPLE_RATE));
        std::vector<int16_t> audio;
        audio.reserve(samplesPerBit * WSPR_SYMBOL_COUNT);
        for(int i=0; i<WSPR_SYMBOL_COUNT; ++i) {
            double freq = alt_syms[i] ? FREQ1 : FREQ0;
            for(size_t n=0; n<samplesPerBit; ++n) {
                double t = double(n + i*samplesPerBit) / SAMPLE_RATE;
                double s = std::sin(2*M_PI*freq*t);
                audio.push_back(int16_t(s * 32767));
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

