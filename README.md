Protégé Launcher - Universal launcher for Protégé
=================================================

This is a binary launcher for the [Protégé Ontology Editor](https://github.com/protegeproject/protege).

Its aim is to provide a single code base to build launchers for all the
supported platforms (currently GNU/Linux x86\_64, macOS x86\_64 and
arm64, and Windows x86\_64).


Setting custom Java options
---------------------------
The recommended way to pass custom Java options to Protégé with this
launcher is to use a `jvm.conf` configuration file which may be located
either:

* in the user’s home directory, under `$HOME/.protege/conf/jvm.conf`;
* in the application directory, under `conf/jvm.conf`.

The launcher will look for the user-specific file first; if no such file
exists, then it will look for the file in the application directory.

The `jvm.conf` file uses a simple `key=value` format. Blank lines and
lines starting with `#` are ignored. Currently allowed keys are:

* `max_heap_size` to set Java’s `-Xmx` option;
* `min_heap_size` to set Java’s `-Xms` option;
* `stack_size` to set Java’s `-Xss` option;
* `append` to set an arbitrary option (may be repeated as needed).

Sample `jvm.conf` file:

```
# Set maximal memory to 4GB
max_heap_size=4G
# Disable class garbage collection
append=-Xnoclassgc
```

For backwards compatibility, if no `jvm.conf` file is found, the macOS
and Windows versions of the launcher will try to use the configuration
methods used in older Protégé versions.

On macOS, the launcher will look for a `JVMOptions` key in the bundle’s
`Info.plist` file. That key should contain an array where each item is
a Java option, as in this example:

```xml
<key>JVMOptions</key>
<array>
    <string>-Xmx8G</string>
    <string>-Xms200M</string>
</array>
```

On Windows, the launcher will look for a `Protege.l4j.ini` file,
containing one option per line (blank lines and lines starting with `#`
are ignored):

```
-Xmx8G
-Xms200M
```


Building the launcher
---------------------

### Building for GNU/Linux

Building under and for GNU/Linux should be straightforward. The only
important thing to be aware of is that the path to the JDK _must_ be
explicitly specified with `--with-jdk`, the configure script will not
try to automatically detect it.

```sh
$ ./configure --with-jdk=<path_to_the_jdk>
$ make
```

The resulting `protege` binary can then be placed inside the directory
containing the GNU/Linux distribution of Protégé.


### Building for macOS

Under macOS, the procedure should be the same as under GNU/Linux. Make
sure you have the _XCode Command Line Tools_ installed, and a JDK
available.

```sh
$ ./configure --with-jdk=<path_to_the_jdk>
$ make
```

The resulting binary is by default a _universal binary_ targeting both
x86\_64 and arm64.

Beware that by default, the compiler seems to generate binaries that can
only run on a version of macOS at least as high as the version on which
the compiler is currently running (so, a binary generated on macOS 13.0
will not run on any macOS < 13.0). Use the `--with-min-osx-version`
option at configure time to change that:

```sh
$ ./configure --with-jdk=<path_to_the_jdk> \
    --with-min-osx-version=11.0
```

Building for macOS under GNU/Linux has been tested to work with the
[OSXCross](https://github.com/tpoechtrager/osxcross) cross-compiling
environment. With OSXcross installed and in the _PATH_, the launcher may
be built with:

```sh
$ CC=o64-clang ./configure --with-jdk=<path_to_the_jdk> \
    --host=x86_64-apple-darwin21.4
$ make
```

Adjust the host triplet depending on the version of the macOS SDK you
have installed with OSXCross. Here, _darwin21.4_ is for the 12.3 SDK.

Beware that even if you build under GNU/Linux, you still need a _macOS_
JDK.


### Building for Windows

Building for Windows has only been tested under GNU/Linux using the
[MinGW64](https://www.mingw-w64.org/) toolchain. With that toolchain
installed and in the _PATH_, build the launcher with:

```sh
$ ./configure --with-jdk=<path_to_the_jdk> \
    --host=x86_64-w64-mingw32
$ make
```

Similarly to cross-compiling for macOS, when cross-compiling for Windows
you need a _Windows_ JDK.


Copying
-------
Most of Protégé Launcher (all source files under `src/`) is distributed
under the terms of the 3-clause BSD license, similar to the license used
by Protégé itself.

Source files for helper and compatibility modules (under `lib/`) are
distributed under the terms of the GNU General Public License, version 3
or higher.
