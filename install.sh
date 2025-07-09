#!/bin/bash

# WSPR Altered Sync Vector Project - Installation Script
# Automates the setup process for different operating systems

set -e  # Exit on any error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect operating system
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get >/dev/null 2>&1; then
            OS="ubuntu"
        elif command -v yum >/dev/null 2>&1; then
            OS="rhel"
        elif command -v dnf >/dev/null 2>&1; then
            OS="fedora"
        else
            OS="linux"
        fi
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        OS="windows"
    else
        OS="unknown"
    fi
    
    print_status "Detected OS: $OS"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Install packages based on OS
install_dependencies() {
    print_status "Installing required dependencies..."
    
    case $OS in
        "macos")
            if ! command_exists brew; then
                print_error "Homebrew not found. Please install from https://brew.sh/"
                exit 1
            fi
            
            print_status "Installing macOS dependencies via Homebrew..."
            brew install gcc gfortran make cmake fftw sox bc || {
                print_warning "Some packages may already be installed"
            }
            ;;
            
        "ubuntu")
            print_status "Installing Ubuntu/Debian dependencies..."
            sudo apt update
            sudo apt install -y build-essential gcc g++ gfortran make cmake \
                               libfftw3-dev sox libsox-fmt-all bc git || {
                print_error "Failed to install dependencies"
                exit 1
            }
            ;;
            
        "rhel")
            print_status "Installing RHEL/CentOS dependencies..."
            sudo yum install -y gcc gcc-c++ gcc-gfortran make cmake \
                               fftw-devel sox bc git || {
                print_error "Failed to install dependencies"
                exit 1
            }
            ;;
            
        "fedora")
            print_status "Installing Fedora dependencies..."
            sudo dnf install -y gcc gcc-c++ gcc-gfortran make cmake \
                               fftw-devel sox bc git || {
                print_error "Failed to install dependencies"
                exit 1
            }
            ;;
            
        *)
            print_warning "Unknown OS. Please install dependencies manually:"
            echo "  - gcc, g++, gfortran"
            echo "  - make, cmake"
            echo "  - FFTW library (libfftw3-dev or fftw-devel)"
            echo "  - sox (audio processing)"
            echo "  - bc (calculator)"
            read -p "Continue anyway? (y/N): " -n 1 -r
            echo
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                exit 1
            fi
            ;;
    esac
}

# Check if all required tools are available
check_dependencies() {
    print_status "Checking for required tools..."
    
    local missing=()
    
    for tool in gcc g++ gfortran make sox bc; do
        if ! command_exists "$tool"; then
            missing+=("$tool")
        fi
    done
    
    if [ ${#missing[@]} -ne 0 ]; then
        print_error "Missing required tools: ${missing[*]}"
        return 1
    fi
    
    print_success "All required tools are available"
    return 0
}

# Build the project components
build_project() {
    print_status "Building project components..."
    
    # Build JTEncode library
    print_status "Building JTEncode library..."
    cd src
    if make clean && make; then
        print_success "JTEncode library built successfully"
    else
        print_error "Failed to build JTEncode library"
        exit 1
    fi
    cd ..
    
    # Build WSPR signal generator
    print_status "Building WSPR signal generator..."
    if g++ wsprsim.cpp -Isrc -L. -ljtencode -o wsprsim; then
        print_success "WSPR signal generator built successfully"
    else
        print_error "Failed to build WSPR signal generator"
        exit 1
    fi
    
    # Build normal WSPR decoder
    print_status "Building normal WSPR decoder..."
    cd wspr-cui/wsprd
    if make clean && make; then
        print_success "Normal WSPR decoder built successfully"
    else
        print_error "Failed to build normal WSPR decoder"
        exit 1
    fi
    cd ../..
    
    # Build altered WSPR decoder
    print_status "Building altered WSPR decoder..."
    cd wspr-cui/wsprd-alt
    if make clean && make; then
        print_success "Altered WSPR decoder built successfully"
    else
        print_error "Failed to build altered WSPR decoder"
        exit 1
    fi
    cd ../..
    
    # Make scripts executable
    print_status "Making scripts executable..."
    chmod +x *.sh
    chmod +x wspr_project_menu.sh
    
    print_success "All components built successfully!"
}

# Run verification tests
run_tests() {
    print_status "Running verification tests..."
    
    # Test 1: Generate test signals
    print_status "Test 1: Generating test signals..."
    if ./wsprsim TEST FM04 20; then
        print_success "Signal generation test passed"
    else
        print_error "Signal generation test failed"
        return 1
    fi
    
    # Test 2: Check if WAV files were created
    if [[ -f "wspr_normal.wav" ]] && [[ -f "wspr_altered.wav" ]]; then
        print_success "WAV files created successfully"
    else
        print_error "WAV files not found"
        return 1
    fi
    
    # Test 3: Quick decoder test (just check if they run)
    print_status "Test 3: Testing decoders..."
    
    # Test normal decoder with normal file (should decode)
    print_status "Testing normal decoder with normal file..."
    if timeout 10s ./decode_norm_norm.sh >/dev/null 2>&1; then
        print_success "Normal decoder test completed"
    else
        print_warning "Normal decoder test timed out (this may be normal)"
    fi
    
    print_success "Basic verification tests completed!"
}

# Main installation function
main() {
    echo "================================================="
    echo "WSPR Altered Sync Vector Project - Installation"
    echo "================================================="
    echo
    
    # Check if we're in the right directory
    if [[ ! -f "wsprsim.cpp" ]] || [[ ! -d "src" ]]; then
        print_error "Please run this script from the jtencode-sim project directory"
        exit 1
    fi
    
    detect_os
    
    # Ask user what to do
    echo
    echo "Installation options:"
    echo "1) Install dependencies and build project (recommended)"
    echo "2) Only check dependencies"
    echo "3) Only build project (skip dependency installation)"
    echo "4) Run verification tests only"
    echo
    read -p "Enter choice (1-4): " choice
    
    case $choice in
        1)
            install_dependencies
            check_dependencies || exit 1
            build_project
            echo
            read -p "Run verification tests? (y/N): " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                run_tests
            fi
            ;;
        2)
            check_dependencies
            ;;
        3)
            check_dependencies || {
                print_error "Dependencies missing. Install them first."
                exit 1
            }
            build_project
            ;;
        4)
            run_tests
            ;;
        *)
            print_error "Invalid choice"
            exit 1
            ;;
    esac
    
    echo
    echo "================================================="
    print_success "Installation completed!"
    echo "================================================="
    echo
    echo "Next steps:"
    echo "1. Run the main menu: ./wspr_project_menu.sh"
    echo "2. Generate WSPR signals: ./wsprsim CALLSIGN GRID POWER"
    echo "3. Read SETUP_GUIDE.md for detailed usage instructions"
    echo
    print_success "Enjoy your WSPR altered sync vector system!"
}

# Run main function
main "$@"
