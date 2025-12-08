import sys, os, subprocess, pathlib
from pathlib import Path

from modbuildcore.utils import invoke_subprocess_run
from invoke import task, Context

project_root = Path(__file__).parent

@task(default=True)
def build(c: Context):
    print("Building!")
    print("Done!")
    

@task
def extlib_win(c: Context):
    invoke_subprocess_run(c, True,
        ["cmake", "--preset=zig-windows-x64-Debug", "-DLIB_NAME=extlib_test", "."]
    )
    invoke_subprocess_run(c, True,
        ["cmake", "--build", "--preset=zig-windows-x64-Debug"]
    )

@task
def extlib_macos(c: Context):
    invoke_subprocess_run(c, True,
        ["cmake", "--preset=zig-macos-aarch64-Debug", "-DLIB_NAME=extlib_test", "."]
    )
    invoke_subprocess_run(c, True,
        ["cmake", "--build", "--preset=zig-macos-aarch64-Debug"]
    )
    
@task
def extlib_linux(c: Context):
    invoke_subprocess_run(c, True,
        ["cmake", "--preset=zig-linux-x64-Debug", "-DLIB_NAME=extlib_test", "."]
    )
    invoke_subprocess_run(c, True,
        ["cmake", "--build", "--preset=zig-linux-x64-Debug"]
    )