# WSPR Altered Sync Vector Project - Setup Guide

This project implements a modified WSPR system with an altered sync vector that creates parallel communication channels invisible to standard WSPR decoders.

## Project Overview

- **Normal WSPR**: Standard WSPR encoding/decoding
- **Altered WSPR**: Modified sync vector for covert communication
- **Cross-compatibility**: Normal signals don't decode with altered decoder and vice versa
- **Complete toolchain**: From message input to Arduino hex files to WAV generation to decoding

## System Requirements

### Operating System Support
- ✅ **macOS** (tested on macOS with zsh)
- ✅ **Linux** (Ubuntu, Debian, CentOS, etc.)
- ✅ **Windows** (with WSL2 or MSYS2/MinGW)

### Hardware Requirements
- Minimum 4GB RAM
- 500MB free disk space
- Audio playback capability (for testing WAV files)

## Required Software Packages

### 1. Essential Build Tools
```bash
# macOS (using Homebrew)
brew install gcc make cmake git

# Ubuntu/Debian
sudo apt update
sudo apt install build-essential gcc g++ make cmake git

# CentOS/RHEL/Fedora
sudo yum install gcc gcc-c++ make cmake git
# OR (newer versions)
sudo dnf install gcc gcc-c++ make cmake git

# Windows (WSL2 Ubuntu)
sudo apt update
sudo apt install build-essential gcc g++ make cmake git
```

### 2. Fortran Compiler (for WSPR decoder)
```bash
# macOS
brew install gfortran

# Ubuntu/Debian  
sudo apt install gfortran

# CentOS/RHEL/Fedora
sudo yum install gcc-gfortran
# OR
sudo dnf install gcc-gfortran
```

### 3. FFTW Library (Fast Fourier Transform)
```bash
# macOS
brew install fftw

# Ubuntu/Debian
sudo apt install libfftw3-dev

# CentOS/RHEL/Fedora
sudo yum install fftw-devel
# OR
sudo dnf install fftw-devel
```

### 4. Audio Processing Tools
```bash
# macOS
brew install sox

# Ubuntu/Debian
sudo apt install sox libsox-fmt-all

# CentOS/RHEL/Fedora
sudo yum install sox
# OR
sudo dnf install sox
```

### 5. Mathematical Libraries
```bash
# Ubuntu/Debian
sudo apt install bc

# CentOS/RHEL/Fedora
sudo yum install bc
# OR 
sudo dnf install bc

# macOS (usually pre-installed, or via Homebrew)
brew install bc
```

### 6. Optional Tools (for enhanced functionality)
```bash
# Audio analysis tools
# macOS
brew install ffmpeg

# Ubuntu/Debian
sudo apt install ffmpeg

# Text processing (usually pre-installed)
# These should be available: grep, sed, awk, xxd
```

## Project Structure

```
jtencode-sim/
├── src/                          # JTEncode library source
├── wspr-cui/
│   ├── wsprd/                    # Normal WSPR decoder
│   └── wsprd-alt/                # Altered WSPR decoder (flipped sync)
├── wsprsim                       # WSPR signal generator (executable)
├── wsprsim.cpp                   # WSPR generator source code
├── decode_norm_norm.sh           # Normal WAV → Normal decoder
├── decode_norm_alt.sh            # Normal WAV → Altered decoder  
├── decode_alt_norm.sh            # Altered WAV → Normal decoder
├── decode_alt_alt.sh             # Altered WAV → Altered decoder
├── wspr_project_menu.sh          # Main control interface
└── README.md / SETUP_GUIDE.md    # Documentation
```

## Installation Steps

### Step 1: Clone/Download the Project
```bash
# If using git
git clone <repository-url> jtencode-sim
cd jtencode-sim

# OR download and extract the project files
```

### Step 2: Build the JTEncode Library
```bash
# Navigate to the source directory
cd src

# Compile the JTEncode library
make clean
make

# This creates libjtencode.a
cd ..
```

### Step 3: Build WSPR Components
```bash
# Build the WSPR signal generator
g++ wsprsim.cpp -Isrc -L. -ljtencode -o wsprsim

# Build normal WSPR decoder
cd wspr-cui/wsprd
make clean
make
cd ../..

# Build altered WSPR decoder  
cd wspr-cui/wsprd-alt
make clean
make
cd ../..
```

### Step 4: Make Scripts Executable
```bash
chmod +x *.sh
chmod +x wspr_project_menu.sh
```

### Step 5: Test the Installation
```bash
# Run the main menu
./wspr_project_menu.sh

# OR test individual components
./wsprsim VK3ABC FM04 20
```

## Verification Tests

### Test 1: Generate Test Signals
```bash
./wsprsim TEST FM04 20
```
**Expected output:**
- `wspr_normal.wav`, `wspr_normal.bits`
- `wspr_altered.wav`, `wspr_altered.bits`

### Test 2: Verify Decoders Work
```bash
# Should decode successfully
./decode_norm_norm.sh

# Should decode successfully  
./decode_alt_alt.sh

# Should NOT decode (proving separation)
./decode_norm_alt.sh
./decode_alt_norm.sh
```

### Test 3: Check Audio Files
```bash
# Verify WAV files are playable
sox wspr_normal.wav -n stat
sox wspr_altered.wav -n stat
```

## Troubleshooting

### Common Issues

**1. "Command not found" errors**
```bash
# Check if tools are installed
which gcc gfortran make sox bc

# Add to PATH if needed (add to ~/.bashrc or ~/.zshrc)
export PATH="/usr/local/bin:$PATH"
```

**2. "FFTW not found" during compilation**
```bash
# Ubuntu/Debian
sudo apt install libfftw3-dev

# Check pkg-config
pkg-config --libs fftw3
```

**3. "Permission denied" for scripts**
```bash
chmod +x *.sh
chmod +x wspr_project_menu.sh
```

**4. Compilation errors in wsprd**
```bash
# Make sure you have gfortran
gfortran --version

# Check Makefile paths
cd wspr-cui/wsprd
cat Makefile
```

### Platform-Specific Notes

**macOS:**
- Install Xcode Command Line Tools: `xcode-select --install`
- Use Homebrew for package management
- May need to set compiler paths

**Linux:**
- Ensure development packages are installed (`-dev` or `-devel`)
- Check library paths: `/usr/lib`, `/usr/local/lib`
- May need `ldconfig` after installing libraries

**Windows (WSL2):**
- Use Ubuntu 20.04+ for best compatibility
- All Linux instructions apply
- Access files from Windows at `/mnt/c/`

## Usage Examples

### Generate Custom WSPR Signals
```bash
# Interactive mode
./wspr_project_menu.sh

# Command line mode
./wsprsim "YOUR_CALL" "GRID" 30
```

### Test Decoder Matrix
```bash
# Test all combinations
./wspr_project_menu.sh  # Option 2

# Test individual combinations
./decode_norm_norm.sh   # Should work
./decode_alt_alt.sh     # Should work  
./decode_norm_alt.sh    # Should fail
./decode_alt_norm.sh    # Should fail
```

### Analyze Generated Files
```bash
# File statistics
./wspr_project_menu.sh  # Option 4

# Manual analysis
sox wspr_normal.wav -n stat
xxd -l 20 wspr_normal.bits
```

## Technical Details

### Key Modifications
1. **Sync Vector Inversion**: The 162-symbol sync pattern is bitwise inverted
2. **Decoder Separation**: Normal decoders can't decode altered signals
3. **Bidirectional**: Both encoding and decoding support the altered format
4. **Arduino Compatible**: Generates hex files for RF transmission

### File Formats
- **`.wav`**: 48kHz/16-bit PCM audio (1400-1440Hz tones)
- **`.bits`**: Raw symbol data (162 bytes, values 0-3)  
- **`.hex`**: Arduino-compatible frequency control data

## Support

### Before Asking for Help
1. ✅ Check all required packages are installed
2. ✅ Verify compilation completed without errors
3. ✅ Test with the provided example data
4. ✅ Check file permissions on scripts
5. ✅ Confirm you're in the correct directory

### Getting Help
- Check error messages carefully
- Include your OS and versions: `gcc --version`, `gfortran --version`
- Provide the exact command that failed
- Include any error output

## Success Criteria

Your installation is successful when:
- ✅ `wsprsim` generates WAV and bits files
- ✅ Normal signals decode with normal decoder
- ✅ Altered signals decode with altered decoder
- ✅ Cross-combinations fail to decode (showing separation)
- ✅ Menu system runs without errors

**Congratulations! You now have a working WSPR altered sync vector system.**
