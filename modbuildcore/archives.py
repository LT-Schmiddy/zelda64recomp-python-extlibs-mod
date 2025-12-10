import pathlib, os, shutil
from pathlib import Path

class ArchiveExtractConfig:
    archive_path: str
    extract_dir: Path
    
    def __init__(self, archive_path: Path, extract_dir: Path):
        self.archive_path = archive_path
        self.extract_dir = extract_dir
        
        
class ArchiveExtractHandler:
    config: ArchiveExtractConfig
    
    def __init__(self, entry: ArchiveExtractConfig):
        self.config = entry

    def should_extract(self) -> bool:
        return not self.config.extract_dir.exists()
    
    def extract(self):
        if not self.config.extract_dir.parent.exists():
            os.makedirs(self.config.extract_dir.parent)
            
        shutil.unpack_archive(self.config.archive_path, self.config.extract_dir)
        