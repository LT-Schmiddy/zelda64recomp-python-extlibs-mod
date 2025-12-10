import pathlib, os
from pathlib import Path

from invoke import Context

from .utils import invoke_subprocess_run

# Class declaration before definition:
class CMakeProjectConfig:
    ...


class CMakeBuildConfig:
    cmake_project: CMakeProjectConfig
    config_args: list[str]
    build_args: list[str]
    output_files: list[Path]
    
    def __init__(self, cmake_project: CMakeProjectConfig, output_files: list[Path], config_args: list[str], build_args: list[str]):
        self.cmake_project = cmake_project
        self.output_files = output_files
        self.config_args = config_args
        self.build_args = build_args
        
        
    @classmethod
    def from_preset_pair(cls, cmake_project: CMakeProjectConfig, output_files: list[Path], config_preset_name: str, build_preset_name: str = None):
        if build_preset_name is None:
            build_preset_name = config_preset_name
        
        return cls(
            cmake_project,
            output_files,
            ["--preset", config_preset_name, cmake_project.project_dir],
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
            cwd=self.config.cmake_project.project_dir
        )
    
    def run_build(self, c: Context, cmake_bin: Path):
        cmake_env = os.environ.copy()
        cmake_env.update(self.config.cmake_project.extended_env)
        
        invoke_subprocess_run(c, True,
            [cmake_bin] + self.config.build_args,
            env=cmake_env,
            cwd=self.config.cmake_project.project_dir
        )


class CMakeProjectConfig:
    project_dir: Path
    extended_env: dict[str, str]
    build_groups: dict[str, dict[str, CMakeBuildConfig]]
    
    def __init__(self, project_dir: Path, expanded_env: dict[str, str]):
        self.project_dir = project_dir
        self.extended_env = expanded_env
        self.build_groups = {}
    
    def add_preset_pair_build_to_group(self, group_name: str, entry_name: str, output_files: list[Path], config_preset_name: str, build_preset_name: str = None):
        new_build = CMakeBuildConfig.from_preset_pair(self, output_files, config_preset_name, build_preset_name)
        
        if group_name not in self.build_groups:
            self.build_groups[group_name] = {}
        
        self.build_groups[group_name][entry_name] = new_build
        
        
class CMakeProjectHandler:
    config: CMakeProjectConfig
    
    build_handlers: dict[str, dict[str, CMakeBuildHandler]]
    
    def __init__(self, config: CMakeProjectConfig):
        self.config = config
        
        self.build_handlers = {}
        for group_name, group_configs in self.config.build_groups.items():
            self.build_handlers[group_name] = {}
            for build_name, build_config in group_configs.items():
                self.build_handlers[group_name][build_name] = CMakeBuildHandler(build_config)
            
    def get_build_group_names(self) -> list[str]:
        return list(self.build_handlers.keys())
    
    def configure_group(self, c: Context, cmake_path: Path, group_name: str):
        for handler in self.build_handlers[group_name].values():
            handler.run_configure(c, cmake_path)
    
    def build_group(self, c: Context, cmake_path: Path, group_name: str):
        for handler in self.build_handlers[group_name].values():
            handler.run_build(c, cmake_path)
            
    
    def configure_and_build_group(self, c: Context, cmake_path: Path, group_name: str):
        for handler in self.build_handlers[group_name].values():
            handler.run_configure(c, cmake_path)
            handler.run_build(c, cmake_path)