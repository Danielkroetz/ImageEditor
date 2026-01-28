# ImageEditor

Markdown
# Qt6 Offscreen Image Transformer

A high-performance C++ tool for offscreen image transformation and grayscale conversion, optimized for both **Debian Linux** and **macOS**.

## Features
- **Offscreen Rendering**: Operates without an X-Server or Display (ideal for servers and CLI environments).
- **Qt6 Native**: Uses the latest Qt 6 API for stable image manipulations.
- **High-Resolution Support**: Automatically bypasses the default 128MB allocation limit for images > 5700x5700px.
- **Cross-Platform**: Fully compatible with Debian (Bookworm) and macOS (Intel/Apple Silicon).

---

## Usage

The `ImageEditor` can be used both as a GUI application and as a command-line tool for automated batch processing.

### Batch Mode (Offscreen)
For server environments or automated scripts, use the `--batch` flag combined with the `-platform minimal` plugin to run without a display.

**Basic Image Conversion:**
```bash
./ImageEditor -platform minimal --batch -f input.jpg -o output.png

**Using JSON Projects:**
  You can apply a history of transformations by providing a JSON project file:
    ./ImageEditor -platform minimal --batch --project transformations.json -o result.png

**Displaying Information:**
  To see the history of recent calls: ./ImageEditor --history
  To check the version: ./ImageEditor -v

**Command Line Options**
 Option   Description
 -f, --file <file>Path to the input image file.
 --project <json>Path to an input JSON project file (transformation history).
 -o, --output <file>Path where the processed image will be saved.
 --batchRequired for CLI. Runs without the graphical user interface.
 --historyPrints the history of previous operations to stdout.
 --vulkanEnables hardware-accelerated Vulkan rendering (if supported).

## Prerequisites

Before compiling, ensure you have the Qt6 development environment and CMake installed.

### 🐧 Linux (Debian 12 / Bookworm)
Debian 6.1 (Bookworm) uses modular Qt6 packages. Install the required development tools:
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-base-dev-tools cmake g++ build-essential
🍎 macOS
On macOS, installation via Homebrew is recommended:

Bash
brew install qt cmake
Note: If CMake fails to find Qt, set the path: export CMAKE_PREFIX_PATH=$(brew --prefix qt)

Build Process
The project uses CMake as the build system. We recommend an "out-of-source" build:

Enter the project directory:

Bash
cd /path/to/your/project
Create a build folder:

Bash
mkdir build && cd build
Configure the project:

Bash
cmake ..
Compile:

Bash
# Use all available CPU cores for faster compilation
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
Usage
Since this tool is designed for server environments without a monitor, you must tell Qt to use the "minimal" platform plugin.

Standard Command:

Bash
./ImageEditor --input input.png --output output.png
Alternative via Environment Variable:

Bash
export QT_QPA_PLATFORM=minimal
./image_transformer --input input.png
Technical Details
Cross-Platform Image Flipping
The tool handles API differences between older Qt 6 versions (Linux) and newer versions (macOS) regarding the transition from mirrored() to flipped():

C++
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    image = image.flipped(Qt::Vertical);
#else
    image = image.mirrored(false, true);
#endif
Memory Management
To handle large-scale images, the allocation limit is increased at runtime:

C++
QImageReader::setAllocationLimit(512); // Increases limit to 512MB
Project Structure
src/ - C++ Source and Header files.

CMakeLists.txt - Build configuration.

.gitignore - Excludes build artifacts and system-specific files (like .DS_Store).

README.md - This documentation.

Troubleshooting
Display Connection Error: If you see qt.qpa.xcb: could not connect to display, ensure you are using the --batch flag.

