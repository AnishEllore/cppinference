# Get the base Ubuntu image from Docker Hub
FROM ubuntu:latest
# Update required apps on the base image
RUN apt-get -y update && apt-get install -y
RUN apt-get -y install g++ cmake wget unzip automake autoconf libtool autotools-dev

# Specify the working directory
WORKDIR /src
# Copy the current folder which contains C++ source code to the Docker image
COPY . .

# installing httplib
RUN apt-get install -y libmicrohttpd-dev
RUN wget https://github.com/etr/libhttpserver/archive/refs/heads/master.zip
RUN unzip master.zip
WORKDIR /src/libhttpserver-master

RUN ./bootstrap
RUN mkdir build
WORKDIR /src/libhttpserver-master/build
RUN ../configure
RUN make
RUN make install

# installing torch
WORKDIR /src
RUN wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.0.1%2Bcpu.zip
RUN unzip libtorch-cxx11-abi-shared-with*
RUN rm *.zip

WORKDIR /src/projectX

# Create build dir, configure CMake and run build
RUN mkdir build
RUN cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
RUN cd build && make
# Run the built application
EXPOSE 7000
CMD ./build/server

# https://awstip.com/a-modern-c-http-server-with-pistache-and-docker-fde07110f2f
# sudo docker build -t cppserver .
# docker run -p 7000:7000 -t cppserver