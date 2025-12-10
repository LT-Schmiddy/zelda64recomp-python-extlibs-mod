import platform, os, shutil, sys
from pathlib import Path
import modbuildcore as mbc

root_dir = Path(__file__).parent
build_dir = root_dir.joinpath("build")
binaries_dir = root_dir.joinpath("binaries")

make_mips_compiler_path = None
make_mips_linker_path = None

# The `mod_project` variable is loaded by tasks.py. `project.py` must have a member named `mod_project`.
mod_project: mbc.ModProjectConfig = mbc.ModProjectConfig(root_dir)

# Deciding with compiler/tool archive to download for your platform:
if platform.system() == "Windows":
    mod_project.add_archive_download_and_extract(
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Windows-AMD64.zip",
        binaries_dir.joinpath("clangmips_win")
    )
    
    make_mips_compiler_path = binaries_dir.joinpath("clangmips_win/nrs_bin/clang.exe")
    make_mips_linker_path = binaries_dir.joinpath("clangmips_win/nrs_bin/ld.lld.exe")
    mod_project.set_mod_tool(binaries_dir.joinpath("clangmips_win/nrs_bin/RecompModTool.exe"))
    
elif platform.system() == "Darwin":
    mod_project.add_archive_download_and_extract(
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Darwin-arm64.tar.xz",
        binaries_dir.joinpath("clangmips_macos")
    )
    
    make_mips_compiler_path = binaries_dir.joinpath("clangmips_macos/nrs_bin/clang")
    make_mips_linker_path = binaries_dir.joinpath("clangmips_macos/nrs_bin/ld.lld")
    mod_project.set_mod_tool(binaries_dir.joinpath("clangmips_win/nrs_bin/RecompModTool"))
    
else:
    mod_project.add_archive_download_and_extract(
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Linux-x86_64.tar.xz",
        binaries_dir.joinpath("clangmips_linux")
    )
    
    make_mips_compiler_path = binaries_dir.joinpath("clangmips_linux/nrs_bin/clang")
    make_mips_linker_path = binaries_dir.joinpath("clangmips_linux/nrs_bin/ld.lld")
    mod_project.set_mod_tool(binaries_dir.joinpath("clangmips_win/nrs_bin/RecompModTool"))

# Registering asset_archive extraction
assets_archive_path = root_dir.joinpath("assets_archive.zip")
assets_extracted_path = root_dir.joinpath("assets_extracted/assets")
mod_project.add_archive_extraction(assets_archive_path, assets_extracted_path)

# Registering mod toml files to build

# Main API NRM
mod_project.add_mod_toml_and_makefile(
    root_dir.joinpath("mod.toml"),
    root_dir.joinpath("mod_elf.mk"),
    {
        "_ELF_PATH": mbc.TomlMakeSpecialVals.TOML_ELF_PATH,
        "_BUILD_DIR": mbc.TomlMakeSpecialVals.TOML_BUILD_DIR,
        "_MIPS_CC": make_mips_compiler_path,
        "_MIPS_LD": make_mips_linker_path,
        "_SRC_DIR": "src/mod",
        "_PY_BUILD_FLAGS": "-DRECOMP_PY_BUILD_MODE"
    }
)

# Tests NRM
mod_project.add_mod_toml_and_makefile(
    root_dir.joinpath("tests.toml"),
    root_dir.joinpath("mod_elf.mk"),
    {
        "_ELF_PATH": mbc.TomlMakeSpecialVals.TOML_ELF_PATH,
        "_BUILD_DIR": mbc.TomlMakeSpecialVals.TOML_BUILD_DIR,
        "_MIPS_CC": make_mips_compiler_path,
        "_MIPS_LD": make_mips_linker_path,
        "_SRC_DIR": "src/tests",
        "_PY_BUILD_FLAGS": ""
    }
)

mod_project.mark_paths_for_clean([
    build_dir
])

mod_project.mark_paths_for_distclean([
    binaries_dir,
    assets_extracted_path
])