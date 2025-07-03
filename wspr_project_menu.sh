#!/bin/bash

# WSPR Project Control Menu
# Centralized interface for generating, testing, and analyzing WSPR signals

# Color codes for better UI
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Clear screen and show header
clear_and_header() {
    clear
    echo -e "${CYAN}=============================================${NC}"
    echo -e "${CYAN}      WSPR Project Control Center${NC}"
    echo -e "${CYAN}=============================================${NC}"
    echo ""
}

# Generate new WSPR wav files
generate_wav_files() {
    clear_and_header
    echo -e "${YELLOW}🎵 Generate New WSPR WAV Files${NC}"
    echo ""
    echo "This will create both normal and altered WSPR signals."
    echo ""
    
    # Get user input
    read -p "Enter callsign (e.g., VK3ABC): " callsign
    read -p "Enter grid locator (e.g., FM04): " grid
    read -p "Enter power in dBm (0-60): " power
    
    # Validate power
    if ! [[ "$power" =~ ^[0-9]+$ ]] || [ "$power" -lt 0 ] || [ "$power" -gt 60 ]; then
        echo -e "${RED}❌ Error: Power must be a number between 0 and 60${NC}"
        read -p "Press Enter to continue..."
        return
    fi
    
    echo ""
    echo -e "${BLUE}🔧 Generating WSPR files for: $callsign $grid ${power}dBm${NC}"
    echo ""
    
    # Run wsprsim
    if ./wsprsim "$callsign" "$grid" "$power"; then
        echo ""
        echo -e "${GREEN}✅ Generation complete!${NC}"
        echo ""
        echo "Files created:"
        echo "  • wspr_normal.wav & wspr_normal.bits"
        echo "  • wspr_altered.wav & wspr_altered.bits"
        
        # Generate 12k versions for compatibility
        echo ""
        echo -e "${BLUE}🔄 Creating 12kHz versions for decoder compatibility...${NC}"
        if command -v sox >/dev/null 2>&1; then
            sox wspr_normal.wav -r 12000 wspr_normal_12k.wav 2>/dev/null
            sox wspr_altered.wav -r 12000 wspr_altered_12k.wav 2>/dev/null
            echo -e "${GREEN}✅ 12kHz versions created${NC}"
        else
            echo -e "${YELLOW}⚠️  Sox not found - 12kHz versions not created${NC}"
        fi
    else
        echo -e "${RED}❌ Error generating files${NC}"
    fi
    
    echo ""
    read -p "Press Enter to continue..."
}

# Test wav files against all decoders
test_all_decoders() {
    clear_and_header
    echo -e "${YELLOW}🧪 Test WAV Files Against All Decoders${NC}"
    echo ""
    
    # Check if wav files exist
    if [[ ! -f "wspr_normal.wav" ]] || [[ ! -f "wspr_altered.wav" ]]; then
        echo -e "${RED}❌ Error: WAV files not found!${NC}"
        echo "Please generate WAV files first (Option 1)"
        echo ""
        read -p "Press Enter to continue..."
        return
    fi
    
    echo "Testing all decoder combinations..."
    echo ""
    
    # Test 1: Normal wav + Normal decoder (should decode)
    echo -e "${BLUE}🔍 Test 1: Normal WAV → Normal Decoder${NC}"
    echo "Expected: ✅ SHOULD DECODE"
    echo "Running decode_norm_norm.sh..."
    echo "----------------------------------------"
    if timeout 30s ./decode_norm_norm.sh | head -20; then
        echo ""
    else
        echo -e "${RED}❌ Test timed out or failed${NC}"
    fi
    
    echo ""
    echo "Press Enter to continue to next test..."
    read
    
    # Test 2: Normal wav + Altered decoder (should NOT decode)
    echo -e "${BLUE}🔍 Test 2: Normal WAV → Altered Decoder${NC}"
    echo "Expected: ❌ SHOULD NOT DECODE"
    echo "Running decode_norm_alt.sh..."
    echo "----------------------------------------"
    if timeout 30s ./decode_norm_alt.sh | head -20; then
        echo ""
    else
        echo -e "${RED}❌ Test timed out or failed${NC}"
    fi
    
    echo ""
    echo "Press Enter to continue to next test..."
    read
    
    # Test 3: Altered wav + Normal decoder (should NOT decode)
    echo -e "${BLUE}🔍 Test 3: Altered WAV → Normal Decoder${NC}"
    echo "Expected: ❌ SHOULD NOT DECODE"
    echo "Running decode_alt_norm.sh..."
    echo "----------------------------------------"
    if timeout 30s ./decode_alt_norm.sh | head -20; then
        echo ""
    else
        echo -e "${RED}❌ Test timed out or failed${NC}"
    fi
    
    echo ""
    echo "Press Enter to continue to final test..."
    read
    
    # Test 4: Altered wav + Altered decoder (should decode)
    echo -e "${BLUE}🔍 Test 4: Altered WAV → Altered Decoder${NC}"
    echo "Expected: ✅ SHOULD DECODE"
    echo "Running decode_alt_alt.sh..."
    echo "----------------------------------------"
    if timeout 30s ./decode_alt_alt.sh | head -20; then
        echo ""
    else
        echo -e "${RED}❌ Test timed out or failed${NC}"
    fi
    
    echo ""
    echo -e "${GREEN}🎉 All tests completed!${NC}"
    echo ""
    read -p "Press Enter to continue..."
}

# Show statistics about WAV files
show_wav_stats() {
    clear_and_header
    echo -e "${YELLOW}📊 WAV File Statistics${NC}"
    echo ""
    
    # Check if files exist
    wav_files=("wspr_normal.wav" "wspr_altered.wav" "wspr_normal_12k.wav" "wspr_altered_12k.wav")
    bit_files=("wspr_normal.bits" "wspr_altered.bits")
    
    echo -e "${PURPLE}📁 File Information:${NC}"
    echo "----------------------------------------"
    for file in "${wav_files[@]}" "${bit_files[@]}"; do
        if [[ -f "$file" ]]; then
            size=$(du -h "$file" | cut -f1)
            date=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$file" 2>/dev/null || stat -c "%y" "$file" 2>/dev/null | cut -d'.' -f1)
            echo -e "${GREEN}✅${NC} $file - ${size} - ${date}"
        else
            echo -e "${RED}❌${NC} $file - Not found"
        fi
    done
    
    echo ""
    echo -e "${PURPLE}🎵 Audio Properties:${NC}"
    echo "----------------------------------------"
    
    for wav_file in "wspr_normal.wav" "wspr_altered.wav"; do
        if [[ -f "$wav_file" ]]; then
            echo -e "${CYAN}📄 $wav_file:${NC}"
            if command -v soxi >/dev/null 2>&1; then
                soxi "$wav_file" | grep -E "(Sample Rate|Duration|Channels|Sample Encoding)"
            elif command -v ffprobe >/dev/null 2>&1; then
                ffprobe -v quiet -show_entries format=duration,bit_rate -show_entries stream=sample_rate,channels "$wav_file" 2>/dev/null
            else
                echo "  Audio analysis tools not available (install sox or ffmpeg)"
            fi
            echo ""
        fi
    done
    
    echo -e "${PURPLE}🔢 Symbol Information:${NC}"
    echo "----------------------------------------"
    
    for bit_file in "wspr_normal.bits" "wspr_altered.bits"; do
        if [[ -f "$bit_file" ]]; then
            echo -e "${CYAN}📄 $bit_file:${NC}"
            size=$(wc -c < "$bit_file")
            echo "  Symbols: $size (should be 162)"
            echo "  First 20 symbols: $(xxd -l 20 -ps "$bit_file" | sed 's/../& /g')"
            echo ""
        fi
    done
    
    echo -e "${PURPLE}💾 Disk Usage:${NC}"
    echo "----------------------------------------"
    total_size=$(du -ch wspr_*.wav wspr_*.bits 2>/dev/null | tail -1 | cut -f1)
    echo "Total project files: ${total_size:-0}"
    
    echo ""
    read -p "Press Enter to continue..."
}

# Quick decode test
quick_decode_test() {
    clear_and_header
    echo -e "${YELLOW}⚡ Quick Decoder Test${NC}"
    echo ""
    
    echo "Select test to run:"
    echo "1) Normal WAV → Normal Decoder (should work)"
    echo "2) Normal WAV → Altered Decoder (should fail)"
    echo "3) Altered WAV → Normal Decoder (should fail)"
    echo "4) Altered WAV → Altered Decoder (should work)"
    echo "5) Back to main menu"
    echo ""
    read -p "Enter choice (1-5): " choice
    
    case $choice in
        1)
            echo -e "${BLUE}Testing Normal WAV → Normal Decoder...${NC}"
            ./decode_norm_norm.sh
            ;;
        2)
            echo -e "${BLUE}Testing Normal WAV → Altered Decoder...${NC}"
            ./decode_norm_alt.sh
            ;;
        3)
            echo -e "${BLUE}Testing Altered WAV → Normal Decoder...${NC}"
            ./decode_alt_norm.sh
            ;;
        4)
            echo -e "${BLUE}Testing Altered WAV → Altered Decoder...${NC}"
            ./decode_alt_alt.sh
            ;;
        5)
            return
            ;;
        *)
            echo -e "${RED}Invalid choice${NC}"
            ;;
    esac
    
    echo ""
    read -p "Press Enter to continue..."
}

# Main menu
main_menu() {
    while true; do
        clear_and_header
        echo -e "${GREEN}Choose an option:${NC}"
        echo ""
        echo "1) 🎵 Generate New WAV Files"
        echo "2) 🧪 Test All Decoders (Complete Suite)"
        echo "3) ⚡ Quick Decoder Test (Individual)"
        echo "4) 📊 Show WAV File Statistics"
        echo "5) 🚪 Exit"
        echo ""
        read -p "Enter your choice (1-5): " choice
        
        case $choice in
            1)
                generate_wav_files
                ;;
            2)
                test_all_decoders
                ;;
            3)
                quick_decode_test
                ;;
            4)
                show_wav_stats
                ;;
            5)
                echo ""
                echo -e "${GREEN}👋 Thank you for using WSPR Project Control Center!${NC}"
                echo ""
                exit 0
                ;;
            *)
                echo ""
                echo -e "${RED}❌ Invalid choice. Please enter 1-5.${NC}"
                sleep 2
                ;;
        esac
    done
}

# Check if we're in the right directory
if [[ ! -f "wsprsim" ]] || [[ ! -f "decode_norm_norm.sh" ]]; then
    echo -e "${RED}❌ Error: This script must be run from the jtencode-sim directory${NC}"
    echo "Make sure you're in the directory containing wsprsim and the decode scripts."
    exit 1
fi

# Start the menu
main_menu
