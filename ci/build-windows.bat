REM Setup environment
set PKG_CONFIG_PATH=C:\Libraries\pkgconfig
set BOOST_ROOT=C:\Libraries\boost_1_70_0
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Configure and build
meson --buildtype=release build-release -Dprefix="%cd%\build-release\pkg" -Dbindir="" || Exit 1
ninja -j2 -C build-release || Exit 1

REM Prepare package directory
cd build-release
ninja install

copy C:\Libraries\boost_1_70_0\lib64-msvc-14.2\boost_filesystem-vc142-mt-x64-1_70.dll pkg
copy C:\Libraries\OpenSSL-Win64\bin\libcrypto-1_1-x64.dll pkg
copy C:\Libraries\OpenSSL-Win64\bin\libssl-1_1-x64.dll pkg
copy C:\Libraries\zstd-v1.4.2-win64\dll\libzstd.dll pkg
copy C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Core.dll pkg
copy C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Gui.dll pkg
copy C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Widgets.dll pkg
copy C:\Windows\Fonts\NotoSans-Bold.ttf pkg
copy C:\Windows\Fonts\NotoSans-Regular.ttf pkg
copy C:\Windows\Fonts\NotoSansMono-Bold.ttf pkg

REM Create installer
cd pkg
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" judoassistant.iss

