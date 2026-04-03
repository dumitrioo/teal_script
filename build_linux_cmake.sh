rm -rf build
cmake -Wno-dev -S. -Bbuild
cmake --build build -j8
rm -rf bin
mkdir bin
cp build/examples/interpreter/tealscript bin
cp build/examples/fractal/*.so bin
cp build/examples/zmq/*.so bin
cp build/examples/ray/*.so bin
