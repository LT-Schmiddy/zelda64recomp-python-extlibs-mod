
import sys, os, subprocess, pathlib, shutil, json, zipfile, re
from pathlib import Path

from invoke import Context
from invoke.config import Config

# For a couple different reasons (primarily related to cross-platform compatability),
# it will usually be better for us to use subprocess instead of shell commands.
# This is a convienient helper function to make subprocess work better with the
# invoke content:
def invoke_subprocess_run(c: Context, required: bool, *args, **kwargs) -> subprocess.CompletedProcess:
    cwd: str = c.cwd
    echo: bool = c.config['run']['echo']
    echo_format: str = c.config['run']['echo_format']
    warn: bool = c.config['run']['warn']
    dry: bool = c.config['run']['dry']
    
    subprocess_str = str(args[0])
    
    if echo:
        print(echo_format.replace("{command}", f"subprocess.run({str(subprocess_str)})"))
    
    if dry:
        return None
    
    if 'cwd' not in kwargs:
        kwargs['cwd'] = Path(cwd)

    result: subprocess.CompletedProcess = subprocess.run(*args, **kwargs)
    
    if result.returncode != 0:
        if warn or not required:
            print(f"WARNING! Command '{subprocess_str}' returned non-zero exit status {result.returncode}.")
        else:
            print(f"FATAL! Command '{subprocess_str}' returned non-zero exit status {result.returncode}. Aborting...")
            sys.exit(1)
    
    return result

def slugify(text: str) -> str:
    text = text.strip()
    text = re.sub(r'[\s_]+', '_', text)
    text = re.sub(r'[^a-zA-Z0-9_]', '', text)
    return text
