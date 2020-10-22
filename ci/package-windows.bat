REM Install into pkg directory
cd build-release
ninja install || Exit 1

REM Copy libraries
mkdir pkg\platforms
copy C:\Libraries\boost_1_70_0\lib64-msvc-14.2\boost_filesystem-vc142-mt-x64-1_70.dll pkg
copy C:\Libraries\OpenSSL-Win64\bin\libcrypto-1_1-x64.dll pkg
copy C:\Libraries\OpenSSL-Win64\bin\libssl-1_1-x64.dll pkg
copy C:\Libraries\zstd-v1.4.2-win64\dll\libzstd.dll pkg
copy C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Core.dll pkg
copy C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Gui.dll pkg
copy C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Widgets.dll pkg
copy C:\Libraries\Qt\5.13.0\msvc2017_64\plugins\platforms\qwindows.dll pkg\platforms
copy C:\Windows\Fonts\NotoSans-Bold.ttf pkg
copy C:\Windows\Fonts\NotoSans-Regular.ttf pkg
copy C:\Windows\Fonts\NotoSansMono-Bold.ttf pkg

REM Create installer
cd pkg
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" judoassistant.iss

