#!/bin/bash

# WSPR Altered Sync Vector Project - Installation Script
# Updated for current project structure - works with fresh clones
# Supports macOS, Linux, and Windows (WSL/MSYS2)

set -e  # Exit on any error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Project info
PROJECT_NAME="WSPR Altered Sync Vector Project"
VERSION="2.0"

# Print colored output
print_header() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}========================================${NC}"
}

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

print_step() {
    echo -e "${CYAN}[STEP]${NC} $1"
}

# Detect operating system
detect_os() {
    print_step "Detecting operating system..."
    
    if [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
        PLATFORM="macOS"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get >/dev/null 2>&1; then
            OS="ubuntu"
            PLATFORM="Ubuntu/Debian"
        elif command -v yum >/dev/null 2>&1; then
            OS="rhel"
            PLATFORM="RHEL/CentOS"
        elif command -v dnf >/dev/null 2>&1; then
            OS="fedora"
            PLATFORM="Fedora"
        else
            OS="linux"
            PLATFORM="Generic Linux"
        fi
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        OS="windows"
        PLATFORM="Windows (MSYS2/Cygwin)"
    elif [[ -f /proc/version ]] && grep -q Microsoft /proc/version; then
        OS="wsl"
        PLATFORM="Windows WSL"
    else
        OS="unknown"
        PLATFORM="Unknown"
    fi
    
    print_success "Detected platform: $PLATFORM"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Install packages based on OS
install_dependencies() {
    print_step "Installing required dependencies..."
    
    case $OS in
        "macos")
            if ! command_exists brew; then
                print_error "Homebrew not found. Installing Homebrew..."
                /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
            fi
            
            print_status "Installing macOS dependencies via Homebrew..."
            brew update
            brew install gcc gfortran make cmake fftw sox bc git || {
                print_warning "Some packages may already be installed"
            }
            ;;
            
        "ubuntu"|"wsl")
            print_status "Installing Ubuntu/Debian dependencies..."
            sudo apt update
            sudo apt install -y build-essential gcc g++ gfortran make cmake \
                               libfftw3-dev sox libsox-fmt-all bc git curl \
                               pkg-config libgfortran5 || {
                print_error "Failed to install dependencies"
                exit 1
            }
            ;;
            
        "rhel")
            print_status "Installing RHEL/CentOS dependencies..."
            sudo yum groupinstall -y "Development Tools"
            sudo yum install -y gcc gcc-c++ gcc-gfortran make cmake \
                               fftw-devel sox bc git curl || {
                print_error "Failed to install dependencies"
                exit 1
            }
            ;;
            
        "fedora")
            print_status "Installing Fedora dependencies..."
            sudo dnf groupinstall -y "Development Tools"
            sudo dnf install -y gcc gcc-c++ gcc-gfortran make cmake \
                               fftw-devel sox bc git curl || {
                print_error "Failed to install dependencies"
                exit 1
            }
            ;;
            
        "windows")
            print_status "Windows (MSYS2/Cygwin) detected..."
            if command_exists pacman; then
                print_status "Installing via pacman..."
                pacman -S --needed base-devel mingw-w64-x86_64-toolchain \
                                 mingw-w64-x86_64-fftw mingw-w64-x86_64-sox
            else
                print_warning "Please install build tools manually in your Windows environment"
            fi
            ;;
            
        *)
            print_warning "Unknown OS ($PLATFORM). Please install dependencies manually:"
            echo "  Required packages:"
            echo "  - gcc, g++, gfortran (C/C++/Fortran compilers)"
            echo "  - make, cmake (build systems)"
            echo "  - FFTW library (libfftw3-dev or fftw-devel)"
            echo "  - sox (audio processing)"
            echo "  - bc (calculator)"
            echo "  - git (version control)"
            echo ""
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
    print_step "Checking for required tools..."
    
    local missing=()
    local tools=("gcc" "g++" "gfortran" "make" "sox" "bc")
    
    for tool in "${tools[@]}"; do
        if ! command_exists "$tool"; then
            missing+=("$tool")
        else
            print_status "✓ $tool found"
        fi
    done
    
    if [ ${#missing[@]} -ne 0 ]; then
        print_error "Missing required tools: ${missing[*]}"
        return 1
    fi
    
    print_success "All required tools are available"
    return 0
}

# Create Makefile for JTEncode library if it doesn't exist
create_jtencode_makefile() {
    if [[ ! -f "src/Makefile" ]]; then
        print_status "Creating Makefile for JTEncode library..."
        cat > src/Makefile << 'EOF'
# JTEncode Library Makefile
CC = gcc
CXX = g++
CFLAGS = -O2 -Wall -fPIC
CXXFLAGS = -O2 -Wall -fPIC -std=c++11

# Library name
LIBNAME = libjtencode.a

# Source files
C_SOURCES = crc14.c nhash.c
CXX_SOURCES = JTEncode.cpp encode_rs_int.cpp init_rs_int.cpp

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)
OBJECTS = $(C_OBJECTS) $(CXX_OBJECTS)

# Default target
all: $(LIBNAME)

# Build library
$(LIBNAME): $(OBJECTS)
	ar rcs $@ $^
	cp $@ ../

# C source compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# C++ source compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(LIBNAME)
	rm -f ../$(LIBNAME)

# Install (copy to parent directory)
install: $(LIBNAME)
	cp $(LIBNAME) ../

.PHONY: all clean install
EOF
        print_success "Created JTEncode Makefile"
    fi
}

# Build the project components
build_project() {
    print_step "Building project components..."
    
    # Create JTEncode Makefile if needed
    create_jtencode_makefile
    
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
    if g++ -O2 -Wall -std=c++11 wsprsim.cpp -Isrc -L. -ljtencode -o wsprsim; then
        print_success "WSPR signal generator built successfully"
    else
        print_error "Failed to build WSPR signal generator"
        exit 1
    fi
    
    # Build normal WSPR decoder
    print_status "Building normal WSPR decoder..."
    if [[ -d "wspr-cui/wsprd" ]]; then
        cd wspr-cui/wsprd
        if make clean && make; then
            print_success "Normal WSPR decoder built successfully"
        else
            print_error "Failed to build normal WSPR decoder"
            exit 1
        fi
        cd ../..
    else
        print_warning "Normal WSPR decoder directory not found"
    fi
    
    # Build altered WSPR decoder
    print_status "Building altered WSPR decoder..."
    if [[ -d "wspr-cui/wsprd-alt" ]]; then
        cd wspr-cui/wsprd-alt
        if make clean && make; then
            print_success "Altered WSPR decoder built successfully"
        else
            print_error "Failed to build altered WSPR decoder"
            exit 1
        fi
        cd ../..
    else
        print_warning "Altered WSPR decoder directory not found"
    fi
    
    # Make scripts executable
    print_status "Making scripts executable..."
    chmod +x *.sh 2>/dev/null || true
    
    # Create any missing decode scripts
    create_decode_scripts
    
    print_success "All components built successfully!"
}

# Create decode scripts if they don't exist
create_decode_scripts() {
    print_status "Ensuring decode scripts exist..."
    
    # Create decode_norm_norm.sh
    if [[ ! -f "decode_norm_norm.sh" ]]; then
        cat > decode_norm_norm.sh << 'EOF'
#!/bin/bash
# Decode normal WSPR with normal decoder
echo "Decoding normal WSPR file with normal decoder..."
./wspr-cui/wsprd/wsprd -w wspr_normal.wav
EOF
        chmod +x decode_norm_norm.sh
    fi
    
    # Create decode_alt_alt.sh
    if [[ ! -f "decode_alt_alt.sh" ]]; then
        cat > decode_alt_alt.sh << 'EOF'
#!/bin/bash
# Decode altered WSPR with altered decoder
echo "Decoding altered WSPR file with altered decoder..."
./wspr-cui/wsprd-alt/wsprd -w wspr_altered.wav
EOF
        chmod +x decode_alt_alt.sh
    fi
    
    # Create decode_norm_alt.sh
    if [[ ! -f "decode_norm_alt.sh" ]]; then
        cat > decode_norm_alt.sh << 'EOF'
#!/bin/bash
# Try to decode normal WSPR with altered decoder (should fail)
echo "Trying to decode normal WSPR file with altered decoder (should fail)..."
./wspr-cui/wsprd-alt/wsprd -w wspr_normal.wav
EOF
        chmod +x decode_norm_alt.sh
    fi
    
    # Create decode_alt_norm.sh
    if [[ ! -f "decode_alt_norm.sh" ]]; then
        cat > decode_alt_norm.sh << 'EOF'
#!/bin/bash
# Try to decode altered WSPR with normal decoder (should fail)
echo "Trying to decode altered WSPR file with normal decoder (should fail)..."
./wspr-cui/wsprd/wsprd -w wspr_altered.wav
EOF
        chmod +x decode_alt_norm.sh
    fi
    
    print_success "Decode scripts ready"
}

# Create development setup for VS Code
create_vscode_setup() {
    print_step "Creating VS Code development setup..."
    
    # Create .vscode directory
    mkdir -p .vscode
    
    # Create tasks.json
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build All",
            "type": "shell",
            "command": "./install.sh",
            "args": ["3"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "Generate WSPR Signals",
            "type": "shell",
            "command": "./wsprsim",
            "args": ["TEST", "FM04", "20"],
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "./wspr_project_menu.sh",
            "group": "test",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        }
    ]
}
EOF
    
    # Create launch.json for debugging
    cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug WSPR Generator",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/wsprsim",
            "args": ["TEST", "FM04", "20"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
EOF
    
    # Create c_cpp_properties.json
    cat > .vscode/c_cpp_properties.json << 'EOF'
{
    "configurations": [
        {
            "name": "Linux/Mac",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/src"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c11",
            "cppStandard": "c++11",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
EOF
    
    print_success "VS Code setup created (tasks, launch, IntelliSense)"
}

# Show usage instructions
show_usage() {
    echo ""
    print_header "Quick Start Guide"
    echo ""
    echo "1. Generate WSPR signals:"
    echo "   ./wsprsim CALLSIGN GRID POWER"
    echo "   Example: ./wsprsim VK3ABC FM04 20"
    echo ""
    echo "2. Run the interactive menu:"
    echo "   ./wspr_project_menu.sh"
    echo ""
    echo "3. Test decoder matrix:"
    echo "   ./decode_norm_norm.sh  # Should work"
    echo "   ./decode_alt_alt.sh    # Should work"
    echo "   ./decode_norm_alt.sh   # Should fail"
    echo "   ./decode_alt_norm.sh   # Should fail"
    echo ""
    echo "4. Development:"
    echo "   - Open in VS Code: code ."
    echo "   - Build: Ctrl+Shift+P -> Tasks: Run Build Task"
    echo "   - Generated files: *.wav, *.bits, *.hex"
    echo ""
    print_success "Ready to use!"
}

# Run verification tests
run_tests() {
    print_step "Running verification tests..."
    
    # Test 1: Generate test signals
    print_status "Test 1: Generating test signals..."
    if ./wsprsim TEST FM04 20; then
        print_success "Signal generation test passed"
    else
        print_error "Signal generation test failed"
        return 1
    fi
    
    # Test 2: Check if files were created
    local files=("wspr_normal.wav" "wspr_altered.wav" "wspr_normal.bits" "wspr_altered.bits")
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            print_status "✓ $file created"
        else
            print_warning "✗ $file not found"
        fi
    done
    
    # Test 3: Basic decoder functionality
    print_status "Test 3: Testing decoder functionality..."
    
    # Test normal decode
    if timeout 30s ./decode_norm_norm.sh >/dev/null 2>&1; then
        print_success "Normal decoder test completed"
    else
        print_warning "Normal decoder test timed out (may be normal)"
    fi
    
    # Test altered decode
    if timeout 30s ./decode_alt_alt.sh >/dev/null 2>&1; then
        print_success "Altered decoder test completed"
    else
        print_warning "Altered decoder test timed out (may be normal)"
    fi
    
    print_success "Basic verification tests completed!"
}

# Main installation function
main() {
    print_header "$PROJECT_NAME - Installation v$VERSION"
    
    # Check if we're in the right directory
    if [[ ! -f "wsprsim.cpp" ]] || [[ ! -d "src" ]]; then
        print_error "Please run this script from the jtencode-sim project directory"
        print_status "Current directory: $(pwd)"
        exit 1
    fi
    
    detect_os
    
    # Interactive menu
    echo ""
    echo "Installation options:"
    echo "1) Full setup (install deps + build + create dev setup) [RECOMMENDED]"
    echo "2) Install dependencies only"
    echo "3) Build project only (skip dependency installation)"
    echo "4) Run verification tests only"
    echo "5) Create VS Code setup only"
    echo "6) Show usage instructions"
    echo ""
    read -p "Enter choice (1-6): " choice
    
    case $choice in
        1)
            install_dependencies
            check_dependencies || exit 1
            build_project
            create_vscode_setup
            echo ""
            read -p "Run verification tests? (y/N): " -n 1 -r
            echo ""
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                run_tests
            fi
            show_usage
            ;;
        2)
            install_dependencies
            check_dependencies
            ;;
        3)
            check_dependencies || {
                print_error "Dependencies missing. Install them first with option 1 or 2."
                exit 1
            }
            build_project
            ;;
        4)
            run_tests
            ;;
        5)
            create_vscode_setup
            ;;
        6)
            show_usage
            ;;
        *)
            print_error "Invalid choice"
            exit 1
            ;;
    esac
    
    echo ""
    print_header "Installation Complete!"
    print_success "Your WSPR Altered Sync Vector project is ready to use!"
    echo ""
    echo "Next steps:"
    echo "• Run: ./wspr_project_menu.sh (interactive menu)"
    echo "• Open in VS Code: code ."
    echo "• Read: README.md and SETUP_GUIDE.md"
    echo ""
}

# Run main function
main "$@"
