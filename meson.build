project('qutejudo', 'cpp', default_options: ['cpp_std=c++17'])

# Qt5 dependency
qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Widgets', 'Gui'])

# Boost dependency
boost_dep = dependency('boost', modules: ['system'])

# Setup configuration files
conf_data = configuration_data()
conf_data.set('version_major', '0')
conf_data.set('version_minor', '1')
conf_data.set('version_patch', '0')

configure_file(input : 'src/version.hpp.in', output: 'version.hpp', configuration: conf_data)

# Copy icons
configure_file(input : 'icomoon/SVG/116-user-plus.svg', output: 'player-add.svg', copy: true)
configure_file(input : 'icomoon/SVG/117-user-minus.svg', output: 'player-delete.svg', copy: true)

# Find all sources and headers
include_dirs = include_directories(['.', 'src', 'cereal/include'])

moc_headers = []
sources = []

subdir('src')

# Qt5 compilations
moc_files = qt5.preprocess(moc_headers: moc_headers, include_directories: include_dirs)

# Executables
qutejudo_exe = executable('qutejudo', 'src/qutejudo.cpp', sources, moc_files, include_directories: include_dirs, dependencies: [qt5_dep, boost_dep])
