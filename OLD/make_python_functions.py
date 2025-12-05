import pathlib, subprocess, os, shutil, tomllib, zipfile, sys, json, platform, urllib.request, urllib.parse
from pathlib import Path

default_compiler_artifacts = {
    "mod": {
        "Windows": "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/release-20.1.8/Clang_version_20.1.8-MipsOnly-Windows-AMD64-N64RecompEssentials.zip",
        "Darwin": "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/release-20.1.8/Clang_version_20.1.8-MipsOnly-Darwin-arm64-N64RecompEssentials.tar.xz",
        "Linux": "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/release-20.1.8/Clang_version_20.1.8-MipsOnly-Linux-x86_64-N64RecompEssentials.tar.xz" 
    },
    "extlib": {
        "Windows": "https://ziglang.org/download/0.14.1/zig-x86_64-windows-0.14.1.zip",
        "Darwin": "https://ziglang.org/download/0.14.1/zig-aarch64-macos-0.14.1.tar.xz",
        "Linux": "https://ziglang.org/download/0.14.1/zig-x86_64-linux-0.14.1.tar.xz"
    }
}

class ModInfo:
    project_root: Path
    mod_toml_file: Path
    mod_data: dict
    
    tests_toml_file: Path
    tests_data: dict
    
    tests_info_set: bool = False
    extlib_info_set: bool = False
    
    enable_tool_downloads: bool
    
    compilation_dir_root: Path
    compilation_artifacts_path: Path
    compilation_mod_extract: Path
    compilation_extlib_extract: Path
    
    def __init__(self, mod_toml_str: str, mod_build_dir: str):
        self.project_root = Path(__file__).parent
        
        self.mod_toml_file = self.project_root.joinpath(mod_toml_str)
        self.mod_data = tomllib.loads(self.mod_toml_file.read_text())

        self.mod_build_dir = self.project_root.joinpath(mod_build_dir)
        self.build_mod_nrm_file = self.mod_build_dir.joinpath(f"{self.mod_data['inputs']['mod_filename']}.nrm")
        
        self.runtime_dir = self.project_root.joinpath("runtime")
        self.runtime_mods_dir = self.runtime_dir.joinpath("mods")
        self.runtime_mod_nrm_file = self.runtime_mods_dir.joinpath(f"{self.mod_data['inputs']['mod_filename']}.nrm")
        
        self.assets_archive_path =self.project_root.joinpath("assets_archive.zip")
        
        self.compilation_dir_root = self.project_root.joinpath("compilation")
        self.compilation_artifacts_path = self.compilation_dir_root.joinpath("artifacts")
        self.mod_compilation_artifact_path = self.compilation_dir_root.joinpath("artifacts")
        self.compilation_mod_extract = self.compilation_dir_root.joinpath("mod")
        self.compilation_extlib_extract = self.compilation_dir_root.joinpath("extlib")
        
        # Handle recomp compilers:
        self.user_config_path = None
        self.user_config = {}
        # if not self.user_config_path.exists():
        #     self.create_user_build_config()
        
        # else:
        #     self.user_config = json.loads(self.user_config_path.read_text())

        # if self.enable_tool_downloads:
        #     if self.user_config["mod_compiling"]["compiler_download"]["download_compiler"] and not self.compilation_mod_extract.exists():
        #         self.download_and_setup_mips_compiler()
        
        # Tests Info
        self.tests_toml_file: Path = None
        self.tests_data: dict = None
        self.tests_build_dir: Path = None
        self.build_tests_nrm_file: Path = None
        self.runtime_tests_nrm_file: Path = None

        # Extlib Info
        self.build_dll_file: Path = None
        self.build_pdb_file: Path = None
        self.build_dylib_file: Path = None
        self.build_so_file: Path = None
        self.build_native_file: Path = None
        self.build_native_pdb_file: Path = None
        
        self.runtime_dll_file: Path = None
        self.runtime_pdb_file: Path = None
        self.runtime_dylib_file: Path = None
        self.runtime_so_file: Path = None
        self.runtime_native_file: Path = None
        self.runtime_native_pdb_file: Path = None
        
        # Python Library Info:
        self.build_python_dll_file: Path = None
        self.build_python_dylib_file: Path = None
        self.build_python_so_file: Path = None
        self.build_python_native_file: Path = None
        
        self.runtime_python_dll_file: Path = None
        self.runtime_python_dylib_file: Path = None
        self.runtime_python_so_file: Path = None
        self.runtime_python_native_file: Path = None
    
    def set_tests_info(self, test_toml_str: str, tests_build_dir: str):
        self.tests_info_set = True
        
        self.tests_toml_file = self.project_root.joinpath(test_toml_str)
        self.tests_data = tomllib.loads(self.tests_toml_file.read_text())

        self.tests_build_dir = self.project_root.joinpath(tests_build_dir)
        self.build_tests_nrm_file = self.tests_build_dir.joinpath(f"{self.tests_data['inputs']['mod_filename']}.nrm")
        
        self.runtime_tests_nrm_file = self.runtime_mods_dir.joinpath(f"{self.tests_data['inputs']['mod_filename']}.nrm")
    
        return self
    
    def set_extlib_info(self, windows_lib: str, macos_lib: str, linux_lib: str, native_lib: str):
        self.extlib_info_set = True
        
        self.build_dll_file = self.project_root.joinpath(windows_lib)
        self.build_pdb_file = self.build_dll_file.with_suffix(".pdb")
        self.build_dylib_file = self.project_root.joinpath(macos_lib)
        self.build_so_file = self.project_root.joinpath(linux_lib)
        self.build_native_file = self.project_root.joinpath(native_lib)
        self.build_native_pdb_file = self.build_native_file.with_suffix(".pdb")
        
        self.runtime_dll_file = self.runtime_mods_dir.joinpath(self.build_dll_file.name.removeprefix("lib"))
        self.runtime_pdb_file = self.runtime_mods_dir.joinpath(self.build_pdb_file.name.removeprefix("lib"))
        self.runtime_dylib_file = self.runtime_mods_dir.joinpath(self.build_dylib_file.name.removeprefix("lib"))
        self.runtime_so_file = self.runtime_mods_dir.joinpath(self.build_so_file.name.removeprefix("lib"))
        self.runtime_native_file = self.runtime_mods_dir.joinpath(self.build_native_file.name.removeprefix("lib"))
        self.runtime_native_pdb_file = self.runtime_mods_dir.joinpath(self.build_native_pdb_file.name.removeprefix("lib"))
        
        self.build_python_dll_file: Path = self.build_dll_file.with_stem("python313")
        self.build_python_dylib_file: Path = self.build_dylib_file.with_stem("libpython3.13")
        self.build_python_so_file: Path = self.build_so_file.with_stem("libpython3.13")
        self.build_python_native_file: Path = self.build_native_file.with_stem("python313" if os.name == 'nt' else "libpython3.13")
        
        self.runtime_python_dll_file: Path = self.runtime_mods_dir.joinpath(self.build_python_dll_file.name)
        self.runtime_python_dylib_file: Path = self.runtime_mods_dir.joinpath(self.build_python_dylib_file.name)
        self.runtime_python_so_file: Path = self.runtime_mods_dir.joinpath(self.build_python_so_file.name)
        self.runtime_python_native_file: Path = self.runtime_mods_dir.joinpath(self.build_python_native_file.name)
        
        # if self.enable_tool_downloads:
        #     if self.user_config["extlib_compiling"]["compiler_download"]["download_compiler"] and not self.compilation_mod_extract.exists():
        #         self.download_and_setup_mips_compiler()
        
        return self
    
    def load_user_config(self, user_config_path: str):
        # self.user_config_path = self.project_root.joinpath("./user_build_config.json")
        self.user_config_path = self.project_root.joinpath(user_config_path)
        self.user_config = json.loads(self.user_config_path.read_text())
        
        return self
    
    def create_user_build_config(self, user_config_path: str):
        self.user_config_path = self.project_root.joinpath(user_config_path)
        self.user_config = {
            "mod_compiling": {
                "compiler_download": {
                    "download_compiler": True,
                    "download_compiler_artifact": self.get_mips_compiler_artifact_url(),
                },
                "compiler": "clang" if self.get_mips_compiler_artifact_url() is None else str(self.project_root.joinpath("./compilation/mod/bin_essentials/clang")).replace("\\", "/"),
                "linker": "ld.lld" if self.get_mips_compiler_artifact_url() is None else str(self.project_root.joinpath("./compilation/mod/bin_essentials/ld.lld")).replace("\\", "/")
            },
        }
        
        if 'extlib_compiling' in self.mod_data:
            self.user_config["extlib_compiling"] = {
                "compiler_download": {
                    "download_compiler": True,
                    "download_compiler_artifact": self.get_extlib_compiler_artifact_url(),
                },
                "compiler": "clang" if self.get_extlib_compiler_artifact_url() is None \
                    else str(self.project_root.joinpath(f"./compilation/extlib/{self.get_filename_from_url(self.get_extlib_compiler_artifact_url()).stem}/zig")).replace("\\", "/"),
                "preset_groups": {
                    "Debug": {
                        "windows": {
                            "configure": self.mod_data['extlib_compiling']['windows_debug_configure_preset'],
                            "build": self.mod_data['extlib_compiling']['windows_debug_build_preset']
                        },
                        "macos": {
                            "configure": self.mod_data['extlib_compiling']['macos_debug_configure_preset'],
                            "build": self.mod_data['extlib_compiling']['macos_debug_build_preset']
                        },
                        "linux": {
                            "configure": self.mod_data['extlib_compiling']['linux_debug_configure_preset'],
                            "build": self.mod_data['extlib_compiling']['linux_debug_build_preset']
                        },
                        "native": {
                            "configure": self.get_native_preset("Debug"),
                            "build": self.get_native_preset("Debug")
                        }
                    },
                    "Release": {
                        "windows": {
                            "configure": self.mod_data['extlib_compiling']['windows_release_configure_preset'],
                            "build": self.mod_data['extlib_compiling']['windows_release_build_preset']
                        },
                        "macos": {
                            "configure": self.mod_data['extlib_compiling']['macos_release_configure_preset'],
                            "build": self.mod_data['extlib_compiling']['macos_release_build_preset']
                        },
                        "linux": {
                            "configure": self.mod_data['extlib_compiling']['linux_release_configure_preset'],
                            "build": self.mod_data['extlib_compiling']['linux_release_build_preset']
                        },
                        "native": {
                            "configure": self.get_native_preset("Release"),
                            "build": self.get_native_preset("Release")
                        }
                    }
                }
            }
            
        self.user_config_path.write_text(json.dumps(self.user_config, indent=4))
    
    def get_native_preset(self, build_type: str):
        if platform.system() == "Windows":
            return f"native-windows-x64-{build_type}"
        elif platform.system() == "Darwin":
            return f"native-macos-aarch64-{build_type}"
        else:
            return f"native-linux-x64-{build_type}"
    
    def get_mips_compiler_artifact_url(self):
        if platform.system() in default_compiler_artifacts["mod"]:
            return default_compiler_artifacts["mod"][platform.system()]
        else:
            return None

    def download_and_setup_mips_compiler(self):
        artifact_url = self.user_config["mod_compiling"]["compiler_download"]["download_compiler_artifact"]
        filename = self.get_filename_from_url(artifact_url)
        dst_path = self.compilation_artifacts_path.joinpath(filename)
        
        if not self.compilation_artifacts_path.exists():
            os.makedirs(self.compilation_artifacts_path)
            
        try:
            urllib.request.urlretrieve(
                artifact_url,
                dst_path
            )
            print(f"File downloaded successfully to {dst_path}")
        except Exception as e:
            print(f"Error downloading file: {e}")
            return
        
        shutil.unpack_archive(dst_path, self.compilation_mod_extract)
        
    def get_extlib_compiler_artifact_url(self):
        if platform.system() in default_compiler_artifacts["extlib"]:
            return default_compiler_artifacts["extlib"][platform.system()]
        else:
            return None
    
    def download_and_setup_extlib_compiler(self):
        artifact_url = self.user_config["extlib_compiling"]["compiler_download"]["download_compiler_artifact"]
        filename = self.get_filename_from_url(artifact_url)
        dst_path = self.compilation_artifacts_path.joinpath(filename)
        
        if not self.compilation_artifacts_path.exists():
            os.makedirs(self.compilation_artifacts_path)
            
        try:
            urllib.request.urlretrieve(
                artifact_url,
                dst_path
            )
            print(f"File downloaded successfully to {dst_path}")
        except Exception as e:
            print(f"Error downloading file: {e}")
            return
        
        shutil.unpack_archive(dst_path, self.compilation_extlib_extract)
    
    def download_compilers(self):
        self.download_and_setup_mips_compiler()
        if 'extlib_compiling' in self.mod_data:
            self.download_and_setup_extlib_compiler()
    
    def get_filename_from_url(self, url: Path) -> Path:
        parsed_url = urllib.parse.urlparse(url)
        return Path(os.path.basename(parsed_url.path))
    
    def get_mod_file(self):
        name = f"{self.mod_data['inputs']['mod_filename']}.nrm"
        return self.print_and_return(self.mod_build_dir.joinpath(name))
    
    def get_mod_elf(self):
        return self.print_and_return(self.mod_toml_file.parent.joinpath(self.mod_data['inputs']['elf_path']))
    
    def get_tests_file(self):
        name = f"{self.tests_data['inputs']['mod_filename']}.nrm"
        return self.print_and_return(self.tests_build_dir.joinpath(name))
    
    def get_tests_elf(self):
        return self.print_and_return(self.tests_toml_file.parent.joinpath(self.tests_data['inputs']['elf_path']))
        
    def get_mod_compiler(self):
        return self.print_and_return(self.user_config["mod_compiling"]["compiler"])
        
    def get_mod_linker(self):
        return self.print_and_return(self.user_config["mod_compiling"]["linker"])
    
    def get_zig_cmd(self):
        return self.print_and_return(self.user_config["extlib_compiling"]["compiler"])
    
    def get_extlib_name(self):
        if 'extlib_compiling' in self.mod_data:
            return self.print_and_return(self.mod_data['extlib_compiling']['library_name'])
        else:
            return self.print_and_return(None)

    def get_extlib_windows_configure_preset(self, build_type: str):
        if 'extlib_compiling' not in self.mod_data: 
            return None
        return self.print_and_return(self.user_config["extlib_compiling"]["preset_groups"][build_type]["windows"]["configure"])

    def get_extlib_macos_configure_preset(self, build_type: str):
        if 'extlib_compiling' not in self.mod_data: 
            return None
        return self.print_and_return(self.user_config["extlib_compiling"]["preset_groups"][build_type]["macos"]["configure"])

    def get_extlib_linux_configure_preset(self, build_type: str):
        if 'extlib_compiling' not in self.mod_data: 
            return None
        return self.print_and_return(self.user_config["extlib_compiling"]["preset_groups"][build_type]["linux"]["configure"])

    def get_extlib_native_configure_preset(self, build_type: str):
        if 'extlib_compiling' not in self.mod_data: 
            return None
        return self.print_and_return(self.user_config["extlib_compiling"]["preset_groups"][build_type]["native"]["configure"])

    def get_extlib_windows_build_preset(self, build_type: str):
        if 'extlib_compiling' not in self.mod_data: 
            return None
        return self.print_and_return(self.user_config["extlib_compiling"]["preset_groups"][build_type]["windows"]["build"])

    def get_extlib_macos_build_preset(self, build_type: str):
        if 'extlib_compiling' not in self.mod_data: 
            return None
        return self.print_and_return(self.user_config["extlib_compiling"]["preset_groups"][build_type]["macos"]["build"])

    def get_extlib_linux_build_preset(self, build_type: str):
        if 'extlib_compiling' not in self.mod_data: 
            return None
        return self.print_and_return(self.user_config["extlib_compiling"]["preset_groups"][build_type]["linux"]["build"])

    def get_extlib_native_build_preset(self, build_type: str):
        if 'extlib_compiling' not in self.mod_data: 
            return None
        return self.print_and_return(self.user_config["extlib_compiling"]["preset_groups"][build_type]["native"]["build"])

    def create_asset_archive(self, assets_extract_path_str: str):
            assets_extract_path = self.project_root.joinpath(assets_extract_path_str)
            print(f"Assets folder '{assets_extract_path.name}' not found. Extracting assets from '{self.assets_archive_path.name}'...")
            with zipfile.ZipFile(self.assets_archive_path, 'r') as zip_ref:
                zip_ref.extractall(assets_extract_path)

    def copy_to_runtime_dir(self):
        # Copying files for debugging:
        os.makedirs(self.runtime_mods_dir, exist_ok=True)
        portable_txt = self.runtime_dir.joinpath("portable.txt")
        if not portable_txt.exists():
            portable_txt.write_text("")
            print(f"Created '{portable_txt}'.")
        
        self.copy_if_exists(self.build_mod_nrm_file, self.runtime_mod_nrm_file)
        
        if self.tests_info_set:
            self.copy_if_exists(self.build_tests_nrm_file, self.runtime_tests_nrm_file)
        
        # If no extlib is being built, we don't need to try to find these.
        if self.extlib_info_set:
            self.copy_if_exists(self.build_dll_file, self.runtime_dll_file)
            self.copy_if_exists(self.build_pdb_file, self.runtime_pdb_file)
            self.copy_if_exists(self.build_dylib_file, self.runtime_dylib_file)
            self.copy_if_exists(self.build_so_file, self.runtime_so_file)
            
            self.copy_if_exists(self.build_python_dll_file, self.runtime_python_dll_file)
            self.copy_if_exists(self.build_python_dylib_file, self.runtime_python_dylib_file)
            self.copy_if_exists(self.build_python_so_file, self.runtime_python_so_file)

    def copy_to_runtime_dir_native(self):
        # Copying files for debugging:
        os.makedirs(self.runtime_mods_dir, exist_ok=True)
        portable_txt = self.runtime_dir.joinpath("portable.txt")
        if not portable_txt.exists():
            portable_txt.write_text("")
            print(f"Created '{portable_txt}'.")
        
        self.copy_if_exists(self.build_mod_nrm_file, self.runtime_mod_nrm_file)
        
        if self.tests_info_set:
            self.copy_if_exists(self.build_tests_nrm_file, self.runtime_tests_nrm_file)
        
        # If no extlib is being built, we don't need to try to find these.
        if self.extlib_info_set:
            self.copy_if_exists(self.build_native_file, self.runtime_native_file)
            self.copy_if_exists(self.build_native_pdb_file, self.runtime_native_pdb_file)
            self.copy_if_exists(self.build_python_native_file, self.runtime_python_native_file)

    def copy_if_exists(self, src: Path, dest: Path):
        if src.exists():
            shutil.copy(src, dest)
            print(f"'{src}' copied to '{dest}'.")
        else:
            print(f"'{src}' does not exist. Skipping.")

    def run_clean(self):
        shutil.rmtree(self.mod_build_dir)
        shutil.rmtree(self.project_root.joinpath("./N64Recomp/build"))

    def print_and_return(self, x):
        print(x)
        return x

def run_build(args: list[str]):
    make_run = subprocess.run(
        [
            shutil.which("make"),
        ] + args,
        cwd=pathlib.Path(__file__).parent
        
    )
    if make_run.returncode != 0:
        raise RuntimeError("Make failed!")

if __name__ == '__main__':
    run_build(sys.argv[1:])
