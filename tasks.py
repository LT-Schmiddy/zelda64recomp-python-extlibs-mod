import sys, os, subprocess, pathlib
from pathlib import Path

import modbuildcore
from modbuildcore.downloads import DownloadArchiveHandler
from modbuildcore.utils import invoke_subprocess_run
from invoke import task, Context

project_root = Path(__file__).parent

from project import mod_project


@task
def download_archives(c: Context, force: bool = False, reextract: bool = False):
    for archive in [DownloadArchiveHandler(i) for i in mod_project.archive_downloads]:
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

@task(
    pre=[download_archives]
)
def build_nrm(c: Context):
    print("Building NRMs")    

@task(
    default=True,
    pre=[build_nrm]
)
def build_all(c: Context):
    print("Done!")
    
