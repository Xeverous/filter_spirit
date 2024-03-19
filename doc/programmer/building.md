# building

Some of project's dependencies are already included in git submodules. If you performed only a shallow clone, do `git submodule update --init`.

## toolchain requirements

- C++17 compiler
- CMake 3.13+

Note that:

- Due to some bugs, the earliest working GCC version is 9.0.
- MSVC toolchain is not being used and not tested.

## external library requirements

- Boost 1.70+:
  - spirit (FS uses newest X3 parser)
  - fusion
  - preprocessor
  - optional
  - variant
  - **program_options** (only if you build command-line executable)
  - date_time
  - system
  - unit_test_framework (only if you build tests)
- **libcurl** 7.17+
- **OpenSSL** 1.1+ or other libcurl-compatible SSL implementation

Bolded dependencies need to be build. Rest are header-only libraries.

Package names for Ubuntu-derived distros (`sudo apt install`):

- OpenSSL: `libssl-dev`
- OpenGL headers: `libgl1-mesa-dev`
- SDL2: `libsdl2-dev`

## invokation

Just use CMake as it is supposed to be used:

```bash
mkdir build
cd build
cmake .. [options...]
make -j
```

You may want to tweak `FILTER_SPIRIT_*` CMake variables to decide what to build and with what options. Read main `CMakeLists.txt` for more info.

For Emscripten, use it as emscripten recommends: `emcmake cmake ...` and `emmake make ...`.

## troubleshooting - missing Windows-specific libraries

```
/usr/bin/ld: cannot find -lComdlg32
/usr/bin/ld: cannot find -lOle32
```

You are building with Windows configuration on a unix-like system. Add `-DFILTER_SPIRIT_PLATFORM=sdl2` to `cmake` command and then `make` again.

## troubleshooting - missing Windows-specific libraries

```
/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/10/libstdc++.so: undefined reference to symbol 'pthread_create@@GLIBC_2.2.5'
```

Add `-DFILTER_SPIRIT_ADD_PTHREAD_FLAGS=1` to cmake command.

## troubleshooting - missing linker plugin

```
/usr/bin/ld: plugin needed to handle lto object
```

A linker plugin is needed to handle LTO in the compilation. The simplest way is to disable link-time optimization (`-DFILTER_SPIRIT_ENABLE_LTO=0`) but first try if you can fix it by enabling compiler-specific toolchain programs that support linker plugins:

```bash
# for GCC
-DCMAKE_AR=/usr/bin/gcc-ar
# for Clang
-DCMAKE_RANLIB=/usr/bin/llvm-ranlib
-DCMAKE_AR=/usr/bin/llvm-ar
```

You may also want to check https://stackoverflow.com/questions/39236917/using-gccs-link-time-optimization-with-static-linked-libraries.
