JudoAssistant
=============
[![Build Status](https://ci.svendcs.com/api/badges/judoassistant/judoassistant/status.svg)](https://ci.svendcs.com/judoassistant/judoassistant)

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

Dev Web Backend Setup
---------------------
The JudoAssistant web server relies on a postgres backend and uses
[Alembic](https://alembic.sqlalchemy.org/en/latest/) for database migrations.
After configuring the postgresql server copy `alembic.ini.example` to
`alembic.ini` and edit the login credentials.
Afterwards run the command `alembic upgrade head` to migrate to the latest
database schema.

Before running the web server copy the file `data/example-server-config.cfg`
into `build/config.cfg` and update the config variables appropriately.
You can then finally run the web server using `./judoassistant-web -c config.cfg`.

