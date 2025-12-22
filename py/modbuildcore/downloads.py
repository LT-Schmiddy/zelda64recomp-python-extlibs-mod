import pathlib, os, shutil, urllib.request, urllib.parse
from pathlib import Path

from invoke import Context
from .job_base import JobBase

class DownloadJob(JobBase):
    url: str
    download_path: Path
    
    def __init__(self, url: str, download_path: Path, append_url_filename: bool=True):
        super().__init__()
        self.url = url
        self.download_path = download_path
        if (append_url_filename):
            self.download_path = self.download_path.joinpath(self.get_filename_from_url())
        
    def get_filename_from_url(self) -> Path:
        parsed_url = urllib.parse.urlparse(self.url)
        return Path(os.path.basename(parsed_url.path))
    
    # Override:
    def needs_to_run(self, c: Context):
        return not self.download_path.exists()
    
    def run(self, c):
        if not self.download_path.parent.exists():
            os.makedirs(self.download_path.parent)
            
        urllib.request.urlretrieve(
            self.url,
            self.download_path
        )