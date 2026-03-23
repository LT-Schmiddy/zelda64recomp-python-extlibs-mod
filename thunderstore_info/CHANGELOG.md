# CHANGELOG

## 2.0.0

* Updated to Python 3.14.3
* Now using a free-threaded build of Python. Python code can now take full advantage of multi-threading.
* Added support for BanjoRecompiled, Starfox64Recompiled, Mariokart64Recompiled, and Goemon64Recompiled, each with their own Thunderstore packages.
* Fixed the .nrm containing `\\` in path names, which prevented use of the `repy_api` module on POSIX systems.
* Critical Python files such as the standard library are no longer stored in the mods folder, but instead stored in `python314t` to match the Python version.
* Total rewrite of the build-scripting. Building is now invoked via `./modbuild.py`
* Python thread state is now managed using seperate API calls to improve performance.
* Limited support for subinterpreters.
* Additional C functions are now available for adding `.nrm` files to the module search path or accessing the `.nrm` as a Python `zipfile.ZipFile`.
* Created a deferred cleanup system for components of `REPY_FN`.
* REPY Memcpy functions use Python's buffer protocol for better compatability and performance.
* New, improved icons.
* Improved catching of Python errors.
* Misc fixes and tweaks.

## Version 1.0.0

* Initial release
