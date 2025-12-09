from pathlib import Path

from . import downloads
from . import tomls
from . import utils

class ModProjectConfig:
    mips_compiler_path: Path
    mips_linker_path: Path
    mod_tool_path: Path
    make_path: Path
    
    archive_downloads: list[downloads.DownloadArchiveConfig]
    mod_tomls: list[tomls.ModTomlConfig]
    
    def __init__(self):
        self.mips_compiler_path = None
        self.mips_linker_path = None
        self.mod_tool_path = None
        self.make_path = None
        self.archive_downloads = []
        self.mod_tomls = []

    def set_mips_compiler(self, mips_compiler_path: Path):
        self.mips_compiler_path = utils.path_check_coerse(mips_compiler_path, "mips_compiler_path")

    def set_mips_linker(self, mips_linker_path: Path):
        self.mips_linker_path = utils.path_check_coerse(mips_linker_path, "mips_compiler_path")
        
    def set_mod_tool(self, mod_tool_path: Path):
        self.mod_tool_path = utils.path_check_coerse(mod_tool_path, "mod_tool_path")
        
    def set_make(self, make_path: Path):
        self.make_path = utils.path_check_coerse(make_path, "make_path")

    def add_archive_download(self, url: str, extract_dir: Path):
        extract_dir = utils.path_check_coerse(extract_dir, "extract_dir")
        self.archive_downloads.append(downloads.DownloadArchiveConfig(url, extract_dir))
        
    def add_mod_toml(self, toml_path: Path, makefile_path: Path, makefile_extended_env: dict[str, str | tomls.ModTomlConfig.MakeEnvSpecialVals]):
        if not isinstance(toml_path, Path):
            toml_path = Path(toml_path)
        
        if not isinstance(makefile_path, Path):
            makefile_path = Path(makefile_path)
        
        self.mod_tomls.append(tomls.ModTomlConfig(toml_path, makefile_path, makefile_extended_env))
        

__all__ = ['downloads', 'tomls', 'utils', 'ModProjectConfig']


