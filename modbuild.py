#!/usr/bin/env python3
import sys, os, subprocess, pathlib
from pathlib import Path

prog_root = Path(__file__).parent

from invoke import task, Collection, __version__, Program
from invoke.config import Config, merge_dicts

class Z64rModBuildConfig(Config):
    prefix = 'z64r_modbuild'
    
    @staticmethod
    def global_defaults():
        their_defaults = Config.global_defaults()
        my_defaults = {
            'run': {
                'echo': True,
            },
        }
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
