# Including Python Modules {#including_python_modules}

Mod code interpreter access and code strings aren't the only ways to run Python code in REPY. You can also write Python modules to include in your mod. This article will go over the essential information related to using Python modules within REPY.

## Adding Python Module Files to Your `.nrm`

The easiest way to include Python modules in an N64Recompiled mod is to include them within the nrm itself as additional files. The Python interpreter has the built-in ability to import modules from within zip files, and `.nrm` files are just .zip files with a specific layout.

Lets say we have and example Python module file named `module_example.py` somewhere in our project with this source code:

```Python
def function_example():
    print("function_example")
```

If we want this module to be importable from within REPY, we need to to two things:

1) Add the mod file to your `mod.toml` under the additional files section.
2) Add the line `REPY_PREINIT_ADD_NRM_TO_SYS_PATH;` at the top level of one of your mod source files. This will instruct REPY to add this `.nrm` to Python's module search path when initializing the interpreter.

This will look like the following:

-> `mod.toml`:

```Toml
additional_files = [
    "path/to/module_example.py" 
]
```

-> `mod_main.c`:

```C
#include "repy_api.h"
REPY_PREINIT_ADD_NRM_TO_SYS_PATH;
```

Once we've done that, we will be able to import `module_example.py` like so:

```Python
import module_example
```

## Constructing a Python Module in Memory at Runtime

Generally, this technique isn't recommended for most cases, but it does have some niche use. For instance, it can be a good way to store global data needed by both inline Python code and other Python files.

The `repy_api.h` header includes two functions to construct modules in memory: `REPY_ConstructModuleFromCStr` and `REPY_ConstructModuleFromCStrN`. Look at the documentations for their usage. What's important is that these functions both return a handle for a newly created Python module, created from a provided code string. Additional members and data can also be added to the module using `REPY_SetAttr` and `REPY_SetAttrCStr`. That module can then (optionally) be made importable through Python `import` statements.

Here's a simple example:

```C
#include "repy_api.h"
#include "recompconfig.h"
#include "recomputils.h"

REPY_Handle example_module = REPY_NO_OBJECT;

REPY_ON_POST_INIT void init_module() {
    example_module = REPY_ConstructModuleFromCStr(
        "example_module", // Module name.
        "some_initial_value = 'Hello Recomp'", // Module source code. Can be blank.
        true // This argument makes it so that this module is now available via `import example_module`.
    );
    char* mod_dir_path = recomp_get_mod_folder_path();
    // This will add `mod_folder_path` as a member of the module.
    REPY_SetAttrCStr(example_module, "mod_folder_path", REPY_CreateStr_SUH(mod_dir_path));
    recomp_free(mod_dir_path);

}

```

## Considerations and Caveats

### Scoped Imports

Technically, Python modules are only ever loaded by an interpreter instance one time. To be more accurate, a Python module is only read from disk and executed the first time it's loaded from an `import` statement, and the resulting module object is then added to the `sys.modules` dictionary. All subsequent `import` statements for that module simply bring the stored object into the current scope. This means that there is no real penalty for repeatedly importing same module. As an anecdotal example, consider the time it takes to import `zipfile` for the first time in comparison to the time needed for subsequent imports:

```Python
>>> timeit.timeit("import zipfile", number=1) # This interpreter instance hasn't imported `zipfile` before.
0.020628099999157712
>>> timeit.timeit("import zipfile", number=1) # The module has already been loaded by this interpreter.
3.1999989005271345e-06
```

(Obviously, these times will be different on your machine. The values themselves are not important, just the relationship between them.)

This has a practical application for REPY: If you're using `REPY_FN` to include Python code within a mod code function, You don't need to set up a global scope to hold your modules. It's perfectly fine to write `import` statements within the function itself. For example, let's say we want to access the current working directory:

```C
char* some_function() {
    REPY_FN_SETUP;
    REPY_FN_EXEC_CACHE(get_os_cwd,
        "import os\n"
        "cwd = os.getcwd()\n"
    );
    REPY_FN_RETURN(char*, REPY_FN_GET_STR("cwd"));
}
```

This is a perfectly fine design pattern for accessing a member of the `os` module. The Python interpreter will only need to load `os` the first time this function is run (assuming that `os` wasn't imported somewhere else first). Subsequent calls to this function will only to bring the already loaded `os` module into the current scope.

That being said, sometimes that initial loading of modules can take a significant amount of time. If you want to ensure a module is already loaded the first time it's needed, one solution is to simply import it during the `REPY_ON_POST_INIT` event, like so.

```C
REPY_ON_POST_INIT void preload_modules() {
    REPY_FN_SETUP;
    REPY_FN_EXEC_CACHE(get_os_cwd,
        "import os, zipfile\n"
    );
    REPY_FN_CLEANUP;
}
```

Using `REPY_FN_IMPORT` or `REPY_ImportModule` should also work.

### Issues with Including Module Directories Inside `.nrm` Files

One caveat with this method worth mentioning: The current version of RecompModTool tool REALLY does not like to include additional files at anything other than the root level, which can be an issue if you want to add directory modules instead of single files. On POSIX systems, the tool may simply refuse to add the directory altogether. On Windows, the resultant directories within the `.nrm` will have `\\` in the paths instead of `/`, making the modules unimportable on POSIX systems.

Until such a time as the mod tool is updated to address these issues, the best solution is to simply add these files to your `.nrm` after it's been generated by RecompModTool. In fact, this is what REPY itself does for the [`repy_api`](@ref python_api) Python files.

A Python script can work well for this. In addition, `modbuild.py` (a custom building tool/script for N64Recompiled mods) has the ability to add additional files to `.nrm` files after creation already built in.

`modbuild.py` can be found [in this repository](https://github.com/LT-Schmiddy/n64recomp-mod-custom-and-build-scripting).

### Sharing the Module Search Path

...

### Third-Party Packages

...

### Native Extension Modules

...
