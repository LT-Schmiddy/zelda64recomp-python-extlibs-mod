import platform, shutil, enum, os
from pathlib import Path
from modbuildcore.config import *

root_dir: Path = Path(__file__).parent

archive_downloads_dir: Path = root_dir.joinpath("downloads")
build_dir: Path = root_dir.joinpath("build")
binaries_dir: Path = root_dir.joinpath("binaries")

make_path: Path = shutil.which("make")
cmake_path: Path = shutil.which("cmake")

make_mips_compiler_path: Path = None
make_mips_linker_path: Path = None
mod_tool_path: Path = None
zig_dir_path: Path = None
zig_bin_path: Path = None
llvm_path: Path = None

archive_extractions: dict[str, ArchiveExtractConfig] = {}
downloads: dict[str, DownloadConfig] = {}
makefiles: dict[str, MakefileConfig] = {}
mod_tomls: dict[str, ModTomlConfig] = {}
cmake_projects: dict[str, CMakeProjectConfig] = {}

test_env_mod_dir: Path = root_dir.joinpath("test_env/mods")
thunderstore_package_dir: Path = root_dir.joinpath("thunderstore_package")

# If you need this enabled, you should probably rethink whatever it is you're doing:
# Convienience function for downloading compiler artifacts.
def add_archive_download_and_extract(name: str, url: str, extract_dir: Path) -> tuple[DownloadConfig, ArchiveExtractConfig]:
    global archive_extractions, downloads, archive_downloads_dir
    
    new_download = DownloadConfig(url, archive_downloads_dir, True)
    new_extraction = ArchiveExtractConfig(new_download.download_path, extract_dir)
    downloads[name] = new_download
    archive_extractions[name] = new_extraction
    
    return new_download, new_extraction

# Deciding with compiler/tool archive to download for your platform:
if platform.system() == "Windows":
    add_archive_download_and_extract(
        "llvmmips",
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Windows-AMD64.zip",
        binaries_dir.joinpath("llvmmips_win")
    )
    make_mips_compiler_path = binaries_dir.joinpath("llvmmips_win/nrs_bin/clang.exe")
    make_mips_linker_path = binaries_dir.joinpath("llvmmips_win/nrs_bin/ld.lld.exe")
    mod_tool_path = binaries_dir.joinpath("llvmmips_win/nrs_bin/RecompModTool.exe")
    
    add_archive_download_and_extract(
        "zig",
        "https://ziglang.org/download/0.14.1/zig-x86_64-windows-0.14.1.zip",
        binaries_dir.joinpath("zig_win")
    )
    zig_dir_path = binaries_dir.joinpath("zig_win/zig-x86_64-windows-0.14.1")
    zig_bin_path = zig_dir_path.joinpath("zig.exe")
    

    add_archive_download_and_extract(
        "llvm",
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-19.1.7/clang+llvm-19.1.7-x86_64-pc-windows-msvc.tar.xz",
        binaries_dir.joinpath("llvm_win")
    )
    
    llvm_path = binaries_dir.joinpath("llvm_win/clang+llvm-19.1.7-x86_64-pc-windows-msvc")
        
    
elif platform.system() == "Darwin":
    add_archive_download_and_extract(
        "llvmmips",
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Darwin-arm64.tar.xz",
        binaries_dir.joinpath("llvmmips_macos")
    )
    make_mips_compiler_path = binaries_dir.joinpath("llvmmips_macos/nrs_bin/clang")
    make_mips_linker_path = binaries_dir.joinpath("llvmmips_macos/nrs_bin/ld.lld")
    mod_tool_path = binaries_dir.joinpath("llvmmips_macos/nrs_bin/RecompModTool")
    
    add_archive_download_and_extract(
        "zig",
        "https://ziglang.org/download/0.14.1/zig-aarch64-macos-0.14.1.tar.xz",
        binaries_dir.joinpath("zig_macos")
    )
    zig_dir_path = binaries_dir.joinpath("zig_linux/zig-aarch64-macos-0.14.1")
    zig_bin_path = zig_dir_path.joinpath("zig")
    
    
    add_archive_download_and_extract(
        "llvm",
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-19.1.7/LLVM-19.1.7-macOS-ARM64.tar.xz",
        binaries_dir.joinpath("llvm_macos")
    )
    llvm_path = binaries_dir.joinpath("llvm_macos/LLVM-19.1.7-macOS-ARM64")
    
else:
    add_archive_download_and_extract(
        "llvmmips",
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Linux-x86_64.tar.xz",
        binaries_dir.joinpath("llvmmips_linux")
    )
    
    make_mips_compiler_path = binaries_dir.joinpath("llvmmips_linux/nrs_bin/clang")
    make_mips_linker_path = binaries_dir.joinpath("llvmmips_linux/nrs_bin/ld.lld")
    mod_tool_path = binaries_dir.joinpath("llvmmips_linux/nrs_bin/RecompModTool")
    
    add_archive_download_and_extract(
        "zig",
        "https://ziglang.org/download/0.14.1/zig-x86_64-linux-0.14.1.tar.xz",
         binaries_dir.joinpath("zig_linux")
    )
    zig_dir_path = binaries_dir.joinpath("zig_linux/zig-x86_64-linux-0.14.1")
    zig_bin_path = binaries_dir.joinpath("zig")
    
    add_archive_download_and_extract(
        "llvm",
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-19.1.7/LLVM-19.1.7-Linux-X64.tar.xz",
        binaries_dir.joinpath("llvm_linux")
    )
    llvm_path = binaries_dir.joinpath("llvm_linux/LLVM-19.1.7-Linux-X64")

# Registering asset_archive extraction
assets_archive_path = root_dir.joinpath("assets_archive.zip")
assets_extracted_path = root_dir.joinpath("assets_extracted/assets")
archive_extractions['assets'] = ArchiveExtractConfig(assets_archive_path, assets_extracted_path)

def prepend_to_env_path(to_append: Path) -> str:
    global llvm_path
    PATH_DELIMITER = ";" if os.name == 'nt' else ":"
    env_path = os.environ['PATH']
    for i in to_append:
        env_path = str(i) + PATH_DELIMITER + env_path
    return env_path

# Registering mod toml files to build
# Main API NRM

main_toml_key = 'mod'
mod_tomls[main_toml_key] = ModTomlConfig(root_dir.joinpath("mod.toml"))
makefiles[main_toml_key] = MakefileConfig(
    root_dir.joinpath("mod_elf.mk"),
    {
        "_ELF_PATH": str(mod_tomls[main_toml_key].get_elf_path()),
        "_BUILD_DIR": str(mod_tomls[main_toml_key].get_elf_path().parent),
        "_MIPS_CC": str(make_mips_compiler_path),
        "_MIPS_LD": str(make_mips_linker_path),
        "_SRC_DIR": "src/mod",
        "_PY_BUILD_FLAGS": "-DRECOMP_PY_BUILD_MODE"
    }
)

# Tests NRM
mod_tomls['tests'] = ModTomlConfig(root_dir.joinpath("tests.toml"))
makefiles['tests'] = MakefileConfig(
    root_dir.joinpath("mod_elf.mk"),
    {
        "_ELF_PATH": str(mod_tomls['tests'].get_elf_path()),
        "_BUILD_DIR": str(mod_tomls['tests'].get_elf_path().parent),
        "_MIPS_CC": str(make_mips_compiler_path),
        "_MIPS_LD": str(make_mips_linker_path),
        "_SRC_DIR": "src/tests",
        "_PY_BUILD_FLAGS": ""
    }
)

cmake_default_build_group = "Debug"
cmake_thunderstore_build_group = "Release"

extlib_name = "RecompPythonNative"
extlib = CMakeProjectConfig(
    root_dir,
    {
        # Unlike with clangmips, we're gonna prepend the LLVM and ZIG directories to the PATH that CMake recieves.
        "PATH": prepend_to_env_path([llvm_path.joinpath("bin"), zig_dir_path]),
        "LIB_NAME": extlib_name
    },
)

def get_preset_lib_path(preset_name: str) -> Path:
    global root_dir
    return root_dir.joinpath(f"build/{preset_name}/lib")

def with_windows_dlls(preset_string: str, output_paths: dict[Path, Path]) -> dict[Path, Path]:
    global root_dir
    windows_dll_names = [
        "libcrypto-3-x64.dll",
        "libffi-8.dll",
        "libssl-3-x64.dll",
        "pyexpat.pyd",
        "select.pyd",
        "sqlite3.dll",
        "tcl86t.dll",
        "tk86t.dll",
        "unicodedata.pyd",
        "winsound.pyd",
        "_asyncio.pyd",
        "_bz2.pyd",
        "_ctypes.pyd",
        "_ctypes_test.pyd",
        "_decimal.pyd",
        "_elementtree.pyd",
        "_hashlib.pyd",
        "_lzma.pyd",
        "_multiprocessing.pyd",
        "_overlapped.pyd",
        "_queue.pyd",
        "_socket.pyd",
        "_sqlite3.pyd",
        "_ssl.pyd",
        "_testbuffer.pyd",
        "_testcapi.pyd",
        "_testclinic.pyd",
        "_testclinic_limited.pyd",
        "_testconsole.pyd",
        "_testimportmultiple.pyd",
        "_testinternalcapi.pyd",
        "_testlimitedcapi.pyd",
        "_testmultiphase.pyd",
        "_testsinglephase.pyd",
        "_tkinter.pyd",
        "_uuid.pyd",
        "_wmi.pyd",
        "_zoneinfo.pyd",
    ]
    
    python_dll_dir =  root_dir.joinpath(f"build/{preset_string}/python-standalone/python/DLLs")
    
    for file in [python_dll_dir.joinpath(i) for i in windows_dll_names]:
        if file.suffix == ".dll":
            output_paths[file] = Path(file.name)
        elif file.suffix == ".pyd":
            output_paths[file] = Path(file.with_suffix(".dll").name)
            
    return output_paths

def native_preset_name(build_type: str):
    if platform.system() == "Windows":
        return f"native-windows-x64-{build_type}"
    if platform.system() == "Darwin":
        return f"native-macos-aarch64-{build_type}"
    if platform.system() == "Linux":
        return f"native-linux-x64-{build_type}"

def native_output_files(build_type: str) -> dict[Path, Path]:
    preset_name = native_preset_name(build_type)
    if platform.system() == "Windows":
        return with_windows_dlls(preset_name, {
            get_preset_lib_path(preset_name).joinpath("python313.dll"): Path("python313.dll"),
            get_preset_lib_path(preset_name).joinpath(f"{extlib_name}.dll"): Path(f"{extlib_name}.dll"),
            get_preset_lib_path(preset_name).joinpath(f"{extlib_name}.pdb"): Path(f"{extlib_name}.pdb")
        })
    if platform.system() == "Darwin":
        return {
            get_preset_lib_path(preset_name).joinpath("libpython3.13.dylib"): Path("libpython3.13.dylib"),
            get_preset_lib_path(preset_name).joinpath(f"{extlib_name}.dylib"): Path(f"{extlib_name}.dylib")
        }
    if platform.system() == "Linux":
        return {
            get_preset_lib_path(preset_name).joinpath("libpython3.13.so"): Path("libpython3.13.so"),
            get_preset_lib_path(preset_name).joinpath(f"lib{extlib_name}.so"): Path(f"{extlib_name}.so")
        }
    

extlib.build_groups = {
    "Debug" : {
        "Windows": CMakeBuildConfig.from_preset_pair(extlib, with_windows_dlls("zig-windows-x64-Debug", {
                get_preset_lib_path("zig-windows-x64-Debug").joinpath("python313.dll"): Path("python313.dll"),
                get_preset_lib_path("zig-windows-x64-Debug").joinpath(f"lib{extlib_name}.dll"): Path(f"{extlib_name}.dll"),
                get_preset_lib_path("zig-windows-x64-Debug").joinpath(f"lib{extlib_name}.pdb"): Path(f"{extlib_name}.pdb")
            }), "zig-windows-x64-Debug"),
        "Darwin": CMakeBuildConfig.from_preset_pair(extlib, {
                get_preset_lib_path("zig-macos-aarch64-Debug").joinpath("libpython3.13.dylib"): Path("libpython3.13.dylib"),
                get_preset_lib_path("zig-macos-aarch64-Debug").joinpath(f"lib{extlib_name}.dylib"): Path(f"{extlib_name}.dylib")
            }, "zig-macos-aarch64-Debug"),
        "Linux": CMakeBuildConfig.from_preset_pair(extlib, {
                get_preset_lib_path("zig-linux-x64-Debug").joinpath("libpython3.13.so"): Path("libpython3.13.so"),
                get_preset_lib_path("zig-linux-x64-Debug").joinpath(f"lib{extlib_name}.so"): Path(f"{extlib_name}.so")
            }, "zig-linux-x64-Debug"),
    },
    "Release" : {
        "Windows": CMakeBuildConfig.from_preset_pair(extlib, with_windows_dlls("zig-windows-x64-Release", {
                get_preset_lib_path("zig-windows-x64-Release").joinpath("python313.dll"): Path("python313.dll"),
                get_preset_lib_path("zig-windows-x64-Release").joinpath(f"lib{extlib_name}.dll"): Path(f"{extlib_name}.dll")
            }), "zig-windows-x64-Release"),
        "Darwin": CMakeBuildConfig.from_preset_pair(extlib, {
                get_preset_lib_path("zig-macos-aarch64-Release").joinpath("libpython3.13.dylib"): Path("libpython3.13.dylib"),
                get_preset_lib_path("zig-macos-aarch64-Release").joinpath(f"lib{extlib_name}.dylib"): Path(f"{extlib_name}.dylib")
            }, "zig-macos-aarch64-Release"),
        "Linux": CMakeBuildConfig.from_preset_pair(extlib, {
                get_preset_lib_path("zig-linux-x64-Release").joinpath("libpython3.13.so"): Path("libpython3.13.so"),
                get_preset_lib_path("zig-linux-x64-Release").joinpath(f"lib{extlib_name}.so"): Path(f"{extlib_name}.so")
            }, "zig-linux-x64-Release"),
    }, 
    "RelWithDebInfo": {
        "Windows": CMakeBuildConfig.from_preset_pair(extlib, with_windows_dlls("zig-windows-x64-RelWithDebInfo", {
                get_preset_lib_path("zig-windows-x64-RelWithDebInfo").joinpath("python313.dll"): Path("python313.dll"),
                get_preset_lib_path("zig-windows-x64-RelWithDebInfo").joinpath(f"lib{extlib_name}.dll"): Path(f"{extlib_name}.dll"),
                get_preset_lib_path("zig-windows-x64-RelWithDebInfo").joinpath(f"lib{extlib_name}.pdb"): Path(f"{extlib_name}.pdb")
            }), "zig-windows-x64-RelWithDebInfo"),
        "Darwin": CMakeBuildConfig.from_preset_pair(extlib, {
                get_preset_lib_path("zig-macos-aarch64-RelWithDebInfo").joinpath("libpython3.13.dylib"): Path("libpython3.13.dylib"),
                get_preset_lib_path("zig-macos-aarch64-RelWithDebInfo").joinpath(f"lib{extlib_name}.dylib"): Path(f"{extlib_name}.dylib")
            }, "zig-macos-aarch64-RelWithDebInfo"),
        "Linux": CMakeBuildConfig.from_preset_pair(extlib, {
                get_preset_lib_path("zig-linux-x64-RelWithDebInfo").joinpath("libpython3.13.so"): Path("libpython3.13.so"),
                get_preset_lib_path("zig-linux-x64-RelWithDebInfo").joinpath(f"lib{extlib_name}.so"): Path(f"{extlib_name}.so")
            }, "zig-linux-x64-RelWithDebInfo"),
    },
    "native-Debug" : {
        "Native": CMakeBuildConfig.from_preset_pair(extlib, native_output_files("Debug"), native_preset_name("Debug")),
    },
    "native-Release" : {
        "Native": CMakeBuildConfig.from_preset_pair(extlib, native_output_files("Release"), native_preset_name("Release")),
    }, 
    "native-RelWithDebInfo": {
        "Native": CMakeBuildConfig.from_preset_pair(extlib, native_output_files("RelWithDebInfo"), native_preset_name("RelWithDebInfo")),
    }
}
cmake_projects["extlib"] = extlib

clean_paths: list[Path] = [
    build_dir
]

distclean_paths: list[Path] = [
    binaries_dir,
    archive_downloads_dir,
    assets_extracted_path.parent
]