import tomllib, pathlib, enum, os
from pathlib import Path

from invoke import Context

from .utils import invoke_subprocess_run

class ModTomlConfig:
    class MakeEnvSpecialVals(enum.Enum):
        TOML_ELF_PATH = 0
        TOML_ELF_PARENT_PATH = 1
        
    toml_path: Path
    makefile_path: Path
    makefile_extended_env: dict[str, str | MakeEnvSpecialVals]
    
    
    def __init__(self, toml_path: Path, makefile_path: Path, makefile_extended_env: dict[str, str | MakeEnvSpecialVals]):
        self.toml_path = toml_path
        self.makefile_path = makefile_path
        self.makefile_extended_env = makefile_extended_env
    
    
class ModTomlHandler:
    config: ModTomlConfig
    data: dict
    
    def __init__(self, config: ModTomlConfig):
        self.config = config
        self.data = tomllib.loads(self.config.toml_path.read_text())
    
    def get_path_from_toml(self, rel_path: str | Path) -> Path:
        return self.config.toml_path.parent.joinpath(rel_path).resolve()
    
    def get_elf_path(self) -> Path:
        return self.get_path_from_toml(self.data["inputs"]["elf_path"])
    
    def resolve_extended_env(self):
        extended_env =  self.config.makefile_extended_env.copy()
        
        # Environmental variables can only be strings. Resolving all non-string entries.
        env_keys = extended_env.keys()
        for key in env_keys:
            if extended_env[key] == ModTomlConfig.MakeEnvSpecialVals.TOML_ELF_PATH:
                toml_elf_path = self.get_elf_path()
                extended_env[key] = str(toml_elf_path)
                
            if extended_env[key] == ModTomlConfig.MakeEnvSpecialVals.TOML_ELF_PARENT_PATH:
                toml_elf_parent_path = self.get_elf_path().parent
                extended_env[key] = str(toml_elf_parent_path)
            
            # Paths need special attention for Make compatability on Windows.
            if isinstance(extended_env[key], Path):
                extended_env[key] = str(extended_env[key]).replace("\\", "/")
            
            elif not isinstance(extended_env[key], str):
                extended_env[key] = str(extended_env[key])
                
        return extended_env
        
    def run_make(self, context: Context, make_binary: Path):
        extended_env = self.resolve_extended_env()
        make_env = os.environ.copy()
        make_env.update(extended_env)
        
        invoke_subprocess_run(context, True,
            [make_binary, "-f", self.config.makefile_path],
            env=make_env
        )
        
    def run_mod_tool(self, context: Context, mod_tool_binary: Path):
        invoke_subprocess_run(context, True,
            [mod_tool_binary, self.config.toml_path, self.get_elf_path().parent]
        )
        