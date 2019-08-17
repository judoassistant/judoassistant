project('judoassistant', 'cpp', default_options: ['cpp_std=c++17', 'default_library=static'], license: 'MIT')

# Setup configuration data
conf_data = configuration_data()
conf_data.set('VERSION_MAJOR', '0')
conf_data.set('VERSION_MINOR', '1')
conf_data.set('VERSION_PATCH', '0')
conf_data.set('DEBUG', get_option('buildtype') == 'debug')
conf_data.set_quoted('DATA_DIR', get_option('prefix') / get_option('datadir') / 'judoassistant')
conf_data.set_quoted('SYSCONF_DIR', get_option('prefix') / get_option('sysconfdir') / 'judoassistant')

# Core dependencies
boost_dep = dependency('boost', modules: ['system', 'filesystem', 'program_options']) # Used for ASIO and on the web server
thread_dep = dependency('threads')

if get_option('zstd_dir') == ''
    zstd_dep = dependency('libzstd') # Compression library for networking and save-files
else
    zstd_dep = cxx.find_library('libzstd', dirs: get_option('zstd_dir') + '/dll')
endif

# UI dependencies
if get_option('ui')
    qt5 = import('qt5')
    qt5_dep = dependency('qt5', modules: ['Core', 'Widgets', 'Gui'])

    if get_option('openssl_dir') == ''
        ssl_dep = dependency('libssl') # SSL library
        crypto_dep = dependency('libcrypto') # Crypto library for SSL
    else
        ssl_dep = cxx.find_library('libssl', dirs: get_option('openssl_dir') + '/lib')
        crypto_dep = cxx.find_library('libcrypto', dirs: get_option('openssl_dir') + '/lib')
    endif
else
    qt5 = disabler()
    qt5_dep = disabler()
    crypto_dep = disabler()
    ssl_dep = disabler()
endif

# Web server dependencies
if get_option('web')
    pqxx_dep = dependency('libpqxx') # PostgreSQL library
    botan_dep = dependency('botan-2') # Crypto library used for blowfish hashing
else
    pqxx_dep = disabler()
    botan_dep = disabler()
endif

# Set-up visual studio args
cxx = meson.get_compiler('cpp')
if cxx.get_id()=='msvc'
    add_project_arguments('/bigobj', language: 'cpp')
    add_project_arguments('-D_WIN32_WINNT=0x0601', language: 'cpp')
endif

# Find all sources and headers
include_array = ['.', 'src']

if get_option('zstd_dir') != ''
    include_array += [get_option('zstd_dir') + '\include']
endif

if get_option('openssl_dir') != ''
    include_array += [get_option('openssl_dir') + '\include']
endif

if get_option('cereal_dir') != ''
    include_array += [get_option('cereal_dir') + '\include']
endif

# Setting BOOST_INCLUDEDIR in the environment does not seem to work
if get_option('boost_dir') != ''
    include_array += [get_option('boost_dir')]
endif

include_dirs = include_directories(include_array)

core_sources = []
ui_sources = []
ui_moc_headers = []

score_sources = []
score_moc_headers = []

kiosk_sources = []
kiosk_moc_headers = []

hub_sources = []
hub_moc_headers = []

web_sources = []

subdir('src')

# Compile core library
core_lib = library('core', core_sources, include_directories: include_dirs, dependencies: [zstd_dep])

# Qt5 compilations
if get_option('ui')
    ui_moc_files = qt5.preprocess(moc_headers: ui_moc_headers, include_directories: include_dirs)
    score_moc_files = qt5.preprocess(moc_headers: score_moc_headers, include_directories: include_dirs)
    kiosk_moc_files = qt5.preprocess(moc_headers: kiosk_moc_headers, include_directories: include_dirs)
    hub_moc_files = qt5.preprocess(moc_headers: hub_moc_headers, include_directories: include_dirs)
else
    ui_moc_files = disabler()
    score_moc_files = disabler()
    kiosk_moc_files = disabler()
    hub_moc_files = disabler()
endif

# Compile ui library
ui_lib = library('ui', ui_sources, ui_moc_files, include_directories: include_dirs, link_with: [core_lib], dependencies: [qt5_dep, thread_dep, boost_dep])

# Copy icons
# if get_option('ui')
#     configure_file(input : 'icons/LICENSE', output: 'icon-license.txt', copy: true)
# endif

# Executables
hub_exe = executable('judoassistant', hub_sources, hub_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_dep, thread_dep, zstd_dep, ssl_dep, crypto_dep], install: true)
score_exe = executable('judoassistant-score', score_sources, score_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_dep, thread_dep], install: true)
kiosk_exe = executable('judoassistant-kiosk', kiosk_sources, kiosk_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_dep, thread_dep], install: true)
web_exe = executable('judoassistant-web', web_sources, include_directories: include_dirs, link_with: [core_lib], dependencies: [boost_dep, thread_dep, pqxx_dep, botan_dep], install: true)

# Install data
if get_option('ui')
    # Icons
    install_data('icons/icons/svg/account-plus.svg', rename: 'player-add.svg', install_dir: get_option('datadir') / 'judoassistant/icons')
    install_data('icons/icons/svg/account-multiple-plus.svg', rename: 'category-add.svg', install_dir: get_option('datadir') / 'judoassistant/icons')
    install_data('icons/icons/svg/filter.svg', rename: 'filter.svg', install_dir: get_option('datadir') / 'judoassistant/icons')
    install_data('icons/icons/svg/eye-off.svg', rename: 'hide.svg', install_dir: get_option('datadir') / 'judoassistant/icons')

    # Flags
    install_data('flags/flags/4x3/dk.svg', rename: 'den.svg', install_dir: get_option('datadir') / 'judoassistant/flags')
    install_data('flags/flags/4x3/gb.svg', rename: 'gbr.svg', install_dir: get_option('datadir') / 'judoassistant/flags')
    install_data('flags/flags/4x3/fr.svg', rename: 'fra.svg', install_dir: get_option('datadir') / 'judoassistant/flags')

    # Desktop Entries
    install_data('data/judoassistant.desktop', install_dir: get_option('datadir') / 'applications')
    install_data('data/judoassistant-score.desktop', install_dir: get_option('datadir') / 'applications')
    install_data('data/judoassistant-kiosk.desktop', install_dir: get_option('datadir') / 'applications')
endif

if get_option('web')
    install_data('data/example-server-config.cfg', rename: 'config.cfg', install_dir: get_option('sysconfdir') / 'judoassistant')
endif

