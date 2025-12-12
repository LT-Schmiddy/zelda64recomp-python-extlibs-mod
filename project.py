import platform, shutil, enum, os
from pathlib import Path
from modbuildcore.config import *

root_dir: Path = Path(__file__).parent

archive_downloads_dir: Path = root_dir.joinpath("downloads")
build_dir = root_dir.joinpath("build")
binaries_dir = root_dir.joinpath("binaries")

make_path: Path = shutil.which("make")
cmake_path: Path = shutil.which("cmake")

make_mips_compiler_path: Path = None
make_mips_linker_path: Path = None
mod_tool_path: Path = None
zig_dir_path: Path = None
zig_bin_path: Path = None

archive_extractions: dict[str, ArchiveExtractConfig] = {}
downloads: dict[str, DownloadConfig] = {}
makefiles: dict[str, MakefileConfig] = {}
mod_tomls: dict[str, ModTomlConfig] = {}
cmake_projects: dict[str, CMakeProjectConfig] = {}

llvm_path = Path(shutil.which("clang")).parent.parent

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
    mod_tool_path = binaries_dir.joinpath("llvmmips_win/nrs_bin/RecompModTool")
    
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
    llvm_path = binaries_dir.joinpath("llvm_linux/LLVM-19.1.7-macOS-ARM64")
    
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
mod_tomls['mod'] = ModTomlConfig(root_dir.joinpath("mod.toml"))
makefiles['mod'] = MakefileConfig(
    root_dir.joinpath("mod_elf.mk"),
    {
        "_ELF_PATH": str(mod_tomls['mod'].get_elf_path()),
        "_BUILD_DIR": str(mod_tomls['mod'].get_elf_path().parent),
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

extlib = CMakeProjectConfig(
    root_dir,
    "Debug",
    {
        # Unlike with clangmips, we're gonna prepend the LLVM and ZIG directories to the PATH that CMake recieves.
        "PATH": prepend_to_env_path([llvm_path.joinpath("bin"), zig_dir_path]),
        "LIB_NAME": "RecompPythonNative"
    },
)

def native_preset_name(build_type: str):
    if platform.system() == "Windows":
        return f"native-windows-x64-{build_type}"
    if platform.system() == "Darwin":
        return f"native-macos-aarch64-{build_type}"
    if platform.system() == "Linux":
        return f"native-linux-x64-{build_type}"

extlib.build_groups = {
    "Debug" : {
        "Windows": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-windows-x64-Debug"),
        "Darwin": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-macos-aarch64-Debug"),
        "Linux": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-linux-x64-Debug"),
    },
    "Release" : {
        "Windows": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-windows-x64-Release"),
        "Darwin": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-macos-aarch64-Release"),
        "Linux": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-linux-x64-Release"),
    }, 
    "RelWithDebInfo": {
        "Windows": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-windows-x64-RelWithDebInfo"),
        "Darwin": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-macos-aarch64-RelWithDebInfo"),
        "Linux": CMakeBuildConfig.from_preset_pair(extlib, {}, "zig-linux-x64-RelWithDebInfo"),
    },
    "native-Debug" : {
        "Native": CMakeBuildConfig.from_preset_pair(extlib, {}, native_preset_name("Debug")),
    },
    "native-Release" : {
        "Native": CMakeBuildConfig.from_preset_pair(extlib, {}, native_preset_name("Release")),
    }, 
    "native-RelWithDebInfo": {
        "Native": CMakeBuildConfig.from_preset_pair(extlib, {}, native_preset_name("RelWithDebInfo")),
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