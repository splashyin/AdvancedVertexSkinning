# Advanced Vertex Skinning

**Master's Study Project - Computer Graphics Research Implementation**

This project implements and demonstrates advanced vertex skinning techniques, specifically focusing on the research published in:

> **"Improved Vertex Skinning Algorithm Based On Dual Quaternions"**  
> *Hao Yin, Ramakrishnan Mukundan*  
> 2019 IEEE 21st International Workshop on Multimedia Signal Processing (MMSP)  
> DOI: [10.1109/MMSP.2019.8901717](https://ieeexplore.ieee.org/abstract/document/8901717)

## Overview

Character animation in computer graphics relies heavily on vertex skinning algorithms to deform 3D models in real-time. This project provides a comprehensive implementation and comparison of traditional skinning methods alongside the improved algorithm presented in the referenced research.

### Research Problem

Traditional skinning algorithms suffer from well-known artifacts:
- **Linear Blend Skinning (LBS)**: Causes volume loss and the "candy wrapper" effect
- **Dual Quaternion Skinning (DQS)**: Produces bulging artifacts in certain deformation scenarios

### Research Solution

This implementation demonstrates the improved vertex skinning algorithm that combines LBS and DQS using a deform factor approach, effectively reducing artifacts while maintaining computational efficiency.

## Features

### 🔬 **Research Implementation**
- Real-time implementation of the improved vertex skinning algorithm
- Interactive comparison between LBS, DQS, and the hybrid approach
- Visual demonstration of artifact reduction techniques

### 🎮 **Interactive Controls**
- **ImGui Interface**: Real-time parameter adjustment
- **Skinning Method Toggle**: Switch between LBS and DQS
- **Blend Ratio Control**: Adjust the deform factor (0.0 to 1.0)
- **Real-time Animation**: Observe skinning effects during animation

### 🛠 **Technical Features**
- **OpenGL 3.2**: Core profile with custom vertex shaders (macOS compatible)
- **Cross-platform**: CMake build system for Windows, Linux, and macOS
- **3D Model Loading**: Assimp integration for FBX model support
- **Real-time Performance**: Optimized for interactive frame rates

## Technical Architecture

### Core Components

- **Application.cpp**: Main application loop and rendering pipeline
- **Shader System**: Custom GLSL shaders implementing skinning algorithms
- **Model Loading**: Assimp-based 3D model and animation loading
- **GUI Interface**: ImGui-based parameter control system

### Skinning Implementation

The vertex shader implements three approaches:

1. **Linear Blend Skinning (LBS)**
   ```glsl
   mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0] + 
                        gBones[BoneIDs[1]] * Weights[1] + 
                        gBones[BoneIDs[2]] * Weights[2] + 
                        gBones[BoneIDs[3]] * Weights[3];
   ```

2. **Dual Quaternion Skinning (DQS)**
   ```glsl
   mat2x4 blendDQ = dqs[BoneIDs[0]] * Weights[0] + 
                    dqs[BoneIDs[1]] * Weights[1] + 
                    dqs[BoneIDs[2]] * Weights[2] + 
                    dqs[BoneIDs[3]] * Weights[3];
   ```

3. **Improved Hybrid Approach**
   - Combines both methods using a ratio-based deform factor
   - Automatically selects optimal blending based on deformation characteristics

## Dependencies

- **OpenGL 3.2+**: Core graphics API (macOS compatible)
- **GLFW**: Window management and input handling
- **GLEW**: OpenGL extension loading
- **Assimp**: 3D model loading and processing
- **GLM**: OpenGL mathematics library
- **ImGui**: Immediate mode GUI for parameter control

## Building the Project

### Prerequisites

This project uses CMake for cross-platform building and requires the following dependencies:

**macOS:**
```bash
# Using Homebrew
brew install cmake glfw glew assimp

# Verify installations
brew list glfw glew assimp
```

**Linux (Ubuntu/Debian):**
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install cmake libglfw3-dev libglew-dev libassimp-dev

# Additional packages for OpenGL
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
```

**Windows:**
```bash
# Using vcpkg (recommended)
vcpkg install glfw3 glew assimp

# Tell CMake where to find vcpkg packages
cmake -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake ..

# Or install manually to dependencies/ or libs/ directory
# CMake will automatically search these locations
```

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/splashyin/AdvancedVertexSkinning.git
cd AdvancedVertexSkinning

# Create build directory
mkdir build
cd build

# Generate build files (CMake will automatically find dependencies)
cmake ..

# Build the project
cmake --build .         # Cross-platform

# Or use platform-specific build tools
make                    # macOS/Linux with Make
# or
msbuild AdvancedVertexSkinning.sln  # Windows with Visual Studio

# Run the application
./AdvancedVertexSkinning           # macOS/Linux
# or
.\Debug\AdvancedVertexSkinning.exe # Windows
```

### CMake Features

The CMake build system includes:
- **Automatic dependency detection**: Searches standard installation paths for GLFW, GLEW, and Assimp
- **Cross-platform support**: Works on Windows, macOS, and Linux
- **Flexible library paths**: Supports Homebrew (macOS), vcpkg (Windows), and system libraries (Linux)
- **Automatic resource copying**: Shader files are automatically copied to the build directory
- **Resource directory setup**: Creates necessary directories for 3D models

### Troubleshooting

If CMake cannot find dependencies:

**macOS:**
```bash
# Ensure Homebrew packages are linked
brew link glfw glew assimp

# Check installation paths
brew --prefix glfw
brew --prefix glew
brew --prefix assimp
```

**Linux:**
```bash
# Verify package installation
dpkg -l | grep -E "glfw|glew|assimp"

# Check library paths
ldconfig -p | grep -E "glfw|glew|assimp"
```

**Windows:**
```bash
# Verify vcpkg installation
vcpkg list

# Or manually specify library paths in CMake
cmake -DGLFW_INCLUDE_DIR=path/to/include -DGLFW_LIBRARY=path/to/lib ..
```

## Usage

### Controls

- **Mouse**: Camera rotation and navigation
- **Scroll**: Zoom in/out
- **GUI Panel**: 
  - Toggle LBS/DQS methods
  - Adjust blend ratio (deform factor)
  - Modify rendering parameters

### Loading Models

1. Place your animated FBX model in `res/object/body/`
2. Update the model path in `Application.cpp`
3. Rebuild and run the application

**Note**: The current implementation expects models with skeletal animation data.

## Academic Context

This project serves as a practical demonstration of computer graphics research in the field of character animation. The implementation validates the theoretical contributions presented in the IEEE MMSP 2019 paper, showing how academic research can be translated into working software systems.

### Key Research Contributions

1. **Hybrid Skinning Algorithm**: Combines strengths of both LBS and DQS
2. **Artifact Reduction**: Addresses volume loss and bulging issues
3. **Real-time Performance**: Maintains interactive frame rates
4. **Practical Implementation**: Demonstrates feasibility for game engines and animation software

## Future Work

- Integration with modern game engines (Unity, Unreal)
- GPU compute shader optimization
- Machine learning-based deform factor prediction
- Extended evaluation with diverse character models

## Publication

This work is based on the research published in:

```bibtex
@inproceedings{yin2019improved,
  title={Improved Vertex Skinning Algorithm Based On Dual Quaternions},
  author={Yin, Hao and Mukundan, Ramakrishnan},
  booktitle={2019 IEEE 21st International Workshop on Multimedia Signal Processing (MMSP)},
  year={2019},
  organization={IEEE},
  doi={10.1109/MMSP.2019.8901717}
}
```

## License

This project is developed for academic and educational purposes as part of master's study research.

---

*For questions about the research or implementation, please refer to the published paper or contact the authors.*