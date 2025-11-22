# Installation Instructions

**AMX** has two components:
1. `amx-core` - the C backend (WAV parsing, DSP, encoding/decoding)
2. `amx` - the Python CLI (program wrapper and ML logic for encoding/decoding)

The `amx` command will automatically dispatch to `amx-core`.

---

## Requirements

### Build Tools
- C compiler (GCC)
- CMake **3.16+**
- Python **3.8+**
- pip

> [!NOTE]
> Other compilers like Clang and MSVC have *not* been tested on this code.
> Use at your own risk.

### Python Dependencies

> [!TIP]
> Install Python through a virtual environment (`python3 -m venv`) to avoid
> polluting system packages.

---

## Building `amx-core`

From the source code root (`/src`):

```bash
$ mkdir build
$ cd build
$ cmake -GNinja ..
$ ninja
```

Which produces the executable `build/amx-core`.

> [!NOTE]
> Make sure you either add the build directory to your path like or install it
> to your `/usr/local/bin` directory so `amx` can find the executable.

---

## Installing `amx`

From the source code root (`/src`)

```bash
$ pip install -e .
```

This installs the `amx` command globally in your active Python environment.
