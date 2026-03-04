# MyVoxelEngine

A voxel rendering engine built with C++ and OpenGL, featuring procedurally generated terrain, dynamic lighting, and an immersive 3D environment.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/language-C%2B%2B-orange.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-4.x-green.svg)

## 🎬 Demo

![Demo](MyVoxelEngine.gif)

## ✨ Features

- **Procedural Terrain Generation** - Random terrain with realistic height maps and biomes
- **Player System** - Full player movement with jumping, climbing, and collision detection
- **Advanced Rendering**
  - Dynamic skybox with HDR support
  - Shadow mapping for realistic lighting
  - Chunk-based rendering for optimized performance
  - Texture atlas system for efficient GPU utilization
- **Physics & Collision** - Proper collision detection for interactive gameplay
- **ImGui Integration** - Real-time debug UI and system monitoring
- **GPU Monitoring** - NVIDIA GPU usage tracking via NVML
- **Configuration System** - JSON-based configuration for easy customization

## 🎮 Controls

- **WASD** - Move forward/backward/left/right
- **Space** - Jump
- **Mouse** - Look around
- **ESC** - Exit

## 📋 Requirements

### Dependencies
- **OpenGL 4.x** or higher
- **GLFW3** - Window and input handling
- **GLM** - OpenGL Mathematics library
- **CMake 3.10+** - Build system
- **C++17** compatible compiler (GCC/Clang recommended)
- **CUDA Toolkit** (optional) - For GPU monitoring features

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential cmake libglfw3-dev libgl1-mesa-dev
```

### Linux (Fedora)
```bash
sudo dnf install cmake glfw-devel mesa-libGL-devel
```

## 🚀 Installation

1. **Clone the repository with submodules:**
   ```bash
   git clone --recursive git@github.com:anurag-dalal/MyVoxelEngine.git
   cd MyVoxelEngine
   ```

2. **Build the project:**
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make -j$(nproc)
   ```

3. **Run from project root:**
   ```bash
   cd ..
   ./build/MyVoxelEngine
   ```

   > ⚠️ **Important:** Always run the executable from the project root directory to ensure shaders and assets are properly loaded.

## ⚙️ Configuration

Edit `Configs/config.json` to customize:
- Window resolution and fullscreen settings
- Camera settings (FOV, position, sensitivity)
- World generation parameters (width, depth, max height)
- Texture atlas configuration

Example configuration:
```json
{
  "window": {
    "width": 2560,
    "height": 1600,
    "title": "MyVoxelEngine"
  },
  "camera": {
    "fov": 70.0
  },
  "gridConfig": {
    "vox_width": 256,
    "vox_depth": 256,
    "vox_maxHeight": 64
  }
}
```

## 📁 Project Structure

```
MyVoxelEngine/
├── Assets/           # Textures and game assets
│   └── Clouds/      # Skybox textures
├── Configs/         # Configuration files
├── Include/         # Third-party libraries (GLM, GLAD, ImGui)
├── Shaders/         # GLSL shader files
├── Source/          # Source code
│   ├── Models/      # 3D models and trees
│   ├── Player/      # Player controller
│   ├── Sky/         # Skybox rendering
│   ├── Utils/       # Utilities (config reader, height maps, etc.)
│   └── World/       # World generation and chunk management
└── build/          # Build output directory
```

## 🐛 Troubleshooting

### Black Screen or Shader Errors
If you see `ERROR::SHADER::VERTEX::FILE_NOT_SUCCESFULLY_READ`, make sure you're running the executable from the project root:

```bash
cd /path/to/MyVoxelEngine
./build/MyVoxelEngine
```

### Failed to Find Safe Spawn Position
This warning occurs when terrain generation creates an unsafe spawn. The engine will retry automatically.

## 📚 Resources

### Texture Assets
- **Tileset:** [Atlas Resource Pack](https://www.9minecraft.net/atlas-resource-pack/)

### Skybox Creation
- **HDR Images:** [Poly Haven HDRI Skies](https://polyhaven.com/hdris/skies)
- **HDR to PNG Converter:** [Convertio](https://convertio.co/hdr-png/)
- **Panorama to Cubemap:** [Cubemap Converter](https://jaxry.github.io/panorama-to-cubemap/)

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest new features
- Submit pull requests

## Demo

## 📝 License

This project is open source. Please check the repository for license details.

## 🙏 Acknowledgments

- [GLFW](https://www.glfw.org/) - Window and input handling
- [GLM](https://github.com/g-truc/glm) - OpenGL Mathematics
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI
- [stb_image](https://github.com/nothings/stb) - Image loading
- Poly Haven for high-quality HDRI resources

---

**Made with ❤️ using C++ and OpenGL**

