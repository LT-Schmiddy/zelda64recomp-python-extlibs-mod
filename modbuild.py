#!/usr/bin/env python3
import sys, os, subprocess, pathlib
from pathlib import Path

prog_root = Path(__file__).parent

from invoke import task, Collection

import tasks

if __name__ == '__main__':
    from invoke import __version__, Program
    program = Program(
        name="Mod Builder",
        binary="modbuild.py",
        binary_names=["modbuild.py"],
        version=__version__,
        namespace=Collection.from_module(tasks)
    )

    program.run()
