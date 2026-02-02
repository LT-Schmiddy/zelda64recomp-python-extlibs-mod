# CHANGELOG

## 2.0.0

* Updated to Python 3.14.2
* Added support for BanjoRecompiled and Starfox64Recompiled, with their own Thunderstore packages.
* Fixed the .nrm containing `\\` in path names, which prevented use of the `repy_api` module.
* Changed the name of the `PyDlls` folder to `python314`, to match the python version.
* Total rewrite of the build-scripting. Building is now invoked via `./modbuild.py`

## Version 1.0.0

* Initial release
