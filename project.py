import platform, os, shutil, sys
from pathlib import Path
import modbuildcore as mbc

root_dir = Path(__file__).parent
compilers_dir = root_dir.joinpath("compilers")


# The `mod_project` variable is loaded by tasks.py. `project.py` must have a member named `mod_project`.
mod_project: mbc.ModProjectConfig = mbc.ModProjectConfig()

if platform.system() == "Windows":
    mod_project.add_archive_download(
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Windows-AMD64.zip",
        compilers_dir.joinpath("clangmips_win")
    )
    
    mod_project.set_mips_compiler(compilers_dir.joinpath("clangmips_win/nrs_bin/clang.exe"))
    mod_project.set_mips_linker(compilers_dir.joinpath("clangmips_win/nrs_bin/ld.lld.exe"))
    mod_project.set_mod_tool(compilers_dir.joinpath("clangmips_win/nrs_bin/RecompModTool.exe"))
    
elif platform.system() == "Darwin":
    mod_project.add_archive_download(
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Darwin-arm64.tar.xz",
        "clangmips_macos"
    )
    
    mod_project.set_mips_compiler(compilers_dir.joinpath("clangmips_macos/nrs_bin/clang"))
    mod_project.set_mips_linker(compilers_dir.joinpath("clangmips_macos/nrs_bin/ld.lld"))
    mod_project.set_mod_tool(compilers_dir.joinpath("clangmips_win/nrs_bin/RecompModTool"))
    
else:
    mod_project.add_archive_download(
        "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Linux-x86_64.tar.xz",
        "clangmips_linux"
    )
    
    mod_project.set_mips_compiler(compilers_dir.joinpath("clangmips_linux/nrs_bin/clang"))
    mod_project.set_mips_linker(compilers_dir.joinpath("clangmips_linux/nrs_bin/ld.lld"))
    mod_project.set_mod_tool(compilers_dir.joinpath("clangmips_win/nrs_bin/RecompModTool"))
    
mod_project.add_mod_toml(root_dir.joinpath("mod.toml"), root_dir.joinpath("mod_elf.mk"))