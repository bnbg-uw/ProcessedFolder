SET vcpkg=C:\vcpkg
SET triplet=x64-windows
SETLOCAL

rmdir /S /Q build

pushd src\lapisgis
git pull
popd

mkdir build
pushd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%vcpkg%\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=%triplet%
popd