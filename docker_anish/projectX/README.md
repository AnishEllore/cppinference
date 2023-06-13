# Steps
1. Download libmicrohttpd-dev or directly from the git repo
2. Download libhttpserver
3. Download libtorch C++ 11 CPU version (Spent almost 2 days because of this error)
4. Use the existing project code and compile it with cmake


## libhttpserver
Step 1.
git clone https://git.gnunet.org/libmicrohttpd.git
goto libmicrohttpd directory
./bootstrap
mkdir build
cd build
../configure
make
sudo make install
Alternate option sudo apt-get install -y libmicrohttpd-dev
Step 2.
git clone https://github.com/etr/libhttpserver.git
cd libhttpserver
./bootstrap
mkdir build
cd build
../configure
make
sudo make install

## torch c++ https://pytorch.org/cppdocs/installing.html
goto anish directory
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.0.1%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-2.0.1+cpu.zip


# Commands
mkdir build
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build
make
./server

# go to another machine

1.  ab -n 1 -c 1 -e data_optimized_1000.csv -T "application/json" http://10.52.0.189:7000/hello

# Model C++ downlaoed from : github repo: https://github.com/EmreOzkose/pytorch_cpp/tree/main/b6
