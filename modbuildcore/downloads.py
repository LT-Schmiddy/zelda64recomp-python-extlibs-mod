import pathlib, os, shutil, urllib.request, urllib.parse
from pathlib import Path

_proot: Path = Path(__file__).parent.parent

class DownloadArchiveConfig:
    url: str
    extract_dir: Path
    
    def __init__(self, url: str, extract_dir: Path):
        self.url = url
        self.extract_dir = extract_dir
        
        
class DownloadArchiveHandler:
    entry: DownloadArchiveConfig
    artifact_dir: Path
    cache_path: Path
    
    def __init__(self, entry: DownloadArchiveConfig, artifact_dir: Path):
        self.entry = entry
        self.artifact_dir = artifact_dir
        
        filename = self.get_filename_from_url(self.entry.url)
        self.cache_path = artifact_dir.joinpath(filename)
    
    def get_filename_from_url(self, url: Path) -> Path:
        parsed_url = urllib.parse.urlparse(url)
        return Path(os.path.basename(parsed_url.path))
    
    
    
    def should_download(self) -> bool:
        return not self.cache_path.exists()
    
    def download(self) -> Exception:
        if not self.artifact_dir.exists():
            os.makedirs(self.artifact_dir)
            
        urllib.request.urlretrieve(
            self.entry.url,
            self.cache_path
        )

    def should_extract(self) -> bool:
        return not self.entry.extract_dir.exists()
    
    def extract(self):
        shutil.unpack_archive(self.cache_path, self.entry.extract_dir)
        