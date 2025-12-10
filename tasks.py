import sys, shutil, os, subprocess, pathlib
from pathlib import Path

import modbuildcore
from modbuildcore.archives import ArchiveExtractHandler
from modbuildcore.downloads import DownloadHandler
from modbuildcore.makefiles import MakefileHandler
from modbuildcore.tomls import ModTomlHandler
from modbuildcore.utils import invoke_subprocess_run
from invoke import task, Context

project_root = Path(__file__).parent

import project as p

@task
def download(c: Context, force: bool = False):
    for download in [DownloadHandler(i) for i in p.mod_project.downloads]:
        if force or download.should_download():
            print(f"Downloading '{download.config.url}'...")
            download.download()
        else:
            print(f"Already downloaded '{download.config.download_path}'.")
    
    print("Downloads complete.")
    
@task
def extract(c: Context, force: bool = False):
    for extraction in [ArchiveExtractHandler(i) for i in p.mod_project.archive_extractions]:
        if force or extraction.should_extract():
            print(f"Extracting '{extraction.config.archive_path}' to '{extraction.config.extract_dir}'...")
            extraction.extract()
        else:
            print(f"Already extracted '{extraction.config.archive_path}'.")
    
    print("Extractions complete.")

@task
def run_makefiles(c: Context):
    for makefile in [MakefileHandler(i) for i in p.mod_project.makefiles]:
        print(f"-> Running makefile '{makefile.config.makefile_path}'")
        makefile.run_make(c, p.mod_project.make_path)

@task
def build_nrms(c: Context):
    for mod in [ModTomlHandler(i) for i in p.mod_project.mod_tomls]:
        print(f"-> Build '{mod.config.data['inputs']['mod_filename']}'.nrm from '{mod.config.toml_path}'")
        mod.run_mod_tool(c, p.mod_project.mod_tool_path)

@task(
    default=True,
    pre=[download, extract, run_makefiles, build_nrms]
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