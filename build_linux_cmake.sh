rm -rf build
cmake -W no-dev -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j16
rm -rf bin
mkdir bin
cp build/examples/pendulum/pendulum bin
cp build/examples/interpreter/tealscript bin
cp build/examples/fractal/*.so bin
cp build/examples/zmq/*.so bin
cp build/examples/ray/*.so bin
