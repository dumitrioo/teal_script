rm -rf build
cmake -Wno-dev -S. -Bbuild
cmake --build build
rm -rf bin
mkdir bin
cp build/examples/interpreter/scaflux bin
strip bin/scaflux
cp build/examples/fractal/libfractal.so bin
