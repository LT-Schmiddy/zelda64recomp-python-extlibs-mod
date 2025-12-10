import sys
if __name__ == '__main__':
    print(f"Wrong file! This is the pyinvoke tasks file for the `modbuild.py` tool. Run `python[3] ./modbuild.py` to use it.")
    sys.exit(0)

import shutil, os, subprocess, pathlib
from pathlib import Path

from modbuildcore.config import *
from modbuildcore.handlers import *

from invoke import Context, task

import project as p

@task
def download(c: Context, force: bool = False):
    for download in [DownloadHandler(i) for i in p.downloads.values()]:
        if force or download.should_download():
            print(f"Downloading '{download.config.url}'...")
            download.download()
        else:
            print(f"Already downloaded '{download.config.download_path}'.")
    
    print("Downloads complete.")
    
@task
def extract(c: Context, force: bool = False):
    for extraction in [ArchiveExtractHandler(i) for i in p.archive_extractions.values()]:
        if force or extraction.should_extract():
            print(f"Extracting '{extraction.config.archive_path}' to '{extraction.config.extract_dir}'...")
            extraction.extract()
        else:
            print(f"Already extracted '{extraction.config.archive_path}'.")
    
    print("Extractions complete.")

@task
def makefile(c: Context):
    for makefile in [MakefileHandler(i) for i in p.makefiles.values()]:
        print(f"-> Running makefile '{makefile.config.makefile_path}'")
        makefile.run_make(c, p.make_path)

@task
def nrm(c: Context):
    for mod in [ModTomlHandler(i) for i in p.mod_tomls.values()]:
        print(f"-> Build '{mod.config.data['inputs']['mod_filename']}'.nrm from '{mod.config.toml_path}'")
        mod.run_mod_tool(c, p.mod_tool_path)

@task
def cmake_configure_and_build(c: Context):
    for project in [CMakeProjectHandler(i) for i in p.cmake_projects.values()]:
        for group in project.get_build_group_names():
            project.configure_and_build_group(c, p.cmake_path, group)


@task(
    default=True,
    pre=[download, extract, makefile, nrm, cmake_configure_and_build]
)
def all(c: Context):
    print("Done!")
    
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