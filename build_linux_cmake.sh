rm -rf build
cmake -Wno-dev -S. -Bbuild
cmake --build build
rm -rf bin
mkdir bin
cp build/examples/interpreter/scaflux bin
cp build/examples/fractal/*.so bin
