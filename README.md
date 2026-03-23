# Main Page {#main_page}

RecompExternalPython (also known as REPY) is a library designed to solve an persistent problem in the N64Recompiled ecosystem: the necessity of external libraries and the headaches they induce.

## Why REPY {#why_repy}

In cases where an N64Recompiled mod needs to 'break containment' and access system resources that wouldn't have been available to an Nintendo 64 (such as file I/O, networking, getting the system time, etc), the de facto solution is to compile an external shared library (commonly referred to as an 'extlib' by the N64Recompiled modding community), and package that alongside the mod's `.nrm` file. This solution does work, but has several major drawbacks for mod developers. Some of these issues are:

* Extlibs need to be natively compiled for each platform that the mod developer wants to support, and then distributed with the `.nrm`. That means producing a `.dll` file for Windows, a `.dylib` file for MacOS, and a `.so` file for Linux users.  Mod templates and build scripts to facilitate cross-compilation do exist, but it's still an imperfect solution that can result in bloated project folders and slow build times.
* Exchanging data between the recompiled game memory and the system memory suffers from multiple restrictions, including the inability to allocate in the recompiled memory space from the extlib code, the byte-swapping required when translating between recompiled memory and regular memory, the restriction that extlib functions (the ones exposed to mod memory) can only have four 32-bit arguments without (without getting into complicated operations involving the recompiled stack pointer).
* Extlib code cannot call recompiled functions, be they functions from the original game or functions created by the mod. Navigating this restriction necessitates creating very awkward code paths and complicated application architecture that can become difficult to maintain.

REPY aims to provide modders with an easier alternative to creating their own extlibs. By embedding the runtime for an interpreted programming language (specifically, Python) into an N64Recompiled extlib of its own,mods can instruct REPY to interact with the host system on their behalf. In essense, REPY is single extlib that encompasses the vast majority of cases where extlibs would be required.

This enables REPY to provide easy solutions to the problems above:

* REPY comes already compiled for every platform that N64Recompiled games support, meaning that modders don't need to deal with cross-compilation. All of the Python code is contained within the `.nrm` itself, making distribution much easier.
* REPY automatically handles the complications of copying data between the native system (in this case, the Python interpreter) and recompiled memory. When using the C API, REPY is able allocate space in recompiled memory automatically when needed.
* While it not possible for the Python interpreter to call into recompiled code, REPY circumvments that issue by allowing modders to control the Python interpreter from inside C functions. In fact, the `REPY_FN` macro collection enables modders to seamlessly interweave recompiled C code and Python extlib code within the same function.

What REPY will NOT do is enable you to write an entire mod with Python. There are still aspects of N64Recompiled modding that must be handled through recompiled mod code, such as hooks and patches, that REPY cannot handle using Python. REPY merely offers an easier way to do things that mod code can't do alone.

## Usage

## ORIGINAL - LTSchmiddy's Majora's Mask: Recompiled Mod Template

This my custom version of the Majora's Mask: Recompiled mod template. It offers a number of features that the base template doesn't have, including:

* Optional building of external libraries (referred to as extlibs) alongside the mod nrm, and keeping the code for both in the same repository.
* Cross-compilation of extlibs using Zig (extlib code is still written in C/C++).
* Dedicated testing environment for mods in the form of the `runtime` directory.
* Automatic creation of Thunderstore packages via a script, or by running `make thunderstore`.
* Easy integration of non-standard clang versions (such the MIPS-only `clang` package I maintain), in case your system `clang` doesn't support MIPS.

### Writing mods

See [this document](https://hackmd.io/fMDiGEJ9TBSjomuZZOgzNg) for an explanation of the modding framework, including how to write function patches and perform interop between different mods.

### Tools

This template has somewhat different requirements from the default mod template. In order to run it, you'll need the following:

* `make`
* `cmake`
* `ninja`
* `python` (or `python3` on POSIX systems).

**You do NOT need the `RecompModTool` tool or any special compilers, as the build script will compile all of the N64Recomp tools for you.**

* On Windows, using [chocolatey](https://chocolatey.org/) to install everything is recommended.
* On Linux, these can both be installed using your distro's package manager.
* On MacOS, these can both be installed using Homebrew.

### Building

Currently, building is only supported on x86-64 Windows and Linux, and arm64 MacOS.

Run `git submodule update --init --recursive` to make sure you've clones all submodules. Then, run `./modbuild.py` to create a debug build. 
Use `./modbuild.py thunderstore` to create the release packages.

For more build options, use `./modbuild.py -h` and for help information

### Extlib Compilation, Cross-Compilation, and CMake Presets

This template is set up to automatically build and cross-compile your extlib code (via CMake and Zig) alongside your mod code .
CMake presets are used to handle any configuration differences between target platforms ([More info about CMake presets can be found
here](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)).

### Testing

This template includes handling of dedicated testing environments for this mod in the form of the `./test_env` folder. First, Copy in the `assets` from the recomp (or just the entire recomp) you want to test against into the folder corresponding to its game id (`./test_env/[game_id]`), along with any config files, saves, and other mods you want to test against. Then, create a file called `./test_env/[game_id]/portable.txt`. After a build, the mod's `.nrm` file and extlib files will be copied to a folder called `./test_env/[game_id]/mods`.  Once that's been done, you use `runtime` and as your CWD, everything's ready to go for immediate testing as soon as your build finishes.

