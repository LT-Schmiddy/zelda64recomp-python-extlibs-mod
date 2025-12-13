import os, shutil, json, zipfile
from pathlib import Path

from . import tomls
from . import cmake

class ThunderstorePackageConfig:
    package_file: Path
    manifest: dict[str, str]
    readme_file: Path
    changelog_file: Path
    icon_file: Path
    mod_list: list[tomls.ModTomlConfig]
    cmake_build_list: list[cmake.CMakeBuildConfig]
    
    def __init__(self,
            package_file: Path,
            manifest: dict,
            readme_file: Path,
            changelog_file: Path,
            icon_file: Path,
            mod_list: list[tomls.ModTomlConfig],
            cmake_build_list: list[cmake.CMakeBuildConfig]
        ):
        self.package_file = package_file
        self.manifest = manifest
        self.mod_list = mod_list
        self.readme_file = readme_file
        self.changelog_file = changelog_file
        self.icon_file = icon_file
        self.cmake_build_list = cmake_build_list
        

class ThunderstorePackageHandler:
    config: ThunderstorePackageConfig
    
    def __init__(self, config: ThunderstorePackageConfig):
        self.config = config
        
    def assemble_package(self):
        # Thunderstore Metadata:
        output_file = zipfile.ZipFile(self.config.package_file, 'w', zipfile.ZIP_DEFLATED)
        output_file.writestr("manifest.json", json.dumps(self.config.manifest, indent=4))
        output_file.write(self.config.readme_file, "README.md")
        output_file.write(self.config.changelog_file, "CHANGELOG.md")
        output_file.write(self.config.icon_file, "icon.png")
        
        # Copying NRM outputs
        for toml in self.config.mod_list:
            output_file.write(toml.get_output_path(), toml.get_output_path().name)

        for cmake_build in self.config.cmake_build_list:
            for src, dst in cmake_build.output_files.items():
                output_file.write(src, dst)