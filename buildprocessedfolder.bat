SET vcpkg=C:\vcpkg
SET triplet=x64-windows
SETLOCAL

rmdir /S /Q build

IF EXIST src\lapisgis\ (
	pushd src\lapisgis\
	git pull
	popd
) ELSE (
	pushd src
	git clone --recurse-submodules https://github.com/jontkane/LapisGis
	popd
)

mkdir build
pushd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%vcpkg%\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=%triplet%
popd