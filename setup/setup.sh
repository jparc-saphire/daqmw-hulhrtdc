#!/bin/bash
# setup.sh - AlmaLinux 9/10 向け DAQ-MW 環境セットアップスクリプト
#
# 使い方:
#   cd /path/to/daq-hulhrtdc
#   bash setup/setup.sh
#
# 動作確認済み: AlmaLinux 10.0 (Purple Lion) x86_64
# 所要時間: 約30〜60分（ネット速度による）

set -e

REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR=/tmp/daq-setup-build

echo "=== DAQ-MW Environment Setup ==="
echo "REPO_DIR: $REPO_DIR"
echo "BUILD_DIR: $BUILD_DIR"
echo ""

mkdir -p "$BUILD_DIR"

# ============================================================
# 1. DNF パッケージのインストール
# ============================================================
echo "=== [1/9] Installing DNF packages ==="
sudo dnf install -y epel-release
sudo dnf config-manager --set-enabled crb
sudo dnf install -y \
    boost-devel \
    python3-devel \
    libuuid-devel \
    ncurses-devel \
    xerces-c-devel \
    tbb-devel \
    cmake \
    gcc-c++ \
    make \
    perl \
    wget \
    autoconf \
    automake \
    libtool \
    openssl-devel \
    root \
    root-net-http

# ============================================================
# 2. omniORB 4.2.5 のビルド・インストール
#    - 4.1.7: GCC 14 でビルド不可 (K&R スタイル C コード)
#    - 4.3.0: OpenRTM-aist 1.2.2 と非互換
# ============================================================
echo ""
echo "=== [2/9] Building omniORB 4.2.5 ==="
cd "$BUILD_DIR"
if [ ! -f omniORB-4.2.5.tar.bz2 ]; then
    wget https://sourceforge.net/projects/omniorb/files/omniORB/omniORB-4.2.5/omniORB-4.2.5.tar.bz2
fi
tar xjf omniORB-4.2.5.tar.bz2
cd omniORB-4.2.5
./configure --prefix=/usr
make -j$(nproc)
sudo make install
echo "omniORB 4.2.5 installed."

# ============================================================
# 3. OpenRTM-aist 1.2.2 のビルド・インストール
# ============================================================
echo ""
echo "=== [3/9] Building OpenRTM-aist 1.2.2 ==="
cd "$BUILD_DIR"
if [ ! -f OpenRTM-aist-1.2.2.tar.gz ]; then
    wget https://github.com/OpenRTM/OpenRTM-aist/archive/refs/tags/v1.2.2.tar.gz \
        -O OpenRTM-aist-1.2.2.tar.gz
fi
rm -rf OpenRTM-aist-1.2.2
tar xzf OpenRTM-aist-1.2.2.tar.gz
cd OpenRTM-aist-1.2.2

# --- パッチ (a): omniORB 4.2.x では RefCountServantBase が ServantBase を
#     継承しなくなったため、CorbaPort, Manager, RTObject の型を変更 ---
echo "  Patching CorbaPort.h ..."
sed -i \
    's/PortableServer::RefCountServantBase\b\*/PortableServer::ServantBase*/g' \
    src/lib/rtm/CorbaPort.h
sed -i \
    's/PortableServer::RefCountServantBase\b&/PortableServer::ServantBase\&/g' \
    src/lib/rtm/CorbaPort.h

echo "  Patching CorbaPort.cpp ..."
sed -i \
    's/PortableServer::RefCountServantBase\b&/PortableServer::ServantBase\&/g' \
    src/lib/rtm/CorbaPort.cpp

echo "  Patching Manager.cpp ..."
sed -i \
    's/PortableServer::RefCountServantBase\b\*/PortableServer::ServantBase*/g' \
    src/lib/rtm/Manager.cpp

echo "  Patching RTObject.cpp ..."
sed -i \
    's/PortableServer::RefCountServantBase\b\*/PortableServer::ServantBase*/g' \
    src/lib/rtm/RTObject.cpp

# --- パッチ (b): バンドルされた SK.cc が omniORB 4.3.0 で生成されていたため
#     omniORB 4.2.5 の omniidl で再生成 ---
echo "  Regenerating IDL SK files ..."
RTM_IDL_SRC="$BUILD_DIR/OpenRTM-aist-1.2.2/src/lib/rtm/idl"
cd "$RTM_IDL_SRC"
for idl in *.idl; do
    omniidl -bcxx -Wba "$idl"
done
OBSERVER_IDL=$(find "$BUILD_DIR/OpenRTM-aist-1.2.2" -name "ComponentObserver.idl" 2>/dev/null | head -1)
if [ -n "$OBSERVER_IDL" ]; then
    cd "$(dirname "$OBSERVER_IDL")"
    omniidl -bcxx -Wba -I"$RTM_IDL_SRC" ComponentObserver.idl
fi
cd "$BUILD_DIR/OpenRTM-aist-1.2.2"

./autogen.sh
./configure --prefix=/usr --without-doxygen --without-ssl --disable-fluentd \
    CXXFLAGS="-Wall -fPIC -O2 -std=c++14"

# --- パッチ (c): configure が Makefile を生成した後に -lomnisslTP4 を除去
#     (autogen/configure より前に除去しても再生成で上書きされるため) ---
echo "  Removing -lomnisslTP4 from generated Makefiles ..."
find . -name "Makefile" | xargs -r sed -i 's/ -lomnisslTP4//g'
find . -name "*.la"     | xargs -r sed -i 's/ -lomnisslTP4//g'

make -j$(nproc)
sudo make install

# rtm-config からも -lomnisslTP4 を除去
sudo sed -i 's/ -lomnisslTP4//' /usr/bin/rtm-config

# --- パッチ (d): C++17 で動的例外指定 throw() が禁止されているため除去
#     (Monitor が ROOT/C++17 を要求するため必要) ---
echo "  Removing dynamic exception specifications from OpenRTM headers ..."
sudo perl -i -0777 -pe 's/\bthrow\s*\([^)]*\)//g' /usr/include/openrtm-1.2/rtm/*.h

echo "OpenRTM-aist 1.2.2 installed."

# ============================================================
# 4. daqmw ヘッダのインストール
# ============================================================
echo ""
echo "=== [4/9] Installing daqmw headers ==="
sudo mkdir -p /usr/include/daqmw/idl
sudo cp -r "$REPO_DIR/setup/daqmw_include/." /usr/include/daqmw/
# idl/ サブディレクトリも含む
echo "daqmw headers installed to /usr/include/daqmw/"

# ============================================================
# 5. DAQ-Middleware 1.4.4 ライブラリのビルド・インストール
#    (libSock, libSitcpRbcp, libsitcpbcp, libJsonSpirit)
# ============================================================
echo ""
echo "=== [5/9] Building DAQ-Middleware 1.4.4 libraries ==="
cd "$BUILD_DIR"
if [ ! -f DAQ-Middleware-1.4.4.tar.gz ]; then
    wget http://daqmw.kek.jp/src/DAQ-Middleware-1.4.4.tar.gz
fi
rm -rf DAQ-Middleware-1.4.4
tar xzf DAQ-Middleware-1.4.4.tar.gz
cd DAQ-Middleware-1.4.4/src/lib
make
sudo make install prefix=/usr
echo "/usr/lib64/daqmw" | sudo tee /etc/ld.so.conf.d/daqmw.conf
sudo ldconfig
echo "daqmw libraries installed to /usr/lib64/daqmw/"

# ============================================================
# 6. TBB 2020.3 のビルド・インストール
#    ROOT 6.x が libtbb.so.2 (旧 API) を必要とするが
#    AlmaLinux 9/10 には libtbb.so.12 (TBB 2021) しかない
# ============================================================
echo ""
echo "=== [6/9] Building TBB 2020.3 (for ROOT compatibility) ==="
cd "$BUILD_DIR"
if [ ! -f tbb-2020.3.tar.gz ]; then
    wget https://github.com/oneapi-src/oneTBB/archive/refs/tags/v2020.3.tar.gz \
        -O tbb-2020.3.tar.gz
fi
rm -rf oneTBB-2020.3
tar xzf tbb-2020.3.tar.gz
cd oneTBB-2020.3
# GCC 14 対応: -fpermissive -w が必要
make -j$(nproc) CXXFLAGS="-fpermissive -w"
sudo cp build/linux_intel64_gcc_*/libtbb.so.2 /usr/lib64/
sudo ln -sf libtbb.so.2 /usr/lib64/libtbb.so
sudo ldconfig
echo "TBB 2020.3 (libtbb.so.2) installed."

# ============================================================
# 7. DaqOperatorComp のビルド・インストール
# ============================================================
echo ""
echo "=== [7/9] Building DaqOperatorComp ==="
sudo mkdir -p /usr/share/daqmw
sudo cp -r "$BUILD_DIR/DAQ-Middleware-1.4.4/src/DaqOperator" /usr/share/daqmw/
sudo cp -r "$BUILD_DIR/DAQ-Middleware-1.4.4/src/mk" /usr/share/daqmw/
sudo sed -i 's/export DAQMWSRCROOT=/#export DAQMWSRCROOT=/' /usr/share/daqmw/DaqOperator/Makefile
sudo sed -i 's|../lib/SiTCP/CPP/Sock|/usr/lib64/daqmw|g' /usr/share/daqmw/DaqOperator/Makefile
cd /usr/share/daqmw/DaqOperator
sudo make clean
sudo make
sudo mkdir -p /usr/libexec/daqmw
sudo cp DaqOperatorComp /usr/libexec/daqmw/
echo "DaqOperatorComp installed to /usr/libexec/daqmw/"

# ============================================================
# 8. autogen 再生成 + DAQ コンポーネントのビルド
# ============================================================
echo ""
echo "=== [8/9] Regenerating autogen and building DAQ components ==="
RTM_IDL=/usr/include/openrtm-1.2/rtm/idl

for comp in DAQLogger Monitor BestEffortDispatcher Merger MznHRTdcReader1; do
    dir="$REPO_DIR/component/$comp/autogen"
    echo "  Regenerating $comp/autogen ..."
    rm -fr "$dir"
    mkdir -p "$dir"
    cd "$dir"
    ln -s /usr/include/daqmw/idl/DAQService.idl
    omniidl -bcxx -Wba -I"$RTM_IDL" DAQService.idl
    rtm-skelwrapper --include-dir="" --skel-suffix=Skel --stub-suffix=Stub \
        --idl-file=DAQService.idl
    cd "$REPO_DIR"
done

echo "  Building daq-mw-app/util (libSiTcpRbcp, libgetaddr, libHexDump) ..."
cd "$REPO_DIR/daq-mw-app/util"
make

echo "  Building standalone libraries for all components ..."
for sa_dir in $(find "$REPO_DIR" -type d -name standalone -path "*/component/*/standalone"); do
    echo "    Building $sa_dir ..."
    mkdir -p "$sa_dir/lib"
    make -C "$sa_dir" clean
    make -C "$sa_dir"
done

echo "  Building all components ..."
cd "$REPO_DIR/component"
make clean
make
echo "DAQ components built."

# ============================================================
# 9. 修正済み run.py のインストール
# ============================================================
echo ""
echo "=== [9/9] Installing patched run.py ==="
sudo cp "$REPO_DIR/setup/run.py" /usr/bin/run.py
sudo chmod +x /usr/bin/run.py
echo "run.py installed."

# ============================================================
# 完了
# ============================================================
echo ""
echo "============================================"
echo "  Setup complete!"
echo "============================================"
echo ""
echo "To start DAQ:"
echo "  cd $REPO_DIR"
echo "  run.py -l daq_work/config_all.xml"
echo ""
echo "Note: Edit daq_work/config_all.xml to set"
echo "  - srcAddr (HRTDC IP address)"
echo "  - dirName (output data directory)"
