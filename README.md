# FR Browser from JxThxNxs

A lightweight, fast, and open-source web browser designed for Linux systems.

## Features

- Lightweight and fast performance
- Minimal resource usage
- Cross-platform compatibility (32-bit and 64-bit Linux)
- Modern web standards support
- Tabbed browsing
- Bookmarks and history
- Clean, intuitive interface

## Building

### Prerequisites

- GCC or Clang compiler
- CMake 3.10 or higher
- GTK+ 3.0 development libraries
- WebKit2GTK development libraries
- pkg-config

### Ubuntu/Debian Dependencies

```bash
sudo apt-get update
sudo apt-get install build-essential cmake pkg-config
sudo apt-get install libgtk-3-dev libwebkit2gtk-4.1-dev libjson-glib-dev
```

### Fedora/RHEL Dependencies

```bash
sudo dnf install gcc gcc-c++ cmake pkg-config
sudo dnf install gtk3-devel webkit2gtk3-devel
```

### Building for 64-bit

```bash
mkdir build-64
cd build-64
cmake ..
make -j$(nproc)
```
(or just run the build script)
### Building for 32-bit

```bash
mkdir build-32
cd build-32
cmake -DCMAKE_C_FLAGS="-m32" -DCMAKE_CXX_FLAGS="-m32" ..
make -j$(nproc)
```
(or just run the build script)
## Running

```bash
./fr-browser
```

## License

MIT License - see LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues. 
(Issue with the bad Icon readability and the only sometimes working logo is allready known)
