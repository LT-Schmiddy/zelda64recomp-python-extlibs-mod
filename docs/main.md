# Main Page {#main_page}

RecompExternalPython (also known as REPY) is a library designed to solve an persistent problem in the N64Recompiled ecosystem: the necessity of external libraries and the headaches they induce.

This webside contains the documentation for the project. This is currently a first draft, with more comprehensive tutorials to be written in the future.

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

## Quickstart {#repy_quickstart}

Simply include the `repy_api.h` header in your mod's source files, and add `RecompExternalPython:2.0.0` as a dependency for your mod in your `mod.toml`.

The `REPY_FN` macros are the recommended interface for interacting with the Python interpreter from recompiled code. See the [`REPY_FN` Overview](\ref repy_fn_overview) for a quick overview of how to use it.

Additionally, while REPY is fully capable of running Python code at the speeds required for N64Recompiled games, there are a few known performance pitfalls one should avoid. See the [Performance Considerations](\ref repy_performance) page for more information.
