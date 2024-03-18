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
