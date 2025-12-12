import tomllib, pathlib
from pathlib import Path

from invoke import Context

from .utils import invoke_subprocess_run

class ModTomlConfig:
    toml_path: Path
    build_dir: Path
    
    def __init__(self, toml_path: Path, build_dir: Path = None):
        self.toml_path = toml_path    
        self.build_dir = build_dir
        self.data = tomllib.loads(self.toml_path.read_text())
        
        if self.build_dir is None:
            self.build_dir = self.get_elf_path().parent
    
    def get_path_from_toml(self, rel_path: str | Path) -> Path:
        return self.toml_path.parent.joinpath(rel_path).resolve()
    
    def get_elf_path(self) -> Path:
        return self.get_path_from_toml(self.data["inputs"]["elf_path"])
    
    def get_output_path(self) -> Path:
        return self.build_dir.joinpath(self.data["inputs"]["mod_filename"]).with_suffix(".nrm")
    
class ModTomlHandler:
    config: ModTomlConfig
    data: dict
    
    def __init__(self, config: ModTomlConfig):
        self.config = config
        self.data = tomllib.loads(self.config.toml_path.read_text())
        
    def run_mod_tool(self, context: Context, mod_tool_binary: Path):
        invoke_subprocess_run(context, True,
            [mod_tool_binary, self.config.toml_path, self.config.build_dir]
        )
        