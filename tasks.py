import sys, shutil, os, subprocess, pathlib
from pathlib import Path

import modbuildcore
from modbuildcore.downloads import DownloadArchiveHandler
from modbuildcore.makefiles import MakefileHandler
from modbuildcore.tomls import ModTomlHandler
from modbuildcore.utils import invoke_subprocess_run
from invoke import task, Context

project_root = Path(__file__).parent

import project as p

@task
def download_archives(c: Context, force: bool = False, reextract: bool = False):
    for archive in [DownloadArchiveHandler(i, p.mod_project.archive_downloads_dir) for i in p.mod_project.archive_downloads]:
        if force or archive.should_download():
            print(f"Downloading '{archive.entry.url}'...")
            archive.download()
        else:
            print(f"Already downloaded '{archive.cache_path.name}'.")

        if force or reextract or archive.should_extract():
            print(f"Extracting '{archive.cache_path.name}' to '{archive.entry.extract_dir}'...")
            archive.extract()
        else:
            print(f"Already extracted '{archive.cache_path.name}'.")
    
    print("Downloads complete.")

@task
def run_makefiles(c: Context):
    for makefile in [MakefileHandler(i) for i in p.mod_project.makefiles]:
        makefile.run_make(c, p.mod_project.make_path)

@task
def build_nrms(c: Context):
    for mod in [ModTomlHandler(i) for i in p.mod_project.mod_tomls]:
        mod.run_mod_tool(c, p.mod_project.mod_tool_path)

@task(
    default=True,
    pre=[download_archives, run_makefiles, build_nrms]
)
def all(c: Context):
    print("Done!")
    
@task
def clean(c: Context):
    for path in p.mod_project.get_paths_for_cleaning():
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
    for path in p.mod_project.get_paths_for_distcleaning():
        if path.is_file():
            os.remove(path)
            print(f"Deleted {path}")
        elif path.is_dir():
            shutil.rmtree(path)
            print(f"Deleted {path}")
        else:
            print(f"Could not delete {path}")