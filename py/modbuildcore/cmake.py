import pathlib, os
from pathlib import Path

from invoke import Context

from .utils import invoke_subprocess_run

# Class declaration before definition:
class CMakeProjectConfig:
    project_working_dir: Path
    extended_env: dict[str, str]
    
    def __init__(self, project_working_dir: Path, expanded_env: dict[str, str]):
        self.project_working_dir = project_working_dir
        self.extended_env = expanded_env


class CMakeBuildConfig:
    config_args: list[str]
    build_args: list[str]
    output_files: dict[Path, Path]
    
    def __init__(self, cmake_project: CMakeProjectConfig, output_files: dict[Path, Path], config_args: list[str], build_args: list[str]):
        self.cmake_project = cmake_project
        self.output_files = output_files
        self.config_args = config_args
        self.build_args = build_args
        
        
    @classmethod
    def from_preset_pair(cls, cmake_project: CMakeProjectConfig, output_files: dict[Path, Path], config_preset_name: str, build_preset_name: str = None):
        if build_preset_name is None:
            build_preset_name = config_preset_name
        
        return cls(
            cmake_project,
            output_files,
            ["--preset", config_preset_name, cmake_project.project_working_dir],
            ["--build", "--preset", build_preset_name]
        )
        

class CMakeBuildHandler:
    config: CMakeBuildConfig
    def __init__(self, config: CMakeBuildConfig):
        self.config = config
        
    def run_configure(self, c: Context, cmake_bin: Path):
        cmake_env = os.environ.copy()
        cmake_env.update(self.config.cmake_project.extended_env)
        
        invoke_subprocess_run(c, True,
            [cmake_bin] + self.config.config_args,
            env=cmake_env,
            cwd=self.config.cmake_project.project_working_dir
        )
    
    def run_build(self, c: Context, cmake_bin: Path):
        cmake_env = os.environ.copy()
        cmake_env.update(self.config.cmake_project.extended_env)
        
        invoke_subprocess_run(c, True,
            [cmake_bin] + self.config.build_args,
            env=cmake_env,
            cwd=self.config.cmake_project.project_working_dir
        )

