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
//   wspr_normal.rf      (162 frequency values for RF transmission)
//   wspr_altered.bits   (162 bytes: inverted symbols)
//   wspr_altered.rf     (162 frequency values for altered RF transmission)

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include "src/JTEncode.h"

// Audio parameters (matching wsprsimwav.c)
const int    SAMPLE_RATE   = 48000;             // 48kHz sampling rate
const int    SYMBOL_LENGTH = 32768;             // samples per symbol
const double CENTER_FREQ   = 1500.0;           // center frequency (Hz)
const double FREQ_SPACING  = 48000.0 / 32768;  // = 1.46484375 Hz spacing
const int    DELAY_SAMPLES = 48000;             // 1 second delay

// Write RF frequency file
void write_rf(const char *fn, const uint8_t *syms) {
    std::ofstream rf(fn);
    rf << "# WSPR RF Frequency File\n";
    rf << "# Frequency: 14095600\n";
    rf << "# Each line contains frequency in Hz for one symbol\n";
    
    for(int i = 0; i < WSPR_SYMBOL_COUNT; i++) {
        // Convert symbol to frequency: base + (symbol - 1.5) * spacing
        double freq = CENTER_FREQ + ((double)syms[i] - 1.5) * FREQ_SPACING;
        rf << std::fixed << std::setprecision(6) << freq << std::endl;
    }
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


    // 2) Dump normal bits + RF
    write_bits("wspr_normal.bits", normal_syms);
    std::puts("→ wspr_normal.bits");
    write_rf("wspr_normal.rf", normal_syms);
    std::puts("→ wspr_normal.rf");

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



    // 4) Dump altered bits + RF
    write_bits("wspr_altered.bits", alt_syms);
    std::puts("→ wspr_altered.bits");
    write_rf("wspr_altered.rf", alt_syms);
    std::puts("→ wspr_altered.rf");

    std::puts("\nSimulation complete. You now have:");
    std::puts(" - wspr_normal.bits & wspr_normal.rf");
    std::puts(" - wspr_altered.bits & wspr_altered.rf");
    return 0;
}

