import tomllib, pathlib, os
from pathlib import Path

from invoke import Context

from .utils import invoke_subprocess_run

class MakefileConfig:
    makefile_path: Path
    extended_env: dict[str, str]
    
    def __init__(self, makefile_path: Path, extended_env: dict[str, str]):
        self.makefile_path = makefile_path
        self.extended_env = extended_env
    
class MakefileHandler:
    config: MakefileConfig
    
    def __init__(self, config: MakefileConfig):
        self.config = config
    
    def run_make(self, context: Context, make_binary: Path):
        make_env = os.environ.copy()
        make_env.update(self.config.extended_env)
        
        invoke_subprocess_run(context, True,
            [make_binary, "-f", self.config.makefile_path],
            env=make_env
        )
        