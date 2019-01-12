project('judoassistant', 'cpp', default_options: ['cpp_std=c++17', 'default_library=static'], license: 'MIT')

# Qt5 dependency
qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Widgets'])

# Boost dependency
boost_dep = dependency('boost', modules: ['system'])

# LZ4 dependency
lz4_dep = dependency('liblz4')

# Threads dependency
thread_dep = dependency('threads')

# Setup configuration files
conf_data = configuration_data()
conf_data.set('version_major', '0')
conf_data.set('version_minor', '1')
conf_data.set('version_patch', '0')

configure_file(input : 'src/version.hpp.in', output: 'version.hpp', configuration: conf_data)

# Copy license
configure_file(input : 'LICENSE', output: 'judoassistant-license.txt', copy: true)

# Copy icons
configure_file(input : 'icomoon/License.txt', output: 'icomoon-license.txt', copy: true)
configure_file(input : 'icomoon/SVG/116-user-plus.svg', output: 'player-add.svg', copy: true)
configure_file(input : 'icomoon/SVG/117-user-minus.svg', output: 'player-erase.svg', copy: true)

# Find all sources and headers
include_dirs = include_directories(['.', 'src', 'cereal/include'])

core_sources = []

ui_sources = []
ui_moc_headers = []

score_sources = []
score_moc_headers = []

hub_sources = []
hub_moc_headers = []

subdir('src')

# Qt5 compilations
ui_moc_files = qt5.preprocess(moc_headers: ui_moc_headers, include_directories: include_dirs)
score_moc_files = qt5.preprocess(moc_headers: score_moc_headers, include_directories: include_dirs)
hub_moc_files = qt5.preprocess(moc_headers: hub_moc_headers, include_directories: include_dirs)

# Compile static libraries
core_lib = library('core', core_sources, include_directories: include_dirs, dependencies: [lz4_dep])
ui_lib = library('ui', ui_sources, ui_moc_files, include_directories: include_dirs, link_with: [core_lib], dependencies: [qt5_dep, thread_dep, boost_dep])

# Executables
hub_exe = executable('judoassistant', hub_sources, hub_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_dep, thread_dep, lz4_dep])
score_exe = executable('judoassistant-score', score_sources, score_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_dep, thread_dep])

