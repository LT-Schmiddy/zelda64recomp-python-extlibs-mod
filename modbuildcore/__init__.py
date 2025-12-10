import enum, shutil
from pathlib import Path

from . import archives
from . import cmake
from . import downloads
from . import makefiles
from . import tomls
from . import utils

class TomlMakeSpecialVals(enum.Enum):
        TOML_ELF_PATH = 0
        TOML_ELF_PARENT_PATH = 1
        TOML_BUILD_DIR = 2

class ModProjectConfig:
    root_dir: Path
    
    archive_downloads_dir: Path
    mips_compiler_path: Path
    mips_linker_path: Path
    mod_tool_path: Path
    make_path: Path
    cmake_path: Path
    
    archive_extractions: list[archives.ArchiveExtractConfig]
    downloads: list[downloads.DownloadConfig]
    makefiles: list[makefiles.MakefileConfig]
    mod_tomls: list[tomls.ModTomlConfig]
    cmake_projects: list[cmake.CMakeProjectConfig]
    
    extended_clean_paths: list[Path]
    extended_distclean_paths: list[Path]
    
    def __init__(self, root_dir: Path):
        self.root_dir = self.path_check_coerse(root_dir, "root_dir")
        
        self.archive_downloads_dir = self.root_dir.joinpath("downloads")
        
        # Finding Default Tools:
        self.mod_tool_path = shutil.which("RecompModTool")
        self.make_path = shutil.which("make")
        self.cmake_path = shutil.which("cmake")
        
        self.archive_extractions = []
        self.downloads = []
        self.makefiles = []
        self.mod_tomls = []
        self.cmake_projects = []
        
        self.extended_clean_paths = []
        self.extended_distclean_paths = []

    # File Management:
    def set_archive_downloads_dir(self, archive_artifacts_dir: Path):
        self.archive_downloads_dir = self.path_check_coerse(archive_artifacts_dir, "archive_artifacts_dir")
        
    def set_mod_tool(self, mod_tool_path: Path):
        self.mod_tool_path = self.path_check_coerse(mod_tool_path, "mod_tool_path")
        
    def set_make_path(self, make_path: Path):
        self.make_path = self.path_check_coerse(make_path, "make_path")

    def add_download(self, url: str, download_path: Path, append_url_filename: bool = True) -> downloads.DownloadConfig:
        extract_dir = self.path_check_coerse(extract_dir, "extract_dir")
        extract_dir = self.path_check_coerse(download_path, "download_path")
        
        retVal = downloads.DownloadConfig(url, download_path, append_url_filename)
        self.downloads.append(retVal)
        return retVal

    def add_archive_extraction(self, archive_path: Path, extract_dir: Path) -> archives.ArchiveExtractConfig:
        archive_path = self.path_check_coerse(archive_path, "archive_path")
        extract_dir = self.path_check_coerse(extract_dir, "extract_dir")
        
        retVal = archives.ArchiveExtractConfig(archive_path, extract_dir)
        self.archive_extractions.append(retVal)
        return retVal
    
    def add_archive_download_and_extract(self, url: str, extract_dir: Path) -> tuple[downloads.DownloadConfig, archives.ArchiveExtractConfig] :
        extract_dir = self.path_check_coerse(extract_dir, "extract_dir")
        new_download = downloads.DownloadConfig(url, self.archive_downloads_dir, True)
        new_extraction = archives.ArchiveExtractConfig(new_download.download_path, extract_dir)
        
        self.downloads.append(new_download)
        self.archive_extractions.append(new_extraction)
        
        return new_download, new_extraction
    
    # Makefile/NRM Building:
    def add_mod_toml(self, toml_path: Path, toml_build_dir: Path = None) -> tomls.ModTomlConfig:
        retVal = tomls.ModTomlConfig(toml_path, toml_build_dir)
        self.mod_tomls.append(retVal)
        return retVal
    
    def add_makefile(self, makefile_path: Path, extended_env: dict[str, str]) -> makefiles.MakefileConfig:
        retVal = makefiles.MakefileConfig(makefile_path, extended_env.copy())
        self.makefiles.append(retVal)
        return retVal
    
    def add_mod_toml_and_makefile(self, toml_path: Path, makefile_path: Path, makefile_extended_env: dict[str, str | TomlMakeSpecialVals],
                                    toml_build_dir: Path = None) -> tuple[tomls.ModTomlConfig, makefiles.MakefileConfig]:
        toml_path = self.path_check_coerse(toml_path, "toml_path")
        makefile_path = self.path_check_coerse(makefile_path, "makefile_path")
        
        new_toml = tomls.ModTomlConfig(toml_path, toml_build_dir)
        new_makefile = makefiles.MakefileConfig(makefile_path, self._resolve_toml_makefile_extended_env(new_toml, makefile_extended_env))
        
        self.makefiles.append(new_makefile)
        self.mod_tomls.append(new_toml)
        
        return new_toml, new_makefile
    
    # CMake:
    def add_cmake_project(self, project_dir: Path, extended_env: dict[str, str]) -> cmake.CMakeProjectConfig:
        retVal = cmake.CMakeProjectConfig(project_dir, self._resolve_cmake_extended_env(extended_env))
        self.cmake_projects.append(retVal)
        return retVal
    
    # Cleaning:
    def mark_path_for_clean(self, path: Path):
        self.extended_clean_paths.append(self.path_check_coerse(path, "path"))
    
    def mark_path_for_distclean(self, path: Path):
        self.extended_distclean_paths.append(self.path_check_coerse(path, "path"))
        
    def mark_paths_for_clean(self, paths: list[Path]):
        self.extended_clean_paths.extend([self.path_check_coerse(i, "entries in paths") for i in paths])
    
    def mark_paths_for_distclean(self, paths: list[Path]):
        self.extended_distclean_paths.extend([self.path_check_coerse(i, "entries in paths") for i in paths])
    
    def get_paths_for_cleaning(self) -> list[Path]:
        return self.extended_clean_paths[:]
    
    def get_paths_for_distcleaning(self) -> list[Path]:
        return [
            self.archive_downloads_dir
        ] + self.extended_distclean_paths[:]
    
    # Helper funtions: 
    def _resolve_cmake_extended_env(self, arg_env: dict[str, str | TomlMakeSpecialVals]):
        retVal =  arg_env.copy()
        
        # Environmental variables can only be strings. Resolving all non-string entries.
        env_keys = retVal.keys()
        for key in env_keys:
            # Paths need special attention for Make compatability on Windows.
            if isinstance(retVal[key], Path):
                p: Path = retVal[key]
                if not p.is_absolute():
                    p = self.root_dir.joinpath(p)
                retVal[key] = str(p).replace("\\", "/")
            
            elif not isinstance(retVal[key], str):
                retVal[key] = str(retVal[key])
                
        return retVal    

    def _resolve_toml_makefile_extended_env(self, toml_config: tomls.ModTomlConfig, arg_env: dict[str, str | TomlMakeSpecialVals]):
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
                p: Path = retVal[key]
                if not p.is_absolute():
                    p = self.root_dir.joinpath(p)
                retVal[key] = str(p).replace("\\", "/")
            
            elif not isinstance(retVal[key], str):
                retVal[key] = str(retVal[key])
                
        return retVal
    
    def path_check_coerse(self, p: Path | str, arg_name: str = "Argument") -> Path:
        retVal = None
        if isinstance(p, Path):
            retVal = p
        elif isinstance(p, str):
            retVal = Path(p)
        else:
            raise RuntimeError(f"{arg_name} should either be pathlib.Path or str")

        if not retVal.is_absolute():
            retVal = self.root_dir.joinpath(retVal)
            
        return retVal
    
__all__ = [
    'downloads',
    'makefiles',
    'tomls',
    'utils',
    'ModProjectConfig'
]


