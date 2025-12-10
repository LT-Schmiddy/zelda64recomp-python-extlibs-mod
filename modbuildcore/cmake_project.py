import pathlib, os
from pathlib import Path

from invoke import Context

from .utils import invoke_subprocess_run


class CMakeProjectConfig:
    project_dir: Path
    expanded_env: dict[str, str]
    
    def __init__(self, project_dir: Path, expanded_env: dict[str, str]):
        self.project_dir = project_dir
        self.expanded_env = expanded_env
        
        
class CMakeProjectHandler:
    config: CMakeProjectConfig
    
    def __init__(self, config: CMakeProjectConfig):
        self.config = config
    
    def configure_preset(self, c: Context, cmake_bin: Path, preset_name: str):
        cmake_env = os.environ.copy()
        cmake_env.update(self.config.extended_env)
        
        invoke_subprocess_run(c, True,
            [cmake_bin, "--preset", preset_name, self.config.project_dir],
            env=cmake_env
        )
        
    def build_preset(self, c: Context, cmake_bin: Path, preset_name: str):
        cmake_env = os.environ.copy()
        cmake_env.update(self.config.extended_env)
        
        invoke_subprocess_run(c, True,
            [cmake_bin, "--build", "--preset", preset_name],
            env=cmake_env
        )
        