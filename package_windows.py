pkgdir=r'C:\Users\Svend\Downloads\judoassistant'
builddir='build-release'

# Configure
subprocess.check_call(['meson', 'build-release', '--buildtype=release', '-Dbindir=""', '-Dprefix='+pkgdir, '-Dlookupmode=relative', '-Dweb=false'])

# Build
subprocess.check_call(['ninja'], cwd=builddir)

# Install
subprocess.check_call(['ninja install'], cwd=builddir)

# Copy extra files
shutil.copy('judoassistant.iss', pkgdir, cwd=builddir)

# Copy DLLs
shutil.copy(r'C:\Libraries\boost_1_70_0\lib64-msvc-14.2\boost_filesystem-vc142-mt-x64-1_70.dll', pkgdir)
shutil.copy(r'C:\Libraries\OpenSSL-Win64\bin\libcrypto-1_1-x64.dll', pkgdir)
shutil.copy(r'C:\Libraries\OpenSSL-Win64\bin\libssl-1_1-x64.dll', pkgdir)
shutil.copy(r'C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Core.dll', pkgdir)
shutil.copy(r'C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Gui.dll', pkgdir)
shutil.copy(r'C:\Libraries\Qt\5.13.0\msvc2017_64\bin\Qt5Widgets.dll', pkgdir)
shutil.copy(r'C:\Libraries\zstd-v1.4.2-win64\dll\libzstd.dll', pkgdir)
shutil.copy(r'C:\Windows\Fonts\NotoSans-Bold.ttf', pkgdir)
shutil.copy(r'C:\Windows\Fonts\NotoSans-Regular.ttf', pkgdir)
shutil.copy(r'C:\Windows\Fonts\NotoSansMono-Bold.ttf', pkgdir)

# Run Inno Setup 6
subprocess.check_call([r'C:\Program Files (x86)\Inno Setup 6\ISCC.exe', 'judoassistant.iss'], cwd=pkgdir)
