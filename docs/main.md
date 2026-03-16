# Main Page

Simply include the `repy_api.h` header in your code files, and add `RecompExternalPython:2.0.0`
as a dependency for your mod.

# The Interpreter Stack {#interpreter_stack}

If you prefer not to use the `REPY_FN` macro collection for interacting with the Python interpreter, you can still 

Python has the ability to provides semi-isolated contexts for Python code execution called subinterpreters, and REPY makes this functionality to N64Recompiled mods.

More information on subinterpreters and how they work is provided on the [subinterpreter page](\ref subinterpreters). 