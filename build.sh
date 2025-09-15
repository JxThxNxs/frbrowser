#!/bin/bash

# FR Browser Build Script
# Supports both 32-bit and 64-bit compilation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"

# Default values
BUILD_TYPE="Release"
ARCHITECTURE="64"
CLEAN_BUILD=false
INSTALL=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -a, --arch ARCH      Architecture (32 or 64, default: 64)"
    echo "  -t, --type TYPE      Build type (Debug or Release, default: Release)"
    echo "  -c, --clean          Clean build directory before building"
    echo "  -i, --install        Install after building"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                   # Build 64-bit release"
    echo "  $0 -a 32            # Build 32-bit release"
    echo "  $0 -t Debug         # Build 64-bit debug"
    echo "  $0 -a 32 -t Debug   # Build 32-bit debug"
    echo "  $0 -c -i            # Clean build and install"
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    log_info "Checking dependencies..."
    
    local missing_deps=()
    
    # Check for required tools
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if ! command -v gcc &> /dev/null; then
        missing_deps+=("gcc")
    fi
    
    if ! command -v pkg-config &> /dev/null; then
        missing_deps+=("pkg-config")
    fi
    
    # Check for required libraries
    if ! pkg-config --exists gtk+-3.0; then
        missing_deps+=("libgtk-3-dev")
    fi
    
    # Check for WebKit2GTK (try different versions)
    if ! pkg-config --exists webkit2gtk-4.1 && ! pkg-config --exists webkit2gtk-4.0; then
        # Determine which package to suggest based on what's available
        if apt list libwebkit2gtk-4.1-dev 2>/dev/null | grep -q "libwebkit2gtk-4.1-dev"; then
            missing_deps+=("libwebkit2gtk-4.1-dev")
        else
            missing_deps+=("libwebkit2gtk-4.0-dev")
        fi
    fi
    
    if ! pkg-config --exists json-glib-1.0; then
        missing_deps+=("libjson-glib-dev")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        echo ""
        echo "On Ubuntu/Debian, install with:"
        echo "sudo apt-get install build-essential cmake pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev libjson-glib-dev"
        echo ""
        echo "On Fedora/RHEL, install with:"
        echo "sudo dnf install gcc gcc-c++ cmake pkg-config gtk3-devel webkit2gtk3-devel json-glib-devel"
        exit 1
    fi
    
    log_success "All dependencies found"
}

parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -a|--arch)
                ARCHITECTURE="$2"
                if [[ "$ARCHITECTURE" != "32" && "$ARCHITECTURE" != "64" ]]; then
                    log_error "Invalid architecture: $ARCHITECTURE. Must be 32 or 64."
                    exit 1
                fi
                shift 2
                ;;
            -t|--type)
                BUILD_TYPE="$2"
                if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
                    log_error "Invalid build type: $BUILD_TYPE. Must be Debug or Release."
                    exit 1
                fi
                shift 2
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            -i|--install)
                INSTALL=true
                shift
                ;;
            -h|--help)
                print_usage
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
        esac
    done
}

build_project() {
    local build_dir="build-${ARCHITECTURE}bit"
    
    log_info "Building FR Browser (${ARCHITECTURE}-bit, ${BUILD_TYPE})"
    
    # Create build directory
    if [ "$CLEAN_BUILD" = true ] && [ -d "$build_dir" ]; then
        log_info "Cleaning build directory..."
        rm -rf "$build_dir"
    fi
    
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    # Configure CMake
    local cmake_args="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    
    if [ "$ARCHITECTURE" = "32" ]; then
        cmake_args="$cmake_args -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32"
        log_info "Configuring for 32-bit build..."
    else
        log_info "Configuring for 64-bit build..."
    fi
    
    cmake $cmake_args ..
    
    # Build
    log_info "Compiling..."
    make -j$(nproc)
    
    log_success "Build completed successfully!"
    
    # Install if requested
    if [ "$INSTALL" = true ]; then
        log_info "Installing..."
        sudo make install
        log_success "Installation completed!"
    fi
    
    cd ..
}

main() {
    log_info "FR Browser Build System"
    echo "=========================="
    
    parse_args "$@"
    check_dependencies
    build_project
    
    echo ""
    log_success "Build process completed!"
    echo ""
    echo "To run FR Browser:"
    echo "  cd build-${ARCHITECTURE}bit"
    echo "  ./fr-browser"
    echo ""
    if [ "$INSTALL" = false ]; then
        echo "To install system-wide, run:"
        echo "  $0 -i"
    fi
}

main "$@"
