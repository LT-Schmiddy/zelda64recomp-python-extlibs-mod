import pathlib, os, shutil
from pathlib import Path

from invoke import Context
from .job_base import JobBase
        
class ArchiveExtractJob(JobBase):
    archive_path: str
    extract_dir: Path
    
    def __init__(self, archive_path: Path, extract_dir: Path):
        super().__init__()
        self.archive_path = archive_path
        self.extract_dir = extract_dir
        
    def needs_to_run(self, c: Context) -> bool:
        return not self.extract_dir.exists()
    
    def run(self, c: Context):
        if not self.extract_dir.parent.exists():
            os.makedirs(self.extract_dir.parent)
            
        shutil.unpack_archive(self.archive_path, self.extract_dir)