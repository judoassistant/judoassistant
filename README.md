JudoAssistant
=============
JudoAssistant is free and open source software for running judo tournaments.
The software is currently in the pre-alpha stage and no official releases are
available.

Getting Started
---------------
Currently the only way to get started is to figure things out yourself. The
latest alpha release can be downloaded on the
[releases page](https://github.com/judoassistant/judoassistant/releases)
or you can compile the software yourself. At a later point in time a manual and
installation instructions will be available at
[judoassistant.com](https://judoassistant.com/).

Compilation
-----------
In order to compile the JudoAssistant software suite you will need to have
the following dependencies installed on your system:
* qt5
* boost
* zstd
* cereal
* meson
* ninja

After cloning the git repository the software is compiled and installed using the
following series of commands.
```bash
cd <sourcecode_directory>
meson build --prefix=/usr --buildtype=release
ninja -C build
ninja install -C build
```

Dev Setup
---------
The following commands are suggested for setting up a development environment
on a unix system.
```bash
cd <sourcecode_directory>
meson build --prefix=/ -Dweb=true
ninja -C build
DESTDIR=$(pwd) ninja install
```
After compilation and running the install command, the executables can be run
from the `build` directory.

