#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status.

# Set the installation directory
INSTALL_DIR=$PWD/_dependency

# Create the installation directory
mkdir -p $INSTALL_DIR


# Define common CMake arguments
extra_link_libs="-static-libstdc++ -static-libgcc -pthread -ldl"
if [[ $(getconf GNU_LIBC_VERSION | cut -d' ' -f2) < 2.17 ]]; then
    extra_link_libs="${extra_link_libs} -lrt"
fi

common_cmake_args=(
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
    "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fPIC ${extra_cpp_flags}"
    "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fPIC"
    -DCMAKE_EXE_LINKER_FLAGS="${extra_link_libs}"
    -DCMAKE_SHARED_LINKER_FLAGS="${extra_link_libs}"
    -DCMAKE_INCLUDE_PATH=${INSTALL_DIR}/include
    "-DCMAKE_LIBRARY_PATH=${INSTALL_DIR}/lib"
    -DBUILD_SHARED_LIBS=OFF
)

# Define linker flags
ld_flags=(
    "-L${INSTALL_DIR}/lib"
    "-L${INSTALL_DIR}/lib64"
)

# Define common configure environment variables
common_configure_envs=(
    "CFLAGS=${CMAKE_C_FLAGS} -fcommon -fno-omit-frame-pointer -fPIC -O2 -D_DEFAULT_SOURCE -D_GNU_SOURCE ${extra_cpp_flags}"
    "CXXFLAGS=${CMAKE_CXX_FLAGS} -fcommon -fno-omit-frame-pointer -fPIC -O2 -D_DEFAULT_SOURCE -D_GNU_SOURCE ${extra_cpp_flags}"
    "CPPFLAGS=-isystem ${INSTALL_DIR}/include ${extra_cpp_flags}"
    "LDFLAGS=-Wl,-rpath=\\\\$\\$ORIGIN ${ld_flags[*]}"
    "LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:${INSTALL_DIR}/lib:${INSTALL_DIR}/lib64"
    "PATH=${PATH}:${INSTALL_DIR}/bin"
)

# Apply common configure environment variables
apply_configure_envs() {
    for env_var in "${common_configure_envs[@]}"; do
        export "$env_var"
    done
}


nproc=8

# Build autoconf
build_autoconf() {
    echo "/*****************************************/"
    echo "/*** Downloading and building autoconf ***/"
    echo "/*****************************************/"
    VERSION="2.71"
    URL="https://ftp.gnu.org/gnu/autoconf/autoconf-${VERSION}.tar.gz"
    cd external
    curl -sLO ${URL}
    tar -xzf autoconf-${VERSION}.tar.gz
    cd autoconf-${VERSION}
    apply_configure_envs
    ./configure --prefix=${INSTALL_DIR}
    make -s -j$(nproc)
    make -s install
    cd ../../
}

# Build autoconf-archive
build_autoconf_archive() {
    echo "/*************************************************/"
    echo "/*** Downloading and building autoconf-archive ***/"
    echo "/*************************************************/"
    VERSION="2021.02.19"
    URL="https://ftp.gnu.org/gnu/autoconf-archive/autoconf-archive-${VERSION}.tar.xz"
    cd external
    curl -sLO ${URL}
    tar -xf autoconf-archive-${VERSION}.tar.xz
    cd autoconf-archive-${VERSION}
    apply_configure_envs
    ./configure --prefix=${INSTALL_DIR}
    make -s -j$(nproc)
    make -s install
    cd ../../
}

# Build automake
build_automake() {
    echo "/*****************************************/"
    echo "/*** Downloading and building automake ***/"
    echo "/*****************************************/"
    VERSION="1.16.5"
    URL="https://ftp.gnu.org/gnu/automake/automake-${VERSION}.tar.xz"
    cd external
    curl -sLO ${URL}
    tar -xf automake-${VERSION}.tar.xz
    cd automake-${VERSION}
    apply_configure_envs
    ./configure --prefix=${INSTALL_DIR}
    make -s -j$(nproc)
    make -s install
    cd ../../
}

# Build libtool
build_libtool() {
    echo "/****************************************/"
    echo "/*** Downloading and building libtool ***/"
    echo "/****************************************/"

    VERSION="2.4.6"
    URL="https://ftp.gnu.org/gnu/libtool/libtool-${VERSION}.tar.xz"
    cd external
    curl -sLO ${URL}
    tar -xf libtool-${VERSION}.tar.xz
    cd libtool-${VERSION}
    apply_configure_envs
    ./configure --prefix=${INSTALL_DIR} --enable-ltdl-install --enable-shared=no --enable-static=yes
    make -s -j$(nproc)
    make -s install
    cd ../../
}

# Build libunwind
build_libunwind() {
    echo "/******************************************/"
    echo "/*** Downloading and building libunwind ***/"
    echo "/******************************************/"

    VERSION="1.5.0"
    URL="http://download.savannah.nongnu.org/releases/libunwind/libunwind-${VERSION}.tar.gz"
    cd external
    curl -sLO ${URL}
    tar -xzf libunwind-${VERSION}.tar.gz
    cd libunwind-${VERSION}
    apply_configure_envs
    ./configure --prefix=${INSTALL_DIR} --disable-minidebuginfo --disable-shared --enable-static
    make -s -j$(nproc)
    make -s install
    cd ../../
}

# Build fmt
build_fmt() {
    echo "/************************/"
    echo "/*** Building fmt ***/"
    echo "/************************/"
    cd external/fmt
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release "${common_cmake_args[@]}" -DBUILD_TESTS=OFF -DFMT_TEST=OFF -DFMT_FUZZ=OFF -DFMT_INSTALL=ON ..
    make -s -j$(nproc)
    make -s install
    cd ../../..
}

# Build gflags
build_gflags() {
    echo "/************************/"
    echo "/*** Building gflags ***/"
    echo "/************************/"
    cd external/gflags
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release "${common_cmake_args[@]}" -DBUILD_SHARED_LIBS=OFF ..
    make -s -j$(nproc)
    make -s install
    cd ../../..
}


# Build glog
build_glog() {
    echo "/**********************/"
    echo "/*** Building glog ***/"
    echo "/*********************/"
    cd external/glog
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release "${common_cmake_args[@]}" -DBUILD_SHARED_LIBS=OFF ..
    make -s -j$(nproc)
    make -s install
    cd ../../..
}

# Build gtest
build_gtest() {
    echo "/**********************/"
    echo "/*** Building gtest ***/"
    echo "/*********************/"
    cd external/gtest
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release "${common_cmake_args[@]}" -DBUILD_SHARED_LIBS=OFF ..
    make -s -j$(nproc)
    make -s install
    cd ../../..
}

# Parse command line arguments
build_all=false
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --autoconf) build_autoconf ;;
        --autoconf-archive) build_autoconf_archive ;;
        --automake) build_automake ;;
        --libtool) build_libtool ;;
        --libunwind) build_libunwind ;;
        --fmt) build_fmt ;;
        --gflags) build_gflags ;;
        --glog) build_glog ;;
        --gtest) build_gtest ;;
        --all) build_all=true ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ "$build_all" = true ] ; then
    build_autoconf
    build_autoconf_archive
    build_automake
    build_libtool
    build_libunwind
    build_fmt
    build_gflags
    build_glog
fi



echo "Build complete."
