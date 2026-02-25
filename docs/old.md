# Python API for N64Recompiled {#mainpage}

In cases where an N64Recompiled mod needs to 'break containment' and access system resources (such as file I/O, networking, getting the system time, etc),
the de facto solution is to compile an external shared library (commonly referred to as an 'extlib' in N64Recomp jargon), and package that alongside the nrm file.
This solution does work, but has several major drawbacks for mod developers:

* Shared libraries need to be natively compiled for each platform that the mod developer wants to support. That means producing a `.dll` file for Windows, a `.dylib` file for MacOS, and a `.so` file for Linux users.  Mod templates and build scripts to facilitate cross-compilation do exist, but it's still not perfect.
* Exchanging data between the recompiled game memory and the system memory suffers from multiple restrictions, including the inability to allocate mod-accessable memory from the extlib code, the byte-swapping required when translating between recompiled memory and regular memory, the restriction on extlib functions (the ones exposed to mod memory) can only have four 32-bit arguments.
* Extlib code cannot call recompiled functions, be they functions from the original game or functions created by the mod. Navigating this restriction necessitates creating very awkward code paths and complicated application architecture that can become difficult to maintain.

This API attempts to mitigate these problems for mod developers...
