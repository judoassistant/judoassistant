REM Setup environment
set PKG_CONFIG_PATH=C:\Libraries\pkgconfig
set BOOST_ROOT=C:\Libraries\boost_1_70_0
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Configure and build
meson --buildtype=release build-release -Dprefix="%cd%\build-release\pkg" -Dbindir="" || Exit 1
ninja -j2 -C build-release || Exit 1

