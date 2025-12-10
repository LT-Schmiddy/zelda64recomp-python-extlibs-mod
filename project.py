import platform, shutil, enum
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
zig_path: Path = None

archive_extractions: dict[str, ArchiveExtractConfig] = {}
downloads: dict[str, DownloadConfig] = {}
makefiles: dict[str, MakefileConfig] = {}
mod_tomls: dict[str, ModTomlConfig] = {}
cmake_projects: dict[str, CMakeProjectConfig] = {}

# Convienience function for downloading compiler artifacts.
def add_archive_download_and_extract(name: str, url: str, extract_dir: Path) -> tuple[DownloadConfig, ArchiveExtractConfig]:
    global archive_extractions, downloads, archive_downloads_dir
    
    new_download = DownloadConfig(url, archive_downloads_dir, True)
    new_extraction = ArchiveExtractConfig(new_download.download_path, extract_dir)
    downloads[name] = new_download
    archive_extractions[name] = new_extraction
    
    return new_download, new_extraction


class TomlMakeSpecialVals(enum.Enum):
        TOML_ELF_PATH = 0
        TOML_ELF_PARENT_PATH = 1
        TOML_BUILD_DIR = 2

# Deciding with compiler/tool archive to download for your platform:
if platform.system() == "Windows":
    add_archive_download_and_extract(
        "clangmips",
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Windows-AMD64.zip",
        binaries_dir.joinpath("clangmips_win")
    )
    make_mips_compiler_path = binaries_dir.joinpath("clangmips_win/nrs_bin/clang.exe")
    make_mips_linker_path = binaries_dir.joinpath("clangmips_win/nrs_bin/ld.lld.exe")
    mod_tool_path = binaries_dir.joinpath("clangmips_win/nrs_bin/RecompModTool.exe")
    
    add_archive_download_and_extract(
        "zig",
        "https://ziglang.org/download/0.14.1/zig-x86_64-windows-0.14.1.zip",
        binaries_dir.joinpath("zig_win")
    )
    zig_path = binaries_dir.joinpath("zig_win/zig-x86_64-windows-0.14.1/zig.exe")
    
elif platform.system() == "Darwin":
    add_archive_download_and_extract(
        "clangmips",
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Darwin-arm64.tar.xz",
        binaries_dir.joinpath("clangmips_macos")
    )
    make_mips_compiler_path = binaries_dir.joinpath("clangmips_macos/nrs_bin/clang")
    make_mips_linker_path = binaries_dir.joinpath("clangmips_macos/nrs_bin/ld.lld")
    mod_tool_path = binaries_dir.joinpath("clangmips_win/nrs_bin/RecompModTool")
    
    add_archive_download_and_extract(
        "zig",
        "https://ziglang.org/download/0.14.1/zig-aarch64-macos-0.14.1.tar.xz",
        binaries_dir.joinpath("zig_macos")
    )
    zig_path = binaries_dir.joinpath("zig_linux/zig-aarch64-macos-0.14.1/zig")
    
else:
    add_archive_download_and_extract(
        "clangmips",
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Linux-x86_64.tar.xz",
        binaries_dir.joinpath("clangmips_linux")
    )
    
    make_mips_compiler_path = binaries_dir.joinpath("clangmips_linux/nrs_bin/clang")
    make_mips_linker_path = binaries_dir.joinpath("clangmips_linux/nrs_bin/ld.lld")
    mod_tool_path = binaries_dir.joinpath("clangmips_linux/nrs_bin/RecompModTool")
    
    add_archive_download_and_extract(
        "zig",
        "https://ziglang.org/download/0.14.1/zig-x86_64-linux-0.14.1.tar.xz",
         binaries_dir.joinpath("zig_linux")
    )
    zig_path = binaries_dir.joinpath("zig_linux/zig-x86_64-linux-0.14.1/zig")

# Registering asset_archive extraction
assets_archive_path = root_dir.joinpath("assets_archive.zip")
assets_extracted_path = root_dir.joinpath("assets_extracted/assets")
archive_extractions['assets'] = ArchiveExtractConfig(assets_archive_path, assets_extracted_path)

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

extlib = CMakeProjectConfig(root_dir, {
    "ZIG_BINARY": str(zig_path)
})
extlib.add_preset_pair_build_to_group("Debug", "Windows", [], "zig-windows-x64-Debug")
extlib.add_preset_pair_build_to_group("Debug", "Darwin", [], "zig-macos-aarch64-Debug")
extlib.add_preset_pair_build_to_group("Debug", "Linux", [], "zig-linux-x64-Debug")

cmake_projects["extlib"] = extlib

clean_paths: list[Path] = [
    build_dir
]

distclean_paths: list[Path] = [
    binaries_dir,
    archive_downloads_dir,
    assets_extracted_path.parent
]