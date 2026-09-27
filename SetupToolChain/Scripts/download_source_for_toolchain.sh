source ./Scripts/env.sh

mkdir -p $TOP/src $TOP/build $PREFIX $SYSROOT
cd $TOP/src

# Stable versions
BINUTILS_VER=2.42
GCC_VER=13.3.0
GMP_VER=6.3.0
MPFR_VER=4.2.1
MPC_VER=1.3.1
MUSL_VER=1.2.6

echo "Downloading sources..."
wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.xz
wget https://ftp.gnu.org/gnu/gmp/gmp-$GMP_VER.tar.xz
wget https://ftp.gnu.org/gnu/mpfr/mpfr-$MPFR_VER.tar.xz
wget https://ftp.gnu.org/gnu/mpc/mpc-$MPC_VER.tar.gz
wget https://musl.libc.org/releases/musl-$MUSL_VER.tar.gz

# Extract
echo "Extracting sources Binutils..."
tar -xf binutils-$BINUTILS_VER.tar.xz
echo "Extracting sources GCC..."
tar -xf gcc-$GCC_VER.tar.xz
echo "Extracting sources GMP..."
tar -xf gmp-$GMP_VER.tar.xz
echo "Extracting sources MPFR..."
tar -xf mpfr-$MPFR_VER.tar.xz
echo "Extracting sources MPC..."
tar -xf mpc-$MPC_VER.tar.gz
echo "Extracting sources Musl..."
tar -xf musl-$MUSL_VER.tar.gz


# Prepare GCC prerequisites (link GMP/MPFR/MPC into GCC tree)
cd $TOP/src/gcc-$GCC_VER
# Either use contrib script or symlink sources into gcc tree
ln -s $TOP/src/gmp-$GMP_VER gmp
ln -s $TOP/src/mpfr-$MPFR_VER mpfr
ln -s $TOP/src/mpc-$MPC_VER mpc
# Optional: ./contrib/download_prerequisites
