if not exist build_Windows mkdir build_Windows
cd build_Windows
cmake -S ../ -B . -G "MinGW Makefiles"
mingw32-make.exe && mingw32-make.exe Shaders
start "" "AVKEngine.exe"
cd ..