# Basic Usage

Simply include the `repy_api.h` header in your code files, and add `RecompExternalPython:2.0.0`
as a dependency for your mod.

This tutorial assumes you know the basics of the Python programming language.

## Introduction - The REPY Model

RecompExternalPython, or REPY for short, is built for the purpose of being an easier alternative
to compiling and building external shared libraries (often referred to as 'extlibs' in N64Recomp
jargon) for all supported platforms. That particular goal informs everything about REPY's design
and how it functions.

While REPY enables you to include blocks of Python code within recompiled mod code to be
executed (in addition to performing other interpreter interactions) as part of recompiled mod
functions it's important to remember that the Python interpreter still exists on the system
side, and therefore the Python code is, in effect, extlib code. This means that Python has most of
the usual capabilities you would expect from it on the system side, but also shares two fundamental
restrictions that plague traditional extlibs:

* Python code cannot call recompiled functions.
* Python code cannot allocate mod memory.

This is what makes the ability to include Python code and command the Python interpreter from within
your mod's C files so important; It allows us to write a function's flow of execution such that it
weaves seamlessly between mod code and the Python interpreter.

The easiest (and generally recommended) way of using REPY is the `REPY_FN` family of macros. This is
what enable REPY's advertised seamless inclusion of Python code into mod code functions. As a general overview, it works something like this:

* At the top of your function call `REPY_FN_SETUP` or one of its sister macros to secure access to the Python interpreter and create a Python scope to correspond to your mod function's scope. You may not nest uses of these macros within the same function, but one function with a Python scope may call another function with a Python scope without issue.
* Python variables can be set in this scope using `REPY_FN_SET` or one of its sibling macros. Depending on the macro, the value can be a `REPY_Handle` reference to a Python object or an equivalent C-type value.
* Python variables can be obtained from this scope `REPY_FN_GET` or one of its sibling macros. Depending on the macro, the value can be a `REPY_Handle` reference to a Python object or an equivalent C-type value.
* Python code can be executed using `REPY_FN_EXEC` or one of it's sibling macros (for performance reasons, `REPY_FN_EXEC_CACHE` is recommended in most cases). The Python code will have access to any Python variables previously defined in this scope, and any resulting variables will be available in the scope afterword.
* Python expressions can be evaluated using `REPY_FN_EVAL` or one of it's sibling macros (for performance reasons, the variants of `REPY_FN_EVAL_CACHE` are recommended in most cases). The Python code will have access to any Python variables previously defined in this scope. The result of the expression is made available to mod code, and can be a reference to a Python object or an equivalent C-type value depending on the macro.
* Multiple macros are available to create mod code control structures based on the state of the Python scope in a performant way. For example, `REPY_FN_FOREACH_CACHE` will let your mod code iterate through a Python object. Macros for if-else statements, C-style for loops, and while loops are also available.
* Either `REPY_FN_CLEANUP` or `REPY_FN_RETURN` is used to prevent a memory leak by ensuring that the scope is released before the function complete. Any additional references to Python objects created in this scope, such as `REPY_Handle` instances or simply a reference elsewhere in Python code, will be preserved.

To put it in even simpler terms: Say we have a mod code function that uses the `REPY_FN` macros. When that
function is called, a corresponding scope is created within the Python interpreter for that function's
exclusive use, and will exist for the lifetime of the function call. Using that Python scope, we can run
Python code and operate on Python objects to do things that would normally require a dedicated extlib.

## Anatomy of a Python Scope

The macro `REPY_FN_SETUP` creates what this documentation will refer to as a Python execution scope, which
requires some explaining:

Python does not use the block scoping like C and most C-derived languages. Instead, it uses
functional scoping, and therefore, there are really only two levels of scope to worry about in
most cases: the global scope and the local scope. Within a (top-level) function, either in the main
file of a Python program or in one of the modules, the global and local scopes are different, with
all variable assignments going into the local scope by default. However, for code executing at the
top level of the module, these scopes are actually the same. Note that I don't say "There is no local
scope", because that would be misleading. Internally, Python uses `dict` objects to represent the
global and local scopes, and at the module level these are actually the same object, and attempting
to access the local scope dictionary directly will return the same `dict` object as accessing the
global scope dictionary. Within the Python interpreter, `dict` objects can be used to pre-define
variable to be used when calling the `exec` and `eval` functions. In fact, this is exactly how REPY
handles Python code execution internally.

So, when we say that `REPY_FN_SETUP` creates an execution scope, what we really mean is that this
macro creates two variables in your mod code function, one for the global scope and one for the local
scope. In the case of `REPY_FN_SETUP`, the same `dict` is used for the global and local scopes, similar to module-level code. However, there are alternate setup functions that allow for use of an externally defined global scope `dict`.

## A Simple Example with `REPY_FN` - Using Inline Code to Read a File

Let's explore all this by looking at a simple example: reading a file into mod memory.

```C
#include "repy_api.h"

const char* read_a_file(const char* file_path) {
    REPY_FN_SETUP;
    REPY_FN_SET_STR("py_file_path", file_path);
    REPY_FN_EXEC_CACHE(readfile_exec1,
        "file = open(py_file_path, 'r')\n"
        "file_text = file.read()\n"
        "file.close()\n"
    );
    const char* retVal = REPY_FN_GET_STR("file_text");
    REPY_FN_CLEANUP;
    return retVal;
}
```

This is a very straight-forward function with no branching paths. Let's walk through it.

* `REPY_FN_SETUP` is invoked to create the Python scope.
* We give that Python scope a variable named `py_file_path`, and set it using the `file_path` argument (The Python variable is given a different name from the C variable for clarity, but there's no reason they can't be the same). This variant of the variable setting macro, `REPY_FN_SET_STR`, automatically converts a null-terminated string in mod code into a Python `str` object. Note that this is a copy operation, as the C-string is being read into the interpreter. Changes to `file_path` will not result in changes to `py_file_path`.
* Next, we execute a block of Python code. The code itself is pretty self-explanatory, simply reading text from a file using the previously set `py_file_path` variable. Note the use of `REPY_FN_EXEC_CACHE` instead of `REPY_FN_EXEC`. This macro compiles the code string into CPython bytecode using Python's built-in `compile` function, and stores a handle to it in a `static` variable within the mod function (this is why the additional identifier name is required by the macro before the code string). This way, the code string only has to be parsed the first time the function is run, saving time on repeated function calls. It also enables us to manually set the source location of the resulting bytecode, which is useful for debugging.
* We copy the text that the Python code produced into mod memory, using the variable name set in the Python code block. Note that the `REPY_FN_GET_STR` macro, and the `REPY_CastStr` function that underlies it, automatically allocates space for the string in mod memory using `recomp_alloc`. This is an advantage of REPY and calling this transfer from the mod code side,  as it eliminates the awkward back and forth between mod and native code that copying data from an extlib to mod memory normally requires.
* Lastly, the Python scope is cleaned up, and the C-string with the file content is returned. These two lines could be consolidated into a single `REPY_FN_RETURN` invokation. However, we're doing them seperately in this example for the same of clarity.
