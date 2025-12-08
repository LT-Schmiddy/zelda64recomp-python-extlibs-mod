import tomllib, pathlib
from pathlib import Path

class ModTomlConfig:
    toml_path: Path
    makefile_path: Path
    
    
    def __init__(self, toml_path: Path, makefile_path: Path):
        self.toml_path = toml_path
        self.makefile_path = makefile_path
    
class ModTomlHandler:
    config: ModTomlConfig
    data: dict
    
    def __init__(self, config: ModTomlConfig):
        self.config = config
        
        self.data = tomllib.loads(self.config.toml_path.read_text())
        
    