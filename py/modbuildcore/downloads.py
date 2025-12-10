import pathlib, os, shutil, urllib.request, urllib.parse
from pathlib import Path

class DownloadConfig:
    url: str
    download_path: Path
    
    def __init__(self, url: str, download_path: Path, append_url_filename: bool=True):
        self.url = url
        self.download_path = download_path
        if (append_url_filename):
            self.download_path = self.download_path.joinpath(self.get_filename_from_url())
        
    def get_filename_from_url(self) -> Path:
        parsed_url = urllib.parse.urlparse(self.url)
        return Path(os.path.basename(parsed_url.path))
    
    
class DownloadHandler:
    config: DownloadConfig
    
    def __init__(self, entry: DownloadConfig):
        self.config = entry
    
        
    def should_download(self) -> bool:
        return not self.config.download_path.exists()
    
    def download(self) -> Exception:
        if not self.config.download_path.parent.exists():
            os.makedirs(self.config.download_path.parent)
            
        urllib.request.urlretrieve(
            self.config.url,
            self.config.download_path
        )