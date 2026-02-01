import platform, shutil, enum, os, subprocess
from pathlib import Path
from modbuildcore.jobs import *
from modbuildcore.job_base import JobBase
import toml

root_dir: Path = Path(__file__).parent

archive_downloads_dir: Path = root_dir.joinpath("downloads")
build_dir: Path = root_dir.joinpath("build")
binaries_dir: Path = root_dir.joinpath("binaries")

mod_build_dir = build_dir.joinpath("mod")
tests_build_dir = build_dir.joinpath("tests")

make_mips_compiler_path: Path = None
make_mips_linker_path: Path = None
mod_tool_path: Path = None
zig_dir_path: Path = None
llvm_path: Path = None

downloads: dict[str, DownloadJob] = {}
archive_extractions: dict[str, ArchiveExtractJob] = {}
makefiles: dict[str, MakefileJob] = {}
tomls: dict[str, GenerateTomlJob] = {}
nrms: dict[str, ModToNRMJob] = {}
cmake_build_groups: dict[str, dict[str, CMakeBuildJob]] = {}
build_outputs: dict[str, BuildOutputJob] = {}
thunderstore_packages: dict[str, ThunderstorePackageJob] = {}

nrm_path_fix_by_default = True
project_name = "N64RecompExternalPython_API"
project_tests_name = "Test_" + project_name
project_version_string = "2.0.0"

mod_elf_path = mod_build_dir.joinpath("mod.elf")
tests_elf_path = tests_build_dir.joinpath("tests.elf")

mm_toml_data = {
    "manifest": {
        "minimum_recomp_version": "1.2.1"
    },
    "inputs": {
        "func_reference_syms_file": str(root_dir.joinpath("./syms/Zelda64RecompSyms/mm.us.rev1.syms.toml")),
        "data_reference_syms_files": [ 
            str(root_dir.joinpath("./syms/Zelda64RecompSyms/mm.us.rev1.datasyms.toml")),
            str(root_dir.joinpath("./syms/Zelda64RecompSyms/mm.us.rev1.datasyms_static.toml")),
        ]
    }
}

bk_toml_data = {
    "manifest": {
        "minimum_recomp_version": "0.0.1"
    },
    "inputs": {
        "func_reference_syms_file": str(root_dir.joinpath("./syms/BanjoRecompSyms/bk.us.rev0.syms.toml")),
        "data_reference_syms_files": [ 
            str(root_dir.joinpath("./syms/BanjoRecompSyms/bk.us.rev0.datasyms.toml"))
        ]
    }
}

sf64_toml_data = {
    "manifest": {
        "minimum_recomp_version": "1.0.0"
    },
    "inputs": {
        "func_reference_syms_file": str(root_dir.joinpath("./syms/Starfox64RecompSyms/sf64.us.rev1.syms.toml")),
        "data_reference_syms_files": [ 
            str(root_dir.joinpath("./syms/Starfox64RecompSyms/sf64.us.rev1.datasyms.toml"))
        ]
    }
}

mod_toml_data = {
    "manifest": {
        "id": project_name,
        "version": project_version_string,
        "dependencies": []
    },
    "inputs": {
        "elf_path": str(mod_elf_path),
        "additional_files": [ 
            str(root_dir.joinpath("thumb.dds")),
            str(root_dir.joinpath("./src/repy_api")) # The repy_api module
        ]
    }
}

tests_toml_data = {
    "manifest": {
        "id": project_tests_name,
        "version": project_version_string,
        "dependencies": [
            f"{project_name}:{project_version_string}"
        ]
    },
    "inputs": {
        "elf_path": str(tests_elf_path),
        "additional_files": []
    }
}

manifest_id_dict = lambda mod_id: {"manifest": {"id": mod_id}}
manifest_gameid_dict = lambda game_id: {"manifest": {"game_id": game_id}}
inputs_modname_dict = lambda name: {"inputs": {"mod_filename": name}}

# If you need this enabled, you should probably rethink whatever it is you're doing:
# Convienience function for downloading compiler artifacts.
def add_archive_download_and_extract(name: str, url: str, extract_dir: Path) -> tuple[DownloadJob, ArchiveExtractJob]:
    global archive_extractions, downloads, archive_downloads_dir
    
    new_download = DownloadJob(url, archive_downloads_dir)
    new_extraction = ArchiveExtractJob(new_download.download_path, extract_dir)
    new_extraction.depends_on([new_download])
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

def prepend_to_env_path(to_append: Path) -> str:
    global llvm_path
    PATH_DELIMITER = ";" if os.name == 'nt' else ":"
    env_path = os.environ['PATH']
    for i in to_append:
        env_path = str(i) + PATH_DELIMITER + env_path
    return env_path

# ELF Binaries:
makefiles['mod'] = mod_makefile = MakefileJob(
    root_dir.joinpath("mod_elf.mk"),
    {
        "_ELF_PATH": str(mod_elf_path),
        "_BUILD_DIR": str(mod_build_dir),
        "_MIPS_CC": str(make_mips_compiler_path),
        "_MIPS_LD": str(make_mips_linker_path),
        "_SRC_DIR": "src/mod",
        "_PY_BUILD_FLAGS": "-DRECOMP_PY_BUILD_MODE"
    }
)

makefiles['tests'] = tests_makefile = MakefileJob(
    root_dir.joinpath("mod_elf.mk"),
    {
        "_ELF_PATH": str(tests_elf_path),
        "_BUILD_DIR": str(tests_build_dir),
        "_MIPS_CC": str(make_mips_compiler_path),
        "_MIPS_LD": str(make_mips_linker_path),
        "_SRC_DIR": "src/tests",
        "_PY_BUILD_FLAGS": ""
    }
)

def add_toml_and_nrm_job(game_id: str, toml_type: str, nrm_base_name: str, build_dir: Path, data_dicts: list[dict], nrm_dependencies: list[JobBase]) -> tuple[GenerateTomlJob, ModToNRMJob]:
    global tomls, nrms
    
    mod_key = f"{game_id}_{toml_type}"
    
    nrm_build_dir = build_dir.joinpath(game_id)
    toml_path = nrm_build_dir.joinpath(f"{mod_key}.toml")
    
    tomls[mod_key] = mod_toml = GenerateTomlJob.from_merged_dicts(toml_path, data_dicts + [inputs_modname_dict(f"{game_id}_{nrm_base_name}"), manifest_gameid_dict(game_id)])
    nrms[mod_key] = mod_nrm = ModToNRMJob(mod_tool_path, toml_path, nrm_build_dir, delay_read=True)
    mod_nrm.depends_on([mod_toml] + nrm_dependencies)
    
    return mod_toml, mod_nrm

# Loading TOML Data:
mod_common_data = toml.loads(root_dir.joinpath("mod_common.toml").read_text())
tests_common_data = toml.loads(root_dir.joinpath("tests_common.toml").read_text())

mm_mod_toml, mm_mod_nrm = add_toml_and_nrm_job("mm", "mod", project_name, mod_build_dir, [mod_common_data, mm_toml_data, mod_toml_data], [archive_extractions["llvmmips"], makefiles['mod']])
bk_mod_toml, bk_mod_nrm = add_toml_and_nrm_job("bk", "mod", project_name, mod_build_dir, [mod_common_data, bk_toml_data, mod_toml_data], [archive_extractions["llvmmips"], makefiles['mod']])
sf64_mod_toml, sf64_mod_nrm = add_toml_and_nrm_job("sf64", "mod", project_name, mod_build_dir, [mod_common_data, sf64_toml_data, mod_toml_data], [archive_extractions["llvmmips"], makefiles['mod']])
mm_tests_toml, mm_tests_nrm = add_toml_and_nrm_job("mm", "tests", project_tests_name, tests_build_dir, [tests_common_data, mm_toml_data, tests_toml_data], [archive_extractions["llvmmips"], makefiles['tests']])
bk_tests_toml, bk_tests_nrm = add_toml_and_nrm_job("bk", "tests", project_tests_name, tests_build_dir, [tests_common_data, bk_toml_data, tests_toml_data], [archive_extractions["llvmmips"], makefiles['tests']])
sf64_tests_toml, sf64_tests_nrm = add_toml_and_nrm_job("sf64", "tests", project_tests_name, tests_build_dir, [tests_common_data, sf64_toml_data, tests_toml_data], [archive_extractions["llvmmips"], makefiles['tests']])

# Extlib Compilation
extlib_name = "RecompPythonNative"
extlib = CMakeProjectConfig(
    root_dir,
    {
        # Unlike with clangmips, we're gonna prepend the LLVM and ZIG directories to the PATH that CMake recieves.
        "PATH": prepend_to_env_path([llvm_path.joinpath("bin"), zig_dir_path]),
        "LIB_NAME": extlib_name
    }
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
            output_paths[Path(file.name)] = file
        elif file.suffix == ".pyd":
            output_paths[Path(file.with_suffix(".dll").name)] = file
            
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
        win_base = {
            Path("python313.dll"): get_preset_lib_path(preset_name).joinpath("python313.dll"),
            Path(f"{extlib_name}.dll"): get_preset_lib_path(preset_name).joinpath(f"{extlib_name}.dll")
        }
        if build_type == "Debug" or build_type == "RelWithDebInfo":
            win_base[Path(f"{extlib_name}.pdb")] = get_preset_lib_path(preset_name).joinpath(f"{extlib_name}.pdb")
        return with_windows_dlls(preset_name, win_base)
    if platform.system() == "Darwin":
        return {
            Path("libpython3.13.dylib"): get_preset_lib_path(preset_name).joinpath("libpython3.13.dylib"),
            Path(f"{extlib_name}.dylib"): get_preset_lib_path(preset_name).joinpath(f"{extlib_name}.dylib")
        }
    if platform.system() == "Linux":
        return {
            Path("libpython3.13.so"): get_preset_lib_path(preset_name).joinpath("libpython3.13.so"),
            Path(f"{extlib_name}.so"): get_preset_lib_path(preset_name).joinpath(f"lib{extlib_name}.so")
        }
    
cmake_default_build_group_name: str = "Debug"
cmake_build_groups = {
    "Debug" : {
        "Windows": CMakeBuildJob.from_preset_pair(extlib, with_windows_dlls("zig-windows-x64-Debug", {
                Path("python313.dll"): get_preset_lib_path("zig-windows-x64-Debug").joinpath("python313.dll"),
                Path(f"{extlib_name}.dll"): get_preset_lib_path("zig-windows-x64-Debug").joinpath(f"lib{extlib_name}.dll"),
                Path(f"{extlib_name}.pdb"): get_preset_lib_path("zig-windows-x64-Debug").joinpath(f"lib{extlib_name}.pdb")
            }), "zig-windows-x64-Debug"),
        "Darwin": CMakeBuildJob.from_preset_pair(extlib, {
                Path("libpython3.13.dylib"): get_preset_lib_path("zig-macos-aarch64-Debug").joinpath("libpython3.13.dylib"),
                Path(f"{extlib_name}.dylib"): get_preset_lib_path("zig-macos-aarch64-Debug").joinpath(f"lib{extlib_name}.dylib")
            }, "zig-macos-aarch64-Debug"),
        "Linux": CMakeBuildJob.from_preset_pair(extlib, {
                Path("libpython3.13.so"): get_preset_lib_path("zig-linux-x64-Debug").joinpath("libpython3.13.so"),
                Path(f"{extlib_name}.so"): get_preset_lib_path("zig-linux-x64-Debug").joinpath(f"lib{extlib_name}.so")
            }, "zig-linux-x64-Debug"),
    },
    "Release" : {
        "Windows": CMakeBuildJob.from_preset_pair(extlib, with_windows_dlls("zig-windows-x64-Release", {
                 Path("python313.dll"): get_preset_lib_path("zig-windows-x64-Release").joinpath("python313.dll"),
                Path(f"{extlib_name}.dll"): get_preset_lib_path("zig-windows-x64-Release").joinpath(f"lib{extlib_name}.dll")
            }), "zig-windows-x64-Release"),
        "Darwin": CMakeBuildJob.from_preset_pair(extlib, {
                Path("libpython3.13.dylib"): get_preset_lib_path("zig-macos-aarch64-Release").joinpath("libpython3.13.dylib"),
                Path(f"{extlib_name}.dylib"): get_preset_lib_path("zig-macos-aarch64-Release").joinpath(f"lib{extlib_name}.dylib")
            }, "zig-macos-aarch64-Release"),
        "Linux": CMakeBuildJob.from_preset_pair(extlib, {
                Path("libpython3.13.so"): get_preset_lib_path("zig-linux-x64-Release").joinpath("libpython3.13.so"),
                Path(f"{extlib_name}.so"): get_preset_lib_path("zig-linux-x64-Release").joinpath(f"lib{extlib_name}.so")
            }, "zig-linux-x64-Release"),
    }, 
    "RelWithDebInfo": {
        "Windows": CMakeBuildJob.from_preset_pair(extlib, with_windows_dlls("zig-windows-x64-RelWithDebInfo", {
                Path("python313.dll"): get_preset_lib_path("zig-windows-x64-RelWithDebInfo").joinpath("python313.dll"),
                Path(f"{extlib_name}.dll"): get_preset_lib_path("zig-windows-x64-RelWithDebInfo").joinpath(f"lib{extlib_name}.dll"),
                Path(f"{extlib_name}.pdb"): get_preset_lib_path("zig-windows-x64-RelWithDebInfo").joinpath(f"lib{extlib_name}.pdb")
            }), "zig-windows-x64-RelWithDebInfo"),
        "Darwin": CMakeBuildJob.from_preset_pair(extlib, {
                Path("libpython3.13.dylib"): get_preset_lib_path("zig-macos-aarch64-RelWithDebInfo").joinpath("libpython3.13.dylib"),
                Path(f"{extlib_name}.dylib"): get_preset_lib_path("zig-macos-aarch64-RelWithDebInfo").joinpath(f"lib{extlib_name}.dylib")
            }, "zig-macos-aarch64-RelWithDebInfo"),
        "Linux": CMakeBuildJob.from_preset_pair(extlib, {
                Path("libpython3.13.so"): get_preset_lib_path("zig-linux-x64-RelWithDebInfo").joinpath("libpython3.13.so"),
                Path(f"{extlib_name}.so"): get_preset_lib_path("zig-linux-x64-RelWithDebInfo").joinpath(f"lib{extlib_name}.so")
            }, "zig-linux-x64-RelWithDebInfo"),
    },
    "MinSizeRel": {
        "Windows": CMakeBuildJob.from_preset_pair(extlib, with_windows_dlls("zig-windows-x64-MinSizeRel", {
                Path("python313.dll"): get_preset_lib_path("zig-windows-x64-MinSizeRel").joinpath("python313.dll"),
                Path(f"{extlib_name}.dll"): get_preset_lib_path("zig-windows-x64-MinSizeRel").joinpath(f"lib{extlib_name}.dll")
            }), "zig-windows-x64-MinSizeRel"),
        "Darwin": CMakeBuildJob.from_preset_pair(extlib, {
                Path("libpython3.13.dylib"): get_preset_lib_path("zig-macos-aarch64-MinSizeRel").joinpath("libpython3.13.dylib"),
                Path(f"{extlib_name}.dylib"): get_preset_lib_path("zig-macos-aarch64-MinSizeRel").joinpath(f"lib{extlib_name}.dylib")
            }, "zig-macos-aarch64-MinSizeRel"),
        "Linux": CMakeBuildJob.from_preset_pair(extlib, {
                Path("libpython3.13.so"): get_preset_lib_path("zig-linux-x64-MinSizeRel").joinpath("libpython3.13.so"),
                Path(f"{extlib_name}.so"): get_preset_lib_path("zig-linux-x64-MinSizeRel").joinpath(f"lib{extlib_name}.so")
            }, "zig-linux-x64-MinSizeRel"),
    },
    "native-Debug" : {
        "Native": CMakeBuildJob.from_preset_pair(extlib, native_output_files("Debug"), native_preset_name("Debug")),
    },
    "native-Release" : {
        "Native": CMakeBuildJob.from_preset_pair(extlib, native_output_files("Release"), native_preset_name("Release")),
    }, 
    "native-RelWithDebInfo": {
        "Native": CMakeBuildJob.from_preset_pair(extlib, native_output_files("RelWithDebInfo"), native_preset_name("RelWithDebInfo")),
    },
    "native-MinSizeRel": {
        "Native": CMakeBuildJob.from_preset_pair(extlib, native_output_files("MinSizeRel"), native_preset_name("MinSizeRel")),
    }
}

for group_key, group in cmake_build_groups.items():
    for build_key, build in group.items():
        if not group_key.startswith("native-"):
            build.depends_on([archive_extractions["zig"]])
        build.depends_on([archive_extractions["llvm"]])


build_outputs["zelda_debug"] = zelda_debug_test_dir = BuildOutputJob(root_dir.joinpath("test_env/zelda/mods"))
zelda_debug_test_dir.depends_on([
    mm_mod_nrm,
    mm_tests_nrm
] + [i for i in cmake_build_groups["Debug"].values()])

build_outputs["bk_debug"] = zelda_debug_test_dir = BuildOutputJob(root_dir.joinpath("test_env/bk/mods"))
zelda_debug_test_dir.depends_on([
    bk_mod_nrm,
    bk_tests_nrm
] + [i for i in cmake_build_groups["Debug"].values()])

build_outputs["sf64_debug"] = sf64_debug_test_dir = BuildOutputJob(root_dir.joinpath("test_env/sf64/mods"))
sf64_debug_test_dir.depends_on([
    sf64_mod_nrm,
    sf64_tests_nrm
] + [i for i in cmake_build_groups["Debug"].values()])

def package_url_from_git() -> str:
    result = subprocess.run(
        [
            shutil.which("git"),
            "config", 
            "--get", 
            "remote.origin.url"
        ],
        cwd=root_dir,
        capture_output=True,
        text=True
    )

    if result.returncode == 0:
        return result.stdout.strip()
    else:
        return None

main_package = ThunderstorePackageJob(
    root_dir.joinpath(f"{project_name}.thunderstore.zip"),
    {
        "name": project_name,
        "version_number": project_version_string,
        "website_url": package_url_from_git(),
        "description": "A resource for modders. Enables use of Python code and the Python Standard Library within mods, enabling many behaviors that would otherwise require an external library to be compiled.",
        "dependencies": []
    },
    root_dir.joinpath("thunderstore_info/README.md").read_text(),
    root_dir.joinpath("thunderstore_info/CHANGELOG.md").read_text(),
    root_dir.joinpath("thumb.png")
)

main_package.depends_on([
    mm_mod_nrm,
    bk_mod_nrm,
    sf64_mod_nrm
] + [i for i in cmake_build_groups["Release"].values()])
thunderstore_packages['package'] = main_package

clean_paths: list[Path] = [
    build_dir
]

distclean_paths: list[Path] = [
    binaries_dir,
    archive_downloads_dir,
]