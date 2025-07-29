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
const int    SIGNAL_LENGTH = SYMBOL_LENGTH * WSPR_SYMBOL_COUNT;  // total samples
const int    SLOPE_SAMPLES = 0.02 * SAMPLE_RATE;  // 20ms slope
// Total samples: each symbol plus one-second delays at start/end
const int    TOTAL_SAMPLES = SIGNAL_LENGTH + 2 * DELAY_SAMPLES;  // =162*32768 + 2*48000 = 5404416 samples

// WAV file header structure
struct WavHeader {
    char     riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunk_size;
    char     wave[4] = {'W', 'A', 'V', 'E'};
    char     fmt[4] = {'f', 'm', 't', ' '};
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1;  // PCM
    uint16_t num_channels = 1;  // mono
    uint32_t sample_rate = SAMPLE_RATE;
    uint32_t byte_rate = SAMPLE_RATE * 2;  // 16-bit samples
    uint16_t block_align = 2;
    uint16_t bits_per_sample = 16;
    char     data[4] = {'d', 'a', 't', 'a'};
    uint32_t subchunk2_size;
};

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
    //
    
    
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

// Write RF frequency file *this writes a text file with one freq per line
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

// Calculate raised cosine slope for fade in/out
double raised_cosine(double x) {
    if (x <= -1.0 || x >= 1.0) return 0.0;
    return 0.5 * (1.0 + cos(M_PI * x));
}

// Volume envelope for fade in/out
double volume_envelope(int sample_pos) {
    if (sample_pos >= 0 && sample_pos < SLOPE_SAMPLES) {
        // Fade in
        return raised_cosine(1.0 - (double)sample_pos / SLOPE_SAMPLES);
    }
    if (sample_pos >= (SIGNAL_LENGTH - SLOPE_SAMPLES) && sample_pos < SIGNAL_LENGTH) {
        // Fade out
        int fade_pos = sample_pos - (SIGNAL_LENGTH - SLOPE_SAMPLES);
        return raised_cosine((double)fade_pos / SLOPE_SAMPLES);
    }
    if (sample_pos >= 0 && sample_pos < SIGNAL_LENGTH) {
        // Full volume
        return 1.0;
    }
    // Outside signal range
    return 0.0;
}

// Generate WAV audio signal from WSPR symbols
void generate_wav_signal(const uint8_t* symbols, std::vector<double>& signal) {
    signal.resize(TOTAL_SAMPLES, 0.0);
    
    double phase = 0.0;
    double two_pi_dt = 2.0 * M_PI / SAMPLE_RATE;
    
    for (int sym = 0; sym < WSPR_SYMBOL_COUNT; sym++) {
        // Calculate frequency for this symbol
        double freq = CENTER_FREQ + ((double)symbols[sym] - 1.5) * FREQ_SPACING; //maps 4 audio freqs spaced around the center freq
        double dphi = two_pi_dt * freq; // uses the freqs to generate the phase since the phase changes wit the freqs
        
        // Generate samples for this symbol
        for (int samp = 0; samp < SYMBOL_LENGTH; samp++) {
            int total_pos = DELAY_SAMPLES + sym * SYMBOL_LENGTH + samp;//gets the absolute position of the signal vector where the sample should be stored
            
            if (total_pos < TOTAL_SAMPLES) {
                // Use constant amplitude for now to isolate frequency issue
                // vol envelope
                int env_idx = total_pos- DELAY_SAMPLES;
                double env = volume_envelope(env_idx);
{
                // apply fade‐in/out envelope
                int env_idx = total_pos - DELAY_SAMPLES;
                double env = volume_envelope(env_idx);
                signal[total_pos] = 0.5 * env * sin(phase); // adds t
            }
                phase += dphi;
            }
        }
        
        // Keep phase continuous but normalize to prevent overflow
        while (phase > 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        while (phase < -2.0 * M_PI) {
            phase += 2.0 * M_PI;
        }
    }
}

// Write WAV file
void write_wav(const char* filename, const uint8_t* symbols) {
    std::vector<double> signal;
    generate_wav_signal(symbols, signal);
    
    // Create WAV header
    WavHeader header;
    header.subchunk2_size = signal.size() * 2; // 16-bit samples
    header.chunk_size = 36 + header.subchunk2_size;
    
    std::ofstream wav(filename, std::ios::binary);
    if (!wav) {
        std::fprintf(stderr, "Error: Cannot create WAV file %s\n", filename);
        return;
    }
    
    // Write header
    wav.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Write audio data (convert to 16-bit signed integers)
    for (double sample : signal) {
        // Clamp to [-1.0, 1.0] and convert to 16-bit
        sample = std::max(-1.0, std::min(1.0, sample));
        int16_t sample_16 = static_cast<int16_t>(sample * 32767);
        wav.write(reinterpret_cast<const char*>(&sample_16), sizeof(sample_16));
    }
    
    wav.close();
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


    // 2) Dump normal bits + RF + WAV
    write_bits("wspr_normal.bits", normal_syms);
    std::puts("→ wspr_normal.bits");
    write_rf("wspr_normal.rf", normal_syms);
    std::puts("→ wspr_normal.rf");
    write_wav("wspr_normal.wav", normal_syms);
    std::puts("→ wspr_normal.wav");
   //inverting the sync bits for altered 
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



    // 4) Dump altered bits + RF + WAV
    write_bits("wspr_altered.bits", alt_syms);
    std::puts("→ wspr_altered.bits");
    write_rf("wspr_altered.rf", alt_syms);
    std::puts("→ wspr_altered.rf");
    write_wav("wspr_altered.wav", alt_syms);
    std::puts("→ wspr_altered.wav");

    std::puts("\nSimulation complete. You now have:");
    std::puts(" - wspr_normal.bits, wspr_normal.rf, wspr_normal.wav");
    std::puts(" - wspr_altered.bits, wspr_altered.rf, wspr_altered.wav");
    return 0;
}

