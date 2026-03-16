# Including Python Modules {#including_python_modules}

Mod code interpreter access and code strings aren't the only ways to run Python code in REPY. You can also write Python modules to include in your mod. This article will go over the essential information related to using Python modules within REPY.

This page only discusses Python's importing machinery as it relates to N64Recompiled and REPY. For a full breakdown of importing modules with Python and how to properly construct Python modules, consult the official Python documentation's page on [the import system](https://docs.python.org/3/reference/import.html).

## Adding Python Module Files to Your `.nrm` {#module_files_in_nrm}

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

## Constructing a Python Module in Memory at Runtime {#in_memory_modules}

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

## Considerations and Caveats {#module_considerations}

### Scoped Imports {#module_scoped_imports}

Because of [Python's module caching](https://docs.python.org/3/reference/import.html#the-module-cache), a Python module is only read from disk and executed the first time it's loaded from an `import` statement. This means that there is no significant penalty for repeatedly importing same module. As an anecdotal example, consider the time it takes to import `zipfile` (a particularly beefy module) for the first time in comparison to the time needed for subsequent imports:

```Python
>>> timeit.timeit("import zipfile", number=1) # This interpreter instance hasn't imported `zipfile` before.
0.020628099999157712
>>> timeit.timeit("import zipfile", number=1) # The module has already been loaded by this interpreter.
3.1999989005271345e-06
```

(Obviously, these times will be different on your machine. The values themselves are not important, just the difference between them.)

This has a practical application for REPY: If you're using `REPY_FN` to include Python code within a mod code function, You don't need to set up a global scope to hold your modules. Running `import` statements within the function itself will not result in significant performance loss. For example, let's say we want to access the current working directory:

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
        "import os\n"
    );
    REPY_FN_CLEANUP;
}
```

Using `REPY_FN_IMPORT` or `REPY_ImportModule` should also work.

### Issues with `RecompModTool` and Including Module Directories Inside `.nrm` Files {#module_directory_issues}

One caveat with this method worth mentioning: The current version of RecompModTool tool REALLY does not like to include additional files at anything other than the root level, which can be an issue if you want to add directory modules instead of single files. On POSIX systems, the tool may simply refuse to add the directory altogether. On Windows, the resultant directories within the `.nrm` will have `\\` in the paths instead of `/`, making the modules unimportable on POSIX systems.

Until such a time as the mod tool is updated to address these issues, the best solution is to not include these folders as `additional_files` in the `mod.toml`, and instead them directly to the `.nrm` after it's been generated by RecompModTool. In fact, this is what REPY itself does for the [`repy_api`](@ref python_api) Python files.

A Python script can work well for this. In addition, `modbuild.py` (a custom building tool/script for N64Recompiled mods) already comes with the ability to inject additional files into an `.nrm` after creation.

`modbuild.py` can be found [in this repository](https://github.com/LT-Schmiddy/n64recomp-mod-custom-and-build-scripting).

### Sharing the Module Search Path {#sharing_module_search_path}

Because, the main Python interpreter is shared by all mods, special care has to be taken by the mod developer to prevent multiple modules from having the same top-level name. A recommended practice when using the main interpreter is to include your mod ID in the names of your modules. Alternatively, you could simply have a root level module directory named with your mod ID, and then include everything else as submodules of that main module (provided you are willing to deal with the technical issues described above).

If these namespace considerations are not an option for your mod, you may want to consider using a [subinterpreter](\ref subinterpreters). See that documentation for more details on how to set that up. The only important thing to mention here is that `REPY_PREINIT_ADD_NRM_TO_SYS_PATH` adds a `.nrm` file to the search path for ALL interpreters, and that behavior is generally not desirable when dealing with subinterpreters. As such, you should exclude `REPY_PREINIT_ADD_NRM_TO_SYS_PATH` from a mod that is meant to primarily use a subinterpreter.

The macro for registering subinterpreters on startup, `REPY_REGISTER_SUBINTERPRETER`, automatically adds your `.nrm` file to the module search path of assigned subinterpreter during the `REPY_ON_CONFIG_SUBINTERPRETERS` event. As such, there is nothing else you need to do for the subinterpreter to import Python modules from your `.nrm`.

### Third-Party Packages {#module_third_party_packages}

REPY does not offer any kind of package management. As such, if you wish to use third-party Python libraries in your mod, you are responsible for including those within your mod's `.nrm` file.

The module search path considerations from the above section apply here. If you're using the main interpreter, you should try to change the name of the module to include your mod's ID or try to inclue the module as a submodule of your own. If neither of these are possible, You should consider using a [subinterpreter](\ref subinterpreters).

Interestingly, this sharing of the module search path means that Python popular Python packages could easily be bundled into standalone `.nrm` files for distribution within the N64Recompiled ecosystem. That way, mods could simply declare these common `.nrm` files as dependencies instead of needing to use one of the solutions described above. Such an `.nrm` would also be free from needing to rename the Python module, as the standard name would be expected. This is not work that REPY itself will be undertaking at this time, but if there are members of the community interested in undertaking this sort of thing, it could be very beneficial.

### Native Extension Modules {#native_extension_modules}

Currently, REPY offers no official support for using native extension modules with Python, outside of the ones that come as part of the standard library.

That's not to say that extension modules will not work. But there are multiple logistical issues related to using and distributing them within the N64Recompiled ecosystem that REPY does not currently have solutions for:

* Native extensions cannot be used from inside a `.nrm`.
* The file extension Python native extensions on Windows is `.pyd` rather than `.dll`, and the mod installation machinery for N64Recompiled games do not have handling for `.pyd` files.
* Native extensions need to be compiled for every platform the mod supports (Admittedly less of an issue, since cross-compiling external libraries using Zig is currently a standard practice in the N64Recompiled community).
* Native extensions generally need to be compiled for a specific minor version of Python. At this time, REPY reserves the right to update to a new minor version of Python whenever it's own minor version updates, due to the fact that REPY relies on some newer Python features that are expected to see performance and feature improvements in newer versions. In fact, an update to Python 3.15 is planned for October, when the first stable release of 3.15 comes out.

It is my intention to provide solutions and otherwise mitigate these issues in future versions, so that native extension modules can be more easily used with REPY. But for the time being, it is recommended to avoid using them.
