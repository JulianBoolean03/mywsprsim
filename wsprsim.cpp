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
#include <cctype>
#include <regex>
#include "src/JTEncode.h"

// Audio parameters (matching wsprsimwav.c)
const int    SAMPLE_RATE   = 48000;             // 48kHz sampling rate
const int    SYMBOL_LENGTH = 32768;             // samples per symbol
const double CENTER_FREQ   = 1500.0;           // center frequency (Hz)
const double FREQ_SPACING  = 48000.0 / 32768;  // = 1.46484375 Hz spacing
const int    DELAY_SAMPLES = 48000;             // 1 second delay

// Validate WSPR callsign format
bool validate_callsign(const char* call) {
    if (!call || strlen(call) == 0 || strlen(call) > 12) {
        return false;
    }
    
    // Basic WSPR callsign validation:
    // - Must contain at least one letter
    // - Can contain letters, digits, '/', '<', '>'
    // - Must not be just numbers or symbols
    
    bool has_letter = false;
    bool has_digit = false;
    int len = strlen(call);
    
    for (int i = 0; i < len; i++) {
        char c = call[i];
        if (isalpha(c)) {
            has_letter = true;
        } else if (isdigit(c)) {
            has_digit = true;
        } else if (c != '/' && c != '<' && c != '>') {
            return false; // Invalid character
        }
    }
    
    // Must have at least one letter for valid callsign
    return has_letter;
}

// Validate WSPR grid locator format
bool validate_grid(const char* grid) {
    if (!grid) return false;
    
    int len = strlen(grid);
    if (len != 4 && len != 6) {
        return false;
    }
    
    // Format: AA00 or AA00AA
    // First 2: A-R
    // Next 2: 0-9
    // Last 2 (if present): A-X
    
    if (len >= 2) {
        for (int i = 0; i < 2; i++) {
            char c = toupper(grid[i]);
            if (c < 'A' || c > 'R') {
                return false;
            }
        }
    }
    
    if (len >= 4) {
        for (int i = 2; i < 4; i++) {
            if (!isdigit(grid[i])) {
                return false;
            }
        }
    }
    
    if (len == 6) {
        for (int i = 4; i < 6; i++) {
            char c = toupper(grid[i]);
            if (c < 'A' || c > 'X') {
                return false;
            }
        }
    }
    
    return true;
}

// Validate WSPR power level
bool validate_power(int dbm) {
    const int valid_dbm[] = {
        -30, -27, -23, -20, -17, -13, -10, -7, -3, 
        0, 3, 7, 10, 13, 17, 20, 23, 27, 30, 33, 37, 40,
        43, 47, 50, 53, 57, 60
    };
    const int valid_count = sizeof(valid_dbm) / sizeof(valid_dbm[0]);
    
    for (int i = 0; i < valid_count; i++) {
        if (dbm == valid_dbm[i]) {
            return true;
        }
    }
    return false;
}

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
        std::fprintf(stderr, "\nExamples:\n");
        std::fprintf(stderr, "  %s VK3ABC FM04 20\n", argv[0]);
        std::fprintf(stderr, "  %s W1AW FN42 30\n", argv[0]);
        return 1;
    }
    
    const char* call = argv[1];
    const char* grid = argv[2];
    int dbm = std::atoi(argv[3]);
    
    // Validate callsign
    if (!validate_callsign(call)) {
        std::fprintf(stderr, "Error: Invalid callsign '%s'\n", call);
        std::fprintf(stderr, "Callsign must contain at least one letter and only valid characters (A-Z, 0-9, /, <, >)\n");
        std::fprintf(stderr, "Examples: VK3ABC, W1AW, PJ4/K1ABC, <PJ4/K1ABC>\n");
        return 2;
    }
    
    // Validate grid locator
    if (!validate_grid(grid)) {
        std::fprintf(stderr, "Error: Invalid grid locator '%s'\n", grid);
        std::fprintf(stderr, "Grid must be 4 or 6 characters in format AA00 or AA00AA\n");
        std::fprintf(stderr, "Examples: FM04, FN42, CN85NM\n");
        return 3;
    }
    
    // Validate power level
    if (!validate_power(dbm)) {
        std::fprintf(stderr, "Error: Invalid power level %d dBm\n", dbm);
        std::fprintf(stderr, "Valid power levels: -30, -27, -23, -20, -17, -13, -10, -7, -3,\n");
        std::fprintf(stderr, "                    0, 3, 7, 10, 13, 17, 20, 23, 27, 30, 33, 37, 40,\n");
        std::fprintf(stderr, "                    43, 47, 50, 53, 57, 60\n");
        return 4;
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

