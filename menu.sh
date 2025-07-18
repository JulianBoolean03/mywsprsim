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

# Generate new WSPR RF files
generate_rf_files() {
    clear_and_header
    echo -e "${YELLOW}[Generate] Generate New WSPR RF Files${NC}"
    echo ""
    echo "This will create both normal and altered WSPR RF signals."
    echo ""
    
    # Get user input
    read -p "Enter callsign (e.g., VK3ABC): " callsign
    read -p "Enter grid locator (e.g., FM04): " grid
    read -p "Enter power in dBm (0-60): " power
    
    # Validate power
    if ! [[ "$power" =~ ^[0-9]+$ ]] || [ "$power" -lt 0 ] || [ "$power" -gt 60 ]; then
        echo -e "${RED}[Error] Power must be a number between 0 and 60${NC}"
        read -p "Press Enter to continue..."
        return
    fi
    
    echo ""
    echo -e "${BLUE}[Working] Generating WSPR files for: $callsign $grid ${power}dBm${NC}"
    echo ""
    
    # Run wsprsim
    if ./wsprsim "$callsign" "$grid" "$power"; then
        echo ""
        echo -e "${GREEN}[OK] Generation complete!${NC}"
        echo ""
        echo "Files created:"
        echo "  • wspr_normal.rf & wspr_normal.bits"
        echo "  • wspr_altered.rf & wspr_altered.bits"
        
        # Show RF file preview
        echo ""
        echo -e "${BLUE}[Preview] RF File Preview:${NC}"
        echo "Normal RF (first 10 lines):"
        head -10 wspr_normal.rf | sed 's/^/  /'
        echo ""
        echo "Altered RF (first 10 lines):"
        head -10 wspr_altered.rf | sed 's/^/  /'
    else
        echo -e "${RED}[Error] Error generating files${NC}"
    fi
    
    echo ""
    read -p "Press Enter to continue..."
}

# Test RF files against all decoders
test_all_decoders() {
    clear_and_header
echo -e "${YELLOW}[Test] Test RF Files Against All Decoders${NC}"
    echo ""
    
    # Check if RF files exist
    if [[ ! -f "wspr_normal.rf" ]] || [[ ! -f "wspr_altered.rf" ]]; then
        echo -e "${RED}[Error] RF files not found!${NC}"
        echo "Please generate RF files first (Option 1)"
        echo ""
        read -p "Press Enter to continue..."
        return
    fi
    
    echo "Testing all decoder combinations..."
    echo ""
    
    # Test 1: Normal RF + Normal decoder (should decode)
echo -e "${BLUE}[Check] Test 1: Normal RF → Normal Decoder${NC}"
    echo "Expected: [OK] SHOULD DECODE"
    echo "Running decode_norm_norm.sh..."
    echo "----------------------------------------"
    ./decode_norm_norm.sh
    
    echo ""
    echo "========================================"
    echo ""
    
    # Test 2: Normal RF + Altered decoder (should NOT decode)
echo -e "${BLUE}[Check] Test 2: Normal RF → Altered Decoder${NC}"
    echo "Expected: [FAIL] SHOULD NOT DECODE"
    echo "Running decode_alt_norm.sh..."
    echo "----------------------------------------"
    ./decode_alt_norm.sh
    
    echo ""
    echo "========================================"
    echo ""
    
    # Test 3: Altered RF + Normal decoder (should NOT decode)
echo -e "${BLUE}[Check] Test 3: Altered RF → Normal Decoder${NC}"
    echo "Expected: [FAIL] SHOULD NOT DECODE"
    echo "Running decode_norm_alt.sh..."
    echo "----------------------------------------"
    ./decode_norm_alt.sh
    
    echo ""
    echo "========================================"
    echo ""
    
    # Test 4: Altered RF + Altered decoder (should decode)
echo -e "${BLUE}[Check] Test 4: Altered RF → Altered Decoder${NC}"
    echo "Expected: [OK] SHOULD DECODE"
    echo "Running decode_alt_alt.sh..."
    echo "----------------------------------------"
    ./decode_alt_alt.sh
    
    echo ""
echo -e "${GREEN}[Done] All tests completed!${NC}"
    echo ""
    echo "Summary:"
    echo "• Test 1 (Normal RF → Normal Decoder): Should decode [OK]"
    echo "• Test 2 (Normal RF → Altered Decoder): Should fail [FAIL]"
    echo "• Test 3 (Altered RF → Normal Decoder): Should fail [FAIL]"
    echo "• Test 4 (Altered RF → Altered Decoder): Should decode [OK]"
    echo ""
    read -p "Press Enter to continue..."
}

# Show statistics about RF files
show_rf_stats() {
    clear_and_header
    echo -e "${YELLOW}[Stats] RF File Statistics${NC}"
    echo ""
    
    # Check if files exist
    rf_files=("wspr_normal.rf" "wspr_altered.rf")
    bit_files=("wspr_normal.bits" "wspr_altered.bits")
    
    echo -e "${PURPLE}[Files] File Information:${NC}"
    echo "----------------------------------------"
    for file in "${rf_files[@]}" "${bit_files[@]}"; do
        if [[ -f "$file" ]]; then
            size=$(du -h "$file" | cut -f1)
            date=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$file" 2>/dev/null || stat -c "%y" "$file" 2>/dev/null | cut -d'.' -f1)
            echo -e "${GREEN}[OK]${NC} $file - ${size} - ${date}"
        else
            echo -e "${RED}[Missing]${NC} $file - Not found"
        fi
    done
    
    echo ""
    echo -e "${PURPLE}[RF] RF Signal Properties:${NC}"
    echo "----------------------------------------"
    
    for rf_file in "wspr_normal.rf" "wspr_altered.rf"; do
        if [[ -f "$rf_file" ]]; then
            echo -e "${CYAN}[File] $rf_file:${NC}"
            
            # Extract frequency from header
            base_freq=$(grep "# Frequency:" "$rf_file" | cut -d' ' -f3)
            if [[ -n "$base_freq" ]]; then
                echo "  Base Frequency: ${base_freq} Hz"
            fi
            
            # Count symbols (excluding header lines)
            symbol_count=$(grep -v "^#" "$rf_file" | wc -l | xargs)
            echo "  Symbol Count: $symbol_count (should be 162)"
            
            # Frequency statistics
            echo "  Frequency Range:"
            grep -v "^#" "$rf_file" | sort -n | head -1 | sed 's/^/    Min: /'
            grep -v "^#" "$rf_file" | sort -n | tail -1 | sed 's/^/    Max: /'
            
            # Show first few frequencies
            echo "  First 5 symbol frequencies:"
            grep -v "^#" "$rf_file" | head -5 | sed 's/^/    /'
            echo ""
        fi
    done
    
    echo -e "${PURPLE}[Symbols] Symbol Information:${NC}"
    echo "----------------------------------------"
    
    for bit_file in "wspr_normal.bits" "wspr_altered.bits"; do
        if [[ -f "$bit_file" ]]; then
            echo -e "${CYAN}[File] $bit_file:${NC}"
            size=$(wc -c < "$bit_file")
            echo "  Symbols: $size (should be 162)"
            echo "  First 20 symbols: $(xxd -l 20 -ps "$bit_file" | sed 's/../& /g')"
            echo ""
        fi
    done
    
    echo -e "${PURPLE}[Disk] Disk Usage:${NC}"
    echo "----------------------------------------"
    total_size=$(du -ch wspr_*.rf wspr_*.bits 2>/dev/null | tail -1 | cut -f1)
    echo "Total project files: ${total_size:-0}"
    
    echo ""
    read -p "Press Enter to continue..."
}

# Quick decode test
quick_decode_test() {
    clear_and_header
    echo -e "${YELLOW}[Quick] Quick Decoder Test${NC}"
    echo ""
    
    echo "Select test to run:"
    echo "1) Normal RF → Normal Decoder (should work)"
    echo "2) Normal RF → Altered Decoder (should fail)"
    echo "3) Altered RF → Normal Decoder (should fail)"
    echo "4) Altered RF → Altered Decoder (should work)"
    echo "5) Back to main menu"
    echo ""
    read -p "Enter choice (1-5): " choice
    
    case $choice in
        1)
            echo -e "${BLUE}Testing Normal RF → Normal Decoder...${NC}"
            ./decode_norm_norm.sh
            ;;
        2)
            echo -e "${BLUE}Testing Normal RF → Altered Decoder...${NC}"
            ./decode_alt_norm.sh
            ;;
        3)
            echo -e "${BLUE}Testing Altered RF → Normal Decoder...${NC}"
            ./decode_norm_alt.sh
            ;;
        4)
            echo -e "${BLUE}Testing Altered RF → Altered Decoder...${NC}"
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
        echo "1) [RF] Generate New RF Files"
        echo "2) [Test] Test All Decoders (Complete Suite)"
        echo "3) [Quick] Quick Decoder Test (Individual)"
        echo "4) [Stats] Show RF File Statistics"
        echo "5) [Exit] Exit"
        echo ""
        read -p "Enter your choice (1-5): " choice
        
        case $choice in
            1)
                generate_rf_files
                ;;
            2)
                test_all_decoders
                ;;
            3)
                quick_decode_test
                ;;
            4)
                show_rf_stats
                ;;
            5)
                echo ""
echo -e "${GREEN}[Goodbye] Thank you for using WSPR Project Control Center!${NC}"
                echo ""
                exit 0
                ;;
            *)
                echo ""
                echo -e "${RED}[Error] Invalid choice. Please enter 1-5.${NC}"
                sleep 2
                ;;
        esac
    done
}

# Check if we're in the right directory
if [[ ! -f "wsprsim" ]] || [[ ! -f "decode_norm_norm.sh" ]]; then
    echo -e "${RED}[Error] This script must be run from the jtencode-sim directory${NC}"
    echo "Make sure you're in the directory containing wsprsim and the decode scripts."
    exit 1
fi

# Start the menu
main_menu

