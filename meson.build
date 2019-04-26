project('judoassistant', 'cpp', default_options: ['cpp_std=c++17', 'default_library=static'], license: 'MIT')

# Core dependencies
boost_dep = dependency('boost', modules: ['system', 'filesystem', 'program_options']) # Used for ASIO and on the web server
lz4_dep = dependency('liblz4') # Compression library for networking and save-files
thread_dep = dependency('threads')

# UI dependencies
if get_option('ui')
    qt5 = import('qt5')
    qt5_dep = dependency('qt5', modules: ['Core', 'Widgets'])
else
    qt5 = disabler()
    qt5_dep = disabler()
endif

# Web server dependencies
if get_option('web')
    pqxx_dep = dependency('libpqxx') # PostgreSQL library
    botan_dep = dependency('botan-2') # Crypto library used for blowfish hashing
else
    pqxx_dep = disabler()
    botan_dep = disabler()
endif

# Setup configuration files
conf_data = configuration_data()
conf_data.set('VERSION_MAJOR', '0')
conf_data.set('VERSION_MINOR', '1')
conf_data.set('VERSION_PATCH', '0')
conf_data.set('DEBUG', get_option('buildtype') == 'debug')

# Copy license
configure_file(input : 'LICENSE', output: 'judoassistant-license.txt', copy: true)

# Find all sources and headers
include_dirs = include_directories(['.', 'src'])

core_sources = []
ui_sources = []
ui_moc_headers = []

score_sources = []
score_moc_headers = []

hub_sources = []
hub_moc_headers = []

web_sources = []

subdir('src')

# Compile static libraries
core_lib = library('core', core_sources, include_directories: include_dirs, dependencies: [lz4_dep])

# Qt5 compilations
if get_option('ui')
    ui_moc_files = qt5.preprocess(moc_headers: ui_moc_headers, include_directories: include_dirs)
    score_moc_files = qt5.preprocess(moc_headers: score_moc_headers, include_directories: include_dirs)
    hub_moc_files = qt5.preprocess(moc_headers: hub_moc_headers, include_directories: include_dirs)
else
    ui_moc_files = disabler()
    score_moc_files = disabler()
    hub_moc_files = disabler()
endif

# Compile static libraries
ui_lib = library('ui', ui_sources, ui_moc_files, include_directories: include_dirs, link_with: [core_lib], dependencies: [qt5_dep, thread_dep, boost_dep])

# Copy icons
if get_option('ui')
    configure_file(input : 'icons/LICENSE', output: 'icon-license.txt', copy: true)
endif

# Executables
hub_exe = executable('judoassistant', hub_sources, hub_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_dep, thread_dep, lz4_dep])
score_exe = executable('judoassistant-score', score_sources, score_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_dep, thread_dep])
web_exe = executable('judoassistant-web', web_sources, include_directories: include_dirs, link_with: [core_lib], dependencies: [boost_dep, thread_dep, pqxx_dep, botan_dep])

# Run targets
run_target('icons', command : 'scripts/compile_icons.sh')
run_target('flags', command : 'scripts/compile_flags.sh')

