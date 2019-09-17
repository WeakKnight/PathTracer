# Build Instruction
## macOS
1. mkdir build
2. cd build
3. mkdir macos
4. cd macos
5. cmake -G  "Xcode" ../../

## Windows(Not Tested)
1. mkdir build
2. cd build
3. mkdir windows
4. cd windows
5. cmake -G "Visual Studio 14 Win64" ../../

## Unix(Not Tested)
1. mkdir build
2. cd build
3. mkdir unix
4. cd unix
5. cmake ../../

# Feature

* multi threaded via std::future
* realtime preview based on GL + GLFW
* debug tool based on IMGUI

# Structure

Main Tracing Logic Located in 
* src/raytracer.cpp
* src/objects.cpp
* src/materials.cpp
* src/lights.cpp

Program and ouput file will be generated in 
* bin/ 

