#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>

const int SAMPLE_RATE = 48000;
const int SYMBOL_LENGTH = 32768;  // 0.683 seconds per symbol
const int NUM_SYMBOLS = 8;        // Just 8 symbols for testing

struct WavHeader {
    char     riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunk_size;
    char     wave[4] = {'W', 'A', 'V', 'E'};
    char     fmt[4] = {'f', 'm', 't', ' '};
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1;
    uint16_t num_channels = 1;
    uint32_t sample_rate = SAMPLE_RATE;
    uint32_t byte_rate = SAMPLE_RATE * 2;
    uint16_t block_align = 2;
    uint16_t bits_per_sample = 16;
    char     data[4] = {'d', 'a', 't', 'a'};
    uint32_t subchunk2_size;
};

int main() {
    // Test symbols: 0, 1, 2, 3, 0, 1, 2, 3
    uint8_t symbols[NUM_SYMBOLS] = {0, 1, 2, 3, 0, 1, 2, 3};
    
    // Frequencies for each symbol (MUCH wider separation for testing)
    double frequencies[4] = {1400.0, 1450.0, 1500.0, 1550.0};
    
    std::vector<double> signal(NUM_SYMBOLS * SYMBOL_LENGTH, 0.0);
    
    double phase = 0.0;
    double two_pi_dt = 2.0 * M_PI / SAMPLE_RATE;
    
    for (int sym = 0; sym < NUM_SYMBOLS; sym++) {
        double freq = frequencies[symbols[sym]];
        double dphi = two_pi_dt * freq;
        
        printf("Symbol %d: value=%d, freq=%.1f Hz\n", sym, symbols[sym], freq);
        
        for (int samp = 0; samp < SYMBOL_LENGTH; samp++) {
            int pos = sym * SYMBOL_LENGTH + samp;
            signal[pos] = 0.5 * sin(phase);
            phase += dphi;
        }
        
        // Keep phase continuous
        while (phase > 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }
    
    // Write WAV file
    WavHeader header;
    header.subchunk2_size = signal.size() * 2;
    header.chunk_size = 36 + header.subchunk2_size;
    
    std::ofstream wav("test_wspr.wav", std::ios::binary);
    wav.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    for (double sample : signal) {
        int16_t sample_16 = static_cast<int16_t>(sample * 32767);
        wav.write(reinterpret_cast<const char*>(&sample_16), sizeof(sample_16));
    }
    
    wav.close();
    printf("Created test_wspr.wav with %d symbols\n", NUM_SYMBOLS);
    printf("Duration: %.1f seconds\n", (double)signal.size() / SAMPLE_RATE);
    
    return 0;
}
