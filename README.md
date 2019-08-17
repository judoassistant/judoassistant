JudoAssistant
=============
[![Build status](https://ci.appveyor.com/api/projects/status/hi862gr87iyd2x0q/branch/master?svg=true)](https://ci.appveyor.com/project/svendcs/judoassistant/branch/master)

JudoAssistant is free and open source software for running judo tournaments.
The software is currently in the pre-alpha stage and no official releases are
available.

Getting Started
---------------
Currently the only way to get started is to compile the software yourself and
figure things out. At a later point in time a manual and official releases will
be available at [judoassistant.com](https://judoassistant.com/).

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

The applications are then compiled using `meson` and `ninja`:
```bash
cd <sourcecode_directory>
meson builddir -Dweb=false
ninja -C builddir
```

