core_sources += ['src/core/id.cpp']
core_sources += ['src/core/log.cpp']
core_sources += ['src/core/random.cpp']
core_sources += ['src/core/version.cpp']
core_sources += ['src/core/position_manager.cpp']

subdir('actions')
subdir('draw_systems')
subdir('network')
subdir('rulesets')
subdir('stores')

configure_file(input : 'version.hpp.in', output: 'version.hpp', configuration: conf_data)

