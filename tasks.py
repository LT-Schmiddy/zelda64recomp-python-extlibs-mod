import sys, os, subprocess, pathlib
from pathlib import Path

from invoke import task, Collection

@task
def build(c):
    print("Building!")
    