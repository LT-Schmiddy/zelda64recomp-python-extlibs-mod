import sys, os, subprocess, pathlib
from pathlib import Path

from invoke import task, Collection, Context

from modbuildcore.utils import ctx_run_subprocess

@task(default=True)
def build(c: Context):
    print("Building!")
    ctx_run_subprocess(c, True,
        ["busybox.exe", "echso", "hi"]
    )
    c.run("busybox ecsho hi")
    print("Done!")