import sys
if __name__ == '__main__':
    print(f"Wrong file! This is the pyinvoke tasks file for the `modbuild.py` tool. Run `python[3] ./modbuild.py` to use it.")
    sys.exit(0)

import shutil, os, json
from pathlib import Path

from modbuildcore.config import *
from modbuildcore.handlers import *

from invoke import Context, task, call

import project as p

ARG_SPLIT_CHAR = ","

_built_tomls: list[ModTomlHandler] = []
_built_cmake_handlers: list[CMakeBuildHandler] = []

@task
def download(c: Context, force: bool = False, name: str = None):
    dl_list : list[DownloadHandler] = None
    if name is None:
        dl_list = [DownloadHandler(i) for i in p.downloads.values()]
    else:
        dl_list = [DownloadHandler(p.downloads[i]) for i in name.split(ARG_SPLIT_CHAR)]
    
    for download in dl_list:
        if force or download.should_download():
            print(f"Downloading '{download.config.url}'...")
            download.download()
        else:
            print(f"Already downloaded '{download.config.download_path}'.")
    
    print("Downloads complete.")
    
@task
def extract(c: Context, force: bool = False, name: str = None):
    extract_list : list[ArchiveExtractHandler] = None
    if name is None:
        extract_list = [ArchiveExtractHandler(i) for i in p.archive_extractions.values()]
    else:
        extract_list = [ArchiveExtractHandler(p.archive_extractions[i]) for i in name.split(ARG_SPLIT_CHAR)]
    
    for extraction in extract_list:
        if force or extraction.should_extract():
            print(f"Extracting '{extraction.config.archive_path}' to '{extraction.config.extract_dir}'...")
            extraction.extract()
        else:
            print(f"Already extracted '{extraction.config.archive_path}'.")
    
    print("Extractions complete.")

@task
def makefile(c: Context, name: str = None):
    makefile_list : list[MakefileHandler] = None
    if name is None:
        makefile_list = [MakefileHandler(i) for i in p.makefiles.values()]
    else:
        makefile_list = [MakefileHandler(p.makefiles[i]) for i in name.split(ARG_SPLIT_CHAR)]
    
    for makefile in makefile_list:
        print(f"-> Running makefile '{makefile.config.makefile_path}'")
        makefile.run_make(c, p.make_path)

@task
def nrm(c: Context, name: str = None, path_fix: bool = True):
    global _built_tomls
    toml_list : list[ModTomlHandler] = None
    if name is None:
        toml_list = [ModTomlHandler(i) for i in p.mod_tomls.values()]
    else:
        toml_list = [ModTomlHandler(p.mod_tomls[i]) for i in name.split(ARG_SPLIT_CHAR)]
    
    for mod in toml_list:
        print(f"-> Building {mod.config.data['inputs']['mod_filename']}.nrm from '{mod.config.toml_path}'...")
        mod.run_mod_tool(c, p.mod_tool_path)
        
        if (path_fix):
            print(f"\tCorrecting path backslashes in {mod.config.data['inputs']['mod_filename']}.nrm...")
            mod.run_nrm_path_fix()
        
        _built_tomls.append(mod)

@task
def cmake(c: Context, name: str = None, group: str = None, build_name: str = None):
    global _built_cmake_handlers
    project_list: list[CMakeProjectHandler] = None
    if name is None:
        project_list = [CMakeProjectHandler(i) for i in p.cmake_projects.values()]
    else:
        project_list = [CMakeProjectHandler(p.cmake_projects[i]) for i in name.split(ARG_SPLIT_CHAR)]
    
    for project in project_list:
        selected_group = p.cmake_default_build_group
        if group is not None:
            selected_group = group
            
        if build_name is None:
            for handler in project.build_handlers[selected_group].values():
                handler.run_configure(c, p.cmake_path)
                handler.run_build(c, p.cmake_path)
                _built_cmake_handlers.append(handler)
        else:
            project.build_handlers[selected_group][build_name].run_configure(c, p.cmake_path)
            project.build_handlers[selected_group][build_name].run_build(c, p.cmake_path)
            _built_cmake_handlers.append(project.build_handlers[selected_group][build_name])
            
@task
def update_test_env(C: Context):
    global _built_tomls, _built_cmake_handlers
    os.makedirs(p.test_env_mod_dir, exist_ok=True)
    
    # Copying NRM outputs
    for toml in _built_tomls:
        src = toml.config.get_output_path()
        dst = p.test_env_mod_dir.joinpath(toml.config.get_output_path().name)
        print(f"Copying '{str(src)}' to '{str(dst)}'...")
        shutil.copy(src, dst)

    for handler in _built_cmake_handlers:
        for src, dst in handler.config.output_files.items():
        
            if not dst.is_absolute():
                dst = p.test_env_mod_dir.joinpath(dst)
            
            print(f"Copying '{str(src)}' to '{str(dst)}'...")
            shutil.copy(src, dst)

@task(
    default=True,
    pre=[download, extract, makefile, nrm, cmake, update_test_env]
)
def all(c: Context):
    pass


@task
def create_thunderstore_package(c: Context, name: str = None):
    package_list: list[ThunderstorePackageHandler] = None
    if name is None:
        package_list = [ThunderstorePackageHandler(i) for i in p.thunderstore_packages.values()]
    else:
        package_list = [ThunderstorePackageHandler(p.thunderstore_packages[i]) for i in name.split(ARG_SPLIT_CHAR)]
        
    for package in package_list:
        package.assemble_package()

@task
def print_thunderstore_manifest(c: Context, name: str = None):
    package_list: list[ThunderstorePackageHandler] = None
    if name is None:
        package_list = [ThunderstorePackageHandler(i) for i in p.thunderstore_packages.values()]
    else:
        package_list = [ThunderstorePackageHandler(p.thunderstore_packages[i]) for i in name.split(ARG_SPLIT_CHAR)]
        
    for package in package_list:
        print(json.dumps(package.config.manifest, indent=4))


@task(
    pre=[download, extract, makefile, nrm, call(cmake, group=p.cmake_thunderstore_build_group), create_thunderstore_package]
)
def thunderstore(c: Context):
    pass


@task
def clean(c: Context):
    for path in p.clean_paths:
        if path.is_file():
            os.remove(path)
            print(f"Deleted {path}")
        elif path.is_dir():
            shutil.rmtree(path)
            print(f"Deleted {path}")
        else:
            print(f"Could not delete {path}")
            
@task(
    pre=[clean]
)
def distclean(c: Context):
    for path in p.distclean_paths:
        if path.is_file():
            os.remove(path)
            print(f"Deleted {path}")
        elif path.is_dir():
            shutil.rmtree(path)
            print(f"Deleted {path}")
        else:
            print(f"Could not delete {path}")