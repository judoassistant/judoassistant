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

# Run Inno Setup 6
subprocess.check_call([r'C:\Program Files (x86)\Inno Setup 6\ISCC.exe', 'judoassistant.iss'], cwd=pkgdir)
