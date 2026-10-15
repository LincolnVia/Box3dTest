# TrenchBroom Compile Profile

## Profile

- Name: `Build BSP and Run Box3D`
- Working directory: `C:\Users\Lincoln Via\Desktop\Utils\trench\compile`

## Tasks

### 1. Export Map

- Target: `${MAP_BASE_NAME}-compile.map`
- Strip TrenchBroom properties: enabled

### 2. Run Tool — QBSP

- Tool: `C:\Users\Lincoln Via\Desktop\Utils\trench\compilers\qbsp.exe`
- Parameters: `-notex -missing_textures_as_zero_size ${MAP_BASE_NAME}-compile.map ${MAP_BASE_NAME}.bsp`
- Treat non-zero exit as error: enabled

### 3. Run Tool — VIS

- Tool: `C:\Users\Lincoln Via\Desktop\Utils\trench\compilers\vis.exe`
- Parameters: `${MAP_BASE_NAME}.bsp`
- Treat non-zero exit as error: enabled

### 4. Run Tool — LIGHT

- Tool: `C:\Users\Lincoln Via\Desktop\Utils\trench\compilers\light.exe`
- Parameters: `${MAP_BASE_NAME}.bsp`
- Treat non-zero exit as error: enabled

### 5. Launch Engine

- Engine profile: `Box3D Game`
- Treat launch failure as error: enabled

## Engine Profile

- Name: `Box3D Game`
- Executable: `C:\Windows\System32\wsl.exe`
- Parameters: `--distribution Ubuntu --cd /home/lincoln_via/projects/Box3dTest --exec ./build/windows-dev/Box3dGame.exe "C:/Users/Lincoln Via/Desktop/Utils/trench/compile/${MAP_BASE_NAME}.bsp"`

For a build-only profile, omit task 5.
