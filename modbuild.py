#!/usr/bin/env python3

# Loading 'py' folder into the module search path.
import sys
from pathlib import Path
prog_root = Path(__file__).parent
sys.path.append(str(prog_root.joinpath("py")))
print(sys.path)

# Program startup:
import sys, os, subprocess, pathlib

from invoke import task, Collection, __version__, Program
from invoke.config import Config, merge_dicts


class Z64rModBuildConfig(Config):
    prefix = 'z64r_modbuild'
    
    @staticmethod
    def global_defaults():
        their_defaults = Config.global_defaults()
        my_defaults = {}
        return merge_dicts(their_defaults, my_defaults)

import tasks

if __name__ == '__main__':
    
    program = Program(
        name="Mod Builder",
        binary="modbuild.py",
        binary_names=["modbuild.py"],
        version=__version__,
        namespace=Collection.from_module(tasks),
        config_class=Z64rModBuildConfig,
    )

    program.run()
