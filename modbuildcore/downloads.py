import pathlib, os, shutil, urllib.request, urllib.parse
from pathlib import Path

_proot: Path = Path(__file__).parent.parent

default_artifact_download_dir = _proot.joinpath("downloads/artifacts")

class DownloadArchiveEntry:
    url: str
    extract_dir: Path
    
    def __init__(self, url: str, extract_dir: Path):
        self.url = url
        self.extract_dir = extract_dir
        
        
class DownloadArchiveHandler:
    entry: DownloadArchiveEntry
    artifact_dir: Path
    
    def __init__(self, entry: DownloadArchiveEntry, artifact_dir: Path=default_artifact_download_dir):
        self.entry = entry
        self.artifact_dir = artifact_dir
    
    def get_filename_from_url(self, url: Path) -> Path:
        parsed_url = urllib.parse.urlparse(url)
        return Path(os.path.basename(parsed_url.path))
    
    def run(self):
        print(f"Downloading '{self.entry.url}'...")
        
        filename = self.get_filename_from_url(self.entry.url)
        dst_path = self.artifact_dir.joinpath(filename)
        
        if not self.artifact_dir.exists():
            os.makedirs(self.artifact_dir)
            
        try:
            urllib.request.urlretrieve(
                self.entry.url,
                dst_path
            )
        except Exception as e:
            print(f"Error downloading file: {e}")
            return
        
        print(f"Extracting to '{self.entry.extract_dir}'...")
        
        shutil.unpack_archive(dst_path, self.entry.extract_dir)