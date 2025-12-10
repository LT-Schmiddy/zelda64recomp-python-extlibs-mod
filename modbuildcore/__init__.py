import enum
from pathlib import Path

from . import downloads
from . import makefiles
from . import tomls
from . import utils

class TomlMakeSpecialVals(enum.Enum):
        TOML_ELF_PATH = 0
        TOML_ELF_PARENT_PATH = 1
        TOML_BUILD_DIR = 2

class ModProjectConfig:
    archive_artifacts_dir: Path
    mips_compiler_path: Path
    mips_linker_path: Path
    mod_tool_path: Path
    make_path: Path
    
    archive_downloads: list[downloads.DownloadArchiveConfig]
    makefiles: list[makefiles.MakefileConfig]
    mod_tomls: list[tomls.ModTomlConfig]
    
    clean_paths: list[Path]
    distclean_paths: list[Path]
    
    def __init__(self):
        self.archive_artifacts_dir = None
        self.mips_compiler_path = None
        self.mips_linker_path = None
        self.mod_tool_path = None
        self.make_path = None
        
        self.archive_downloads = []
        self.makefiles = []
        self.mod_tomls = []
        
        self.clean_paths = []
        self.distclean_paths = []

    def set_archive_artifacts_dir(self, archive_artifacts_dir: Path):
        self.archive_artifacts_dir = utils.path_check_coerse(archive_artifacts_dir, "archive_artifacts_dir")
    
    def set_mips_compiler(self, mips_compiler_path: Path):
        self.mips_compiler_path = utils.path_check_coerse(mips_compiler_path, "mips_compiler_path")

    def set_mips_linker(self, mips_linker_path: Path):
        self.mips_linker_path = utils.path_check_coerse(mips_linker_path, "mips_linker_path")
        
    def set_mod_tool(self, mod_tool_path: Path):
        self.mod_tool_path = utils.path_check_coerse(mod_tool_path, "mod_tool_path")
        
    def set_make(self, make_path: Path):
        self.make_path = utils.path_check_coerse(make_path, "make_path")

    def add_archive_download(self, url: str, extract_dir: Path):
        extract_dir = utils.path_check_coerse(extract_dir, "extract_dir")
        self.archive_downloads.append(downloads.DownloadArchiveConfig(url, extract_dir))
        
    def add_mod_toml(self, toml_path: Path, makefile_path: Path, makefile_extended_env: dict[str, str | TomlMakeSpecialVals], toml_build_dir: Path = None):
        toml_path = utils.path_check_coerse(toml_path, "toml_path")
        makefile_path = utils.path_check_coerse(makefile_path, "makefile_path")
        
        new_toml = tomls.ModTomlConfig(toml_path, toml_build_dir)
        new_makefile = makefiles.MakefileConfig(makefile_path, self._resolve_extended_env(new_toml, makefile_extended_env))
        
        self.makefiles.append(new_makefile)
        self.mod_tomls.append(new_toml)
    
    def mark_path_for_clean(self, path: Path):
        self.clean_paths.append(utils.path_check_coerse(path, "path"))
    
    def mark_path_for_distclean(self, path: Path):
        self.distclean_paths.append(utils.path_check_coerse(path, "path"))
        
    def mark_paths_for_clean(self, paths: list[Path]):
        self.clean_paths.extend([utils.path_check_coerse(i, "entries in paths") for i in paths])
    
    def mark_paths_for_distclean(self, paths: list[Path]):
        self.distclean_paths.append([utils.path_check_coerse(i, "entries in paths") for i in paths])
    
    def _resolve_extended_env(self, toml_config: tomls.ModTomlConfig, arg_env: dict[str, str | TomlMakeSpecialVals]):
        retVal =  arg_env.copy()
        
        # Environmental variables can only be strings. Resolving all non-string entries.
        env_keys = retVal.keys()
        for key in env_keys:
            if retVal[key] == TomlMakeSpecialVals.TOML_ELF_PATH:
                retVal[key] = str(toml_config.get_elf_path())
                
            if retVal[key] == TomlMakeSpecialVals.TOML_ELF_PARENT_PATH:
                retVal[key] = str(toml_config.get_elf_path().parent)
                
            if retVal[key] == TomlMakeSpecialVals.TOML_BUILD_DIR:
                retVal[key] = str(toml_config.build_dir)
            
            # Paths need special attention for Make compatability on Windows.
            if isinstance(retVal[key], Path):
                retVal[key] = str(retVal[key]).replace("\\", "/")
            
            elif not isinstance(retVal[key], str):
                retVal[key] = str(retVal[key])
                
        return retVal
    
    
__all__ = [
    'downloads',
    'makefiles',
    'tomls',
    'utils',
    'ModProjectConfig'
]


