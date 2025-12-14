import sys
if __name__ == '__main__':
    print(f"Wrong file! This is the pyinvoke tasks file for the `modbuild.py` tool. Run `python[3] ./modbuild.py` to use it.")
    sys.exit(0)

import shutil, os, json
from pathlib import Path

from modbuildcore.config import *
from modbuildcore.handlers import *
from modbuildcore.utils import *

from invoke import Context, task, call

try: 
    import user_project as p # type: ignore
except ImportError as e:
    import project as p

ARG_SPLIT_CHAR = ","

_built_tomls: list[ModTomlHandler] = []
_built_cmake_handlers: list[CMakeBuildHandler] = []

def print_task_header(*args, **kwargs): 
    print_color('green', "\n-> ", *args, **kwargs)

@task(help={
    'force': "Redownloads any previously downloaded files.",
    'name': f"Only download specific files. Names should be the keys used in `project.downloads`, separated by '{ARG_SPLIT_CHAR}'."
})
def download(c: Context, force: bool = False, name: str = None):
    """
    Runs the downloads defined in `project.downloads`. 
    
    Entries in `project.downloads` should be instances of `modbuildcore.downloads.DownloadConfig`. 
    By default, downloads will be skipped if the destination file already exists.
    """
    print_task_header("Performing downloads...")
    
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
    
@task(help={
    'force': "Re-extract archives even if the output folder already exists",
    'name': f"Only extract specific archives. Names should be the keys used in `project.archive_extractions`, separated by '{ARG_SPLIT_CHAR}'."
})
def extract(c: Context, force: bool = False, name: str = None):
    """
    Runs the archive extractions defined in `project.archive_extractions`. 
    
    Entries in `project.archive_extractions` should be instances of `modbuildcore.archives.ArchiveExtractConfig`. 
    By default, extractions will be skipped if the destination folder already exists.
    """
    print_task_header("Extracting archives...")
    
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

@task(help={
    'name': f"Only run specific makefile configurations. Names should be the keys used in `project.makefiles`, separated by '{ARG_SPLIT_CHAR}'."
})
def makefile(c: Context, name: str = None):
    """
    Builds the makefile configurations defined in `project.makefiles`.
    
    Entries in `project.makefiles` should be instances of `modbuildcore.makefiles.MakefileConfig`. 
    """
    print_task_header("Running makefiles...")
    
    makefile_list : list[MakefileHandler] = None
    if name is None:
        makefile_list = [MakefileHandler(i) for i in p.makefiles.values()]
    else:
        makefile_list = [MakefileHandler(p.makefiles[i]) for i in name.split(ARG_SPLIT_CHAR)]
    
    for makefile in makefile_list:
        print(f"-> Running makefile '{makefile.config.makefile_path}'")
        makefile.run_make(c, p.make_path)

@task(help={
    'name': f"Only build .nrm files from specific registered .toml files. " \
        "Names should be the keys used in `project.mod_tomls`, separated by '{ARG_SPLIT_CHAR}'.",
    'path_fix': "EXPERIMENTAL! Reconstructs the .nrm file after RecompModTool finishes in order to eliminate backslashes from filepaths."
})
def nrm(c: Context, name: str = None, path_fix: bool = p.nrm_path_fix_by_default):
    """
    Builds .nrm files from .toml files, as specified in `project.mod_tomls`.
    
    Entries in `project.mod_tomls` should be instances of `modbuildcore.makefiles.ModTomlConfig`. 
    This task does not run makefiles as a prerequisite. To update mod code, run `./modbuild.py makefile nrm`.
    """
    print_task_header("Building NRM files...")
    
    global _built_tomls
    toml_list : list[ModTomlHandler] = None
    if name is None:
        toml_list = [ModTomlHandler(i) for i in p.mod_tomls.values()]
    else:
        toml_list = [ModTomlHandler(p.mod_tomls[i]) for i in name.split(ARG_SPLIT_CHAR)]
    
    for mod in toml_list:
        print(f"Building {mod.config.data['inputs']['mod_filename']}.nrm from '{mod.config.toml_path}'...")
        mod.run_mod_tool(c, p.mod_tool_path)
        
        if (path_fix):
            print(f"\tCorrecting path backslashes in {mod.config.data['inputs']['mod_filename']}.nrm...")
            mod.run_nrm_path_fix()
        
        _built_tomls.append(mod)

@task(help={
    
})
def cmake(c: Context, project_name: str = None, release_group: bool=False, group_name: str = None, build_name: str = None):
    """
    Run the
    """
    
    print_task_header("Running CMake builds...")
    
    global _built_cmake_handlers
    project_list: list[CMakeProjectHandler] = None
    if project_name is None:
        project_list = [CMakeProjectHandler(i) for i in p.cmake_projects.values()]
    else:
        project_list = [CMakeProjectHandler(p.cmake_projects[i]) for i in project_name.split(ARG_SPLIT_CHAR)]
    
    for project in project_list:
        selected_groups: dict[str, dict[str, CMakeBuildHandler]] = {}
        if release_group and (group_name is not None):
            print_error("Error: Subcommand `cmake` arguments `release_group` and `group_name` cannot be used together.")
            sys.exit(1)
        
        if release_group:
            selected_groups[project.config.release_build_group_name] = project.get_release_build_group()
        elif group_name is not None:
            for i in group_name.split(ARG_SPLIT_CHAR):
                selected_groups[i] = project.build_handlers[i]
        else:
            selected_groups[project.config.debug_build_group_name] = project.get_debug_build_group()
            
        for group_key, group in selected_groups.items():
            if build_name is None:
                for build_key, build_handler in group.items():
                    print_color('blue', f"\n--> CMake build '{build_key}', from build group '{group_key}': Configure")
                    build_handler.run_configure(c, p.cmake_path)
                    
                    print_color('blue', f"\n--> CMake build '{build_key}', from build group '{group_key}': Build")
                    build_handler.run_build(c, p.cmake_path)
                    _built_cmake_handlers.append(build_handler)
            else:
                for build_key, build_handler in [(i, group[i]) for i in build_name.split(ARG_SPLIT_CHAR)]:
                
                    print_color('blue', f"\n--> CMake build '{build_key}', from build group '{group_key}': Configure")
                    build_handler.run_configure(c, p.cmake_path)
                    
                    print_color('blue', f"\n--> CMake build '{build_key}', from build group '{group_key}': Build")
                    build_handler.run_build(c, p.cmake_path)
                    _built_cmake_handlers.append(build_handler)
                    
@task
def update_test_env(C: Context):
    """
    Updates the test environment mod folder with the resultant .nrm files and CMake build outputs from the current run.
    
    This utility decides which files to copy by tracking which ModTomlConfig and CMakeBuildConfig objects were processed while running.
    Therefore, if neither the `nrm` or `cmake` tasks were run in the current invokation, this command will do nothing.
    """
    print_task_header("Updating mod test environment...")
    
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
def build(c: Context):
    """
    Compile makefiles, .nrm files, and CMake debug builds, and then update the test_env folder. Handles downloads and extractions if needed.
    
    Shortcut for `modbuild.py download extract makefile nrm cmake update-test-env`.
    """
    pass


@task
def create_thunderstore_package(c: Context, name: str = None):
    print_task_header("Creating Thunderstore packages...")
    
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
    pre=[download, extract, makefile, nrm, call(cmake, release_group=True), create_thunderstore_package]
)
def thunderstore(c: Context):
    """
    Compile makefiles, .nrm files, and CMake release builds, and then create the Thunderstore package zip. Handles downloads and extractions if needed.
    
    Shortcut for `modbuild.py download extract makefile nrm cmake -r create-thunderstore-package`.
    """
    pass


@task
def clean(c: Context):
    """
    Deletes the contents of the builds folder.
    """
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