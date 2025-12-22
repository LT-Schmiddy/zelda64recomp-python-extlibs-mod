from invoke import Context
from pathlib import Path

class JobBase:
    _has_been_resolved: bool
    no_duplication: bool
    dependencies: list[JobBase]
    mod_output_files: dict[Path, Path]
    
    def __init__(self):
        self._has_been_resolved = False
        self.no_duplication = True
        self.dependencies = []
        self.mod_output_files = {}
    
    # Overridable Functions:
    def needs_to_run(self, c: Context) -> bool:
        return True
    
    def run(self, c: Context):
        pass
    
    # Not to override:
    def depends_on(self, new_dependencies: list[JobBase]) -> JobBase:
        self.dependencies.extend(new_dependencies)
        return self
    
    def resolve(self, c: Context, run_dependencies: bool = True):            
        if self.no_duplication and self._has_been_resolved:
            return
        
        if self.needs_to_run(c):
            if run_dependencies:
                for i in self.dependencies:
                    i.resolve(c)
            self.run(c)
        
        self._has_been_resolved = True
        
    def get_recursive_mod_outputs(self) -> dict[Path, Path]:
        retVal = self.mod_output_files
        
        for i in self.dependencies:
            retVal.update(i.get_recursive_mod_outputs())
        
        return retVal