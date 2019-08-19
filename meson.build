project('judoassistant', 'cpp', default_options: ['cpp_std=c++17', 'default_library=static'], version: '0.1.0', license: 'MIT')

# JudoAssistant Version
judoassistant_version = meson.project_version()
version_array = judoassistant_version.split('.')
version_major = version_array[0].to_int()
version_minor = version_array[1].to_int()
version_patch = version_array[2].to_int()

# Setup configuration data
conf_data = configuration_data()
conf_data.set('VERSION', judoassistant_version)
conf_data.set('VERSION_MAJOR', version_major)
conf_data.set('VERSION_MINOR', version_minor)
conf_data.set('VERSION_PATCH', version_patch)
conf_data.set('DEBUG', get_option('buildtype') == 'debug')

if get_option('lookupmode') == 'absolute'
    conf_data.set_quoted('DATA_DIR', join_paths(get_option('prefix'), get_option('datadir'), meson.project_name()))
    conf_data.set_quoted('SYSCONF_DIR', join_paths(get_option('prefix'), get_option('sysconfdir'), meson.project_name()))
else
    conf_data.set_quoted('DATA_DIR', join_paths(get_option('datadir'), meson.project_name()))
    conf_data.set_quoted('SYSCONF_DIR', join_paths(get_option('sysconfdir'), meson.project_name()))
endif

# Core dependencies
thread_dep = dependency('threads')
cereal_dep = dependency('cereal', fallback: ['cereal', 'cereal_dep'])
zstd_dep = dependency('libzstd') # Compression library for networking and save-files
boost_core_dep = dependency('boost') # Used for ASIO

# UI dependencies
if get_option('ui')
    boost_ui_dep = dependency('boost', modules: ['filesystem']) # Used for handling file paths

    qt5 = import('qt5')
    qt5_dep = dependency('qt5', modules: ['Core', 'Widgets', 'Gui'])

    ssl_dep = dependency('libssl') # SSL library
    crypto_dep = dependency('libcrypto') # Crypto library for SSL
else
    boost_ui_dep = disabler()
    qt5 = disabler()
    qt5_dep = disabler()
    crypto_dep = disabler()
    ssl_dep = disabler()
endif

# Web server dependencies
if get_option('web')
    pqxx_dep = dependency('libpqxx') # PostgreSQL library
    botan_dep = dependency('botan-2') # Crypto library used for blowfish hashing
    boost_web_dep = dependency('boost', modules: ['filesystem', 'program_options']) # Used for ASIO, file paths and program options
else
    pqxx_dep = disabler()
    botan_dep = disabler()
    boost_web_dep = disabler()
endif

# Set-up visual studio args
cxx = meson.get_compiler('cpp')
if cxx.get_id()=='msvc'
    add_project_arguments('/bigobj', language: 'cpp')
    add_project_arguments('-D_WIN32_WINNT=0x0601', language: 'cpp')
    add_project_link_arguments('/entry:mainCRTStartup', language: 'cpp')
endif

include_dirs = include_directories(['.', 'src'])

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
core_lib = library('core', core_sources, include_directories: include_dirs, dependencies: [zstd_dep, cereal_dep, boost_core_dep, crypto_dep, ssl_dep])

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
ui_lib = library('ui', ui_sources, ui_moc_files, include_directories: include_dirs, link_with: [core_lib], dependencies: [boost_ui_dep, qt5_dep, thread_dep, cereal_dep])

# Copy icons
# if get_option('ui')
#     configure_file(input : 'icons/LICENSE', output: 'icon-license.txt', copy: true)
# endif

# Executables
hub_exe = executable('judoassistant', hub_sources, hub_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_ui_dep, thread_dep, zstd_dep, ssl_dep, crypto_dep, cereal_dep], gui_app: true, install: true)
score_exe = executable('judoassistant-score', score_sources, score_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_ui_dep, thread_dep, cereal_dep], gui_app: true, install: true)
kiosk_exe = executable('judoassistant-kiosk', kiosk_sources, kiosk_moc_files, include_directories: include_dirs, link_with: [core_lib, ui_lib], dependencies: [qt5_dep, boost_ui_dep, thread_dep, cereal_dep], gui_app: true, install: true)
web_exe = executable('judoassistant-web', web_sources, include_directories: include_dirs, link_with: [core_lib], dependencies: [thread_dep, pqxx_dep, botan_dep, cereal_dep, boost_web_dep], install: true)

# Install data
if get_option('ui')
    # Icons
    install_data('icons/icons/svg/account-plus.svg', rename: 'player-add.svg', install_dir: join_paths(get_option('datadir'), meson.project_name(), 'icons'))
    install_data('icons/icons/svg/account-multiple-plus.svg', rename: 'category-add.svg', install_dir: join_paths(get_option('datadir'), meson.project_name(), 'icons'))
    install_data('icons/icons/svg/filter.svg', rename: 'filter.svg', install_dir: join_paths(get_option('datadir'), meson.project_name(), 'icons'))
    install_data('icons/icons/svg/eye-off.svg', rename: 'hide.svg', install_dir: join_paths(get_option('datadir'), meson.project_name(), 'icons'))

    # Flags
    install_data('flags/flags/4x3/dk.svg', rename: 'den.svg', install_dir: join_paths(get_option('datadir'), meson.project_name(), 'flags'))
    install_data('flags/flags/4x3/gb.svg', rename: 'gbr.svg', install_dir: join_paths(get_option('datadir'), meson.project_name(), 'flags'))
    install_data('flags/flags/4x3/fr.svg', rename: 'fra.svg', install_dir: join_paths(get_option('datadir'), meson.project_name(), 'flags'))

    # Desktop Entries
    if host_machine.system() == 'linux'
        install_data('data/judoassistant.desktop', install_dir: join_paths(get_option('datadir'), 'applications'))
        install_data('data/judoassistant-score.desktop', install_dir: join_paths(get_option('datadir'), 'applications'))
        install_data('data/judoassistant-kiosk.desktop', install_dir: join_paths(get_option('datadir'), 'applications'))
    endif

    if host_machine.system() == 'windows'
        configure_file(input : 'data/judoassistant.iss.in', output: 'judoassistant.iss', configuration: conf_data, install: true, install_dir: '.')
    endif
endif

if get_option('web')
    install_data('data/example-server-config.cfg', rename: 'config.cfg', install_dir: join_paths(get_option('sysconfdir'), 'judoassistant'))
endif

