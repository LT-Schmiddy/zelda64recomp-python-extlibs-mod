import sys
if __name__ == '__main__':
    print(f"Wrong file! This is the pyinvoke tasks file for the `modbuild.py` tool. Run `python[3] ./modbuild.py` to use it.")
    sys.exit(0)

import shutil, os, json
from pathlib import Path

from modbuildcore.jobs import *
from modbuildcore.utils import *

from invoke import Context, task, call

try: 
    import user_project as p # type: ignore
except ImportError as e:
    import project as p

ARG_SPLIT_CHAR = ","

_built_tomls: list[ModTomlJob] = []
_built_cmake_handlers: list[CMakeBuildJob] = []

def print_task_header(*args, **kwargs): 
    print_color('green', "\n-> ", *args, **kwargs)
    
def print_job_header(*args, **kwargs):
    print_color('blue', f"\n--> ", *args, **kwargs)

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
    
    dl_list : list[DownloadJob] = None
    if name is None:
        dl_list = p.downloads.values()
    else:
        dl_list = [p.downloads[i] for i in name.split(ARG_SPLIT_CHAR)]
    
    for download in dl_list:
        download.force = force
        download.resolve(c)
    
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
    
    extract_list : list[ArchiveExtractJob] = None
    if name is None:
        extract_list = p.archive_extractions.values()
    else:
        extract_list = [p.archive_extractions[i] for i in name.split(ARG_SPLIT_CHAR)]
    
    for extraction in extract_list:
        extraction.force = force
        extraction.resolve()

@task(help={
    'name': f"Only run specific makefile configurations. Names should be the keys used in `project.makefiles`, separated by '{ARG_SPLIT_CHAR}'."
})
def makefile(c: Context, name: str = None):
    """
    Builds the makefile configurations defined in `project.makefiles`.
    
    Entries in `project.makefiles` should be instances of `modbuildcore.makefiles.MakefileConfig`. 
    """
    print_task_header("Running makefiles...")
    
    makefile_list : list[MakefileJob] = None
    if name is None:
        makefile_list = p.makefiles.values()
    else:
        makefile_list = [p.makefiles[i] for i in name.split(ARG_SPLIT_CHAR)]
    
    for makefile in makefile_list:
        print_job_header(f"Running makefile '{makefile.config.makefile_path}'")
        makefile.resolve(c)

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
    toml_list : list[ModTomlJob] = None
    if name is None:
        toml_list = p.mod_tomls.values()
    else:
        toml_list = [p.mod_tomls[i] for i in name.split(ARG_SPLIT_CHAR)]
    
    for mod in toml_list:
        mod.run_nrm_path_fix = path_fix
        mod.resolve(c)

@task(help={
    'group_name': "Build selected groups by name. Should not be used with `release_group`." \
        " Group names should be the keys used in `project.mod_tomls`, separated by '{ARG_SPLIT_CHAR}'.",
    'build_name': "Only run specific builds within selected groups. Build names should be the keys used in "\
        "`project.mod_tomls`, separated by '{ARG_SPLIT_CHAR}'. Will error if any build name is not in all groups.",
})
def cmake(c: Context, group_name: str = None, build_name: str = None):
    """
    Run CMake Builds by group. Build groups are `dict[str, CMakeBuildConfig]` entries in `project.cmake_build_groups`. 
     
    By default, this command will only build the default build group, since running too many CMake builds can be time consuming,
    and some CMake build outputs can overwrite each other in the test directory. The default group is set via the varaible 
    `project.cmake_default_build_group_name`, which should be assigned to a key value from `project.cmake_build_groups`. 
    Generally, this should be some sort of debug build group.
    
    You will also need to select a build group to be used for releases. This will be set with the variable `project.cmake_release_build_group_name`
    """
    print_task_header("Running CMake builds...")
    global _built_cmake_handlers
    
    selected_groups: dict[str, dict[str, CMakeBuildJob]] = {}

    if group_name is not None:
        for i in group_name.split(ARG_SPLIT_CHAR):
            selected_groups[i] = p.cmake_build_groups[i]
    else:
        selected_groups[p.cmake_default_build_group_name] = p.cmake_build_groups[p.cmake_default_build_group_name]
        
    for group_key, group in selected_groups.items():
        if build_name is None:
            for build_key, build_job in  group.items():
                build_job.resolve(c)
        else:
            for build_key, build_job in [(bkey, group[bkey]) for bkey in build_name.split(ARG_SPLIT_CHAR)]:
                build_job.resolve(c)
                    
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


# @task
# def create_thunderstore_package(c: Context, name: str = None):
#     """
#     Creates Thunderstore .zip packages, as specified in `project.thunderstore_packages`.
    
#     Entries in `project.thunderstore_packages` should be instances of `modbuildcore.thunderstore.ThunderstorePackageConfig`. 
#     This command does not build mod tomls or cmake builds that the project depends on. Consider looking at the `thunderstore` command for that.
#     """
#     print_task_header("Creating Thunderstore packages...")
    
#     package_list: list[ThunderstorePackageHandler] = None
#     if name is None:
#         package_list = [ThunderstorePackageHandler(i) for i in p.thunderstore_packages.values()]
#     else:
#         package_list = [ThunderstorePackageHandler(p.thunderstore_packages[i]) for i in name.split(ARG_SPLIT_CHAR)]
        
#     for package in package_list:
#         package.assemble_package()

# @task
# def print_thunderstore_manifest(c: Context, name: str = None):
#     package_list: list[ThunderstorePackageHandler] = None
#     if name is None:
#         package_list = [ThunderstorePackageHandler(i) for i in p.thunderstore_packages.values()]
#     else:
#         package_list = [ThunderstorePackageHandler(p.thunderstore_packages[i]) for i in name.split(ARG_SPLIT_CHAR)]
        
#     for package in package_list:
#         print(json.dumps(package.config.manifest, indent=4))


# @task(
#     pre=[download, extract, makefile, nrm, call(cmake, release_group=True), create_thunderstore_package]
# )
# def thunderstore(c: Context):
#     """
#     Compile makefiles, .nrm files, and CMake release builds, and then create the Thunderstore package zip. Handles downloads and extractions if needed.
    
#     Shortcut for `modbuild.py download extract makefile nrm cmake -r create-thunderstore-package`.
#     """
#     pass


@task
def clean(c: Context):
    """
    Deletes files and folders specified in `project.clean_paths`. Used for deleting build folders.
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
    """_summary_
    Deletes files and folders specified in `project.clean_paths` and `project.distclean_paths`. Used for deleting build folders andexit downloaded artifacts.
    """
    for path in p.distclean_paths:
        if path.is_file():
            os.remove(path)
            print(f"Deleted {path}")
        elif path.is_dir():
            shutil.rmtree(path)
            print(f"Deleted {path}")
        else:
            print(f"Could not delete {path}")
            
@task
def invoke_test(c: Context):
    makefile(c)
    nrm(c)