Markdown# ImageEditor

A versatile Qt6-based image processing tool with JSON-history support, designed to run on **Debian Linux** and **macOS**. It supports both a Graphical User Interface and a headless batch mode for server environments.

## Features
- **Batch Processing**: Run transformations via CLI without a GUI.
- **JSON Project Support**: Load and apply transformation histories from JSON files.
- **Offscreen Optimized**: Perfect for headless servers using the `minimal` platform plugin.
- **High-Res Ready**: Automatically handles large image allocations (>128MB).
- **Vulkan Support**: Optional hardware acceleration for rendering.

---

## Prerequisites

### 🐧 Linux (Debian 12 / Bookworm)
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-base-dev-tools cmake g++ build-essential libvulkan-dev
🍎 macOSBashbrew install qt cmake
Note: If CMake fails to find Qt, run: export CMAKE_PREFIX_PATH=$(brew --prefix qt)Build ProcessWe recommend an "out-of-source" build to keep the repository clean:Enter the project directory:cd /path/to/your/ImageEditorCreate a build folder:mkdir build && cd buildConfigure and Compile:Bashcmake ..
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
Usage & CLI OptionsTo run the editor without a GUI (e.g., via SSH on your Debian server), use the --batch flag combined with the -platform minimal Qt option.Command Line ArgumentsOptionDescription-f, --file <file>Path to the input image file.--project <json>Path to an input JSON-project file (history).-o, --output <file>Path to the output image file.--batchRun in batch mode (no GUI).--vulkanEnable hardware accelerated Vulkan rendering.--historyPrint history of last calls to stdout.-v, --versionDisplays version information.-h, --helpDisplays help options.ExamplesConvert an image to grayscale (Offscreen):Bash./ImageEditor -platform minimal --batch -f photo.jpg -o output.png
Apply a JSON transformation project:Bash./ImageEditor -platform minimal --batch --project task.json -o result.png
Technical NotesCross-Platform CompatibilityThe codebase automatically handles the transition from mirrored() (older Qt6) to flipped() (Qt 6.7+) to ensure warning-free compilation on both Debian and macOS:C++#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    image = image.flipped(Qt::Vertical);
#else
    image = image.mirrored(false, true);
#endif
Headless ExecutionWhen running on a server without an X11/Wayland session, always append -platform minimal to the command to prevent QPA connection errors.
