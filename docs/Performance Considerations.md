# Performance Considerations {#repy_performance}

Performance and efficiency is a major goal of REPY, which may be surprising considering Python's reputation as a 'slow' language, and furthermore the official interpreter lags behind many third-party implementations. However, it is more than possible to use REPY without performance issues, even for code that needs to run every frame.

Three factors to consider:

* According to some sources, CPython has actually been [getting faster in recent versions](https://lost.co.nz/articles/sixteen-years-of-python-performance/).
* REPY does its best to keep its own performance light, and tries to avoid expensive interpreter operations unless necessary.
* Because N64Recompiled converts the original native binary to new machine code and doesn't use some kind of interpreted or JITed runtime, recompiled game code runs extremely fast. Furthermore, increased frame rates are handled at render time, with the gameplay code running at the original refresh rate. This means that the amount of time we have to run mod code is extremely generous compared to most other modern games.

That being said, a poorly designed mod can definitely result in serious lag. This page goes over a couple of common pitfalls that can seriously hurt your mod's performance.

## Interpreter Stack Management {#repy_performance_interpreter_stack}

The current implementation of REPY's Interpreter Stack (the mechanism that manages thread and interpreter tracking) only changes the `PyThreadState` whenever the stack is empty, or when the value at the top of the stack changes. There is a potential performance issue with this design, however: Accidentally creating a situation where the current `PyThreadState` is released too often can result in considerable slowdown. In fact, this can often result in `PyThreadState` management actually taking up more CPU time than the actual Pythonic operations that the `PyThreadState` is meant to encapsulate.

This can often happen when encapsulating simple Python operations in short C functions. Consider the following example:

```C
void pyfunc1() {
    REPY_FN_SETUP;
    ...
    REPY_FN_CLEANUP;
}

void pyfunc2() {
    REPY_FN_SETUP;
    ...
    REPY_FN_CLEANUP;
}

void main_func() {
    pyfunc1();
    pyfunc2();
}
```

Remember that `REPY_FN_SETUP` and `REPY_FN_CLEANUP` make calls to `REPY_PushInterpreter` and `REPY_PopInterpreter`, respectively. If we assume that the Interpreter Stack is empty when `main_func` is called, then REPY will activate acquire a `PyThreadState` during `pyfunc`, release it, and then immediately acquire it again during `pyfunc2`. This quite inefficient. While one uncessessary release won't make a noticable difference, lots of reaquirings can add up, particularly if `main_func` is called many times per frame.

The saving grace is that fix here is rather simple: Simply encapsulate your calls to `pyfunc1` and `pyfunc2` between their own `REPY_PushInterpreter` and `REPY_PopInterpreter` calls, like so:

```C
void pyfunc1() {
    REPY_FN_SETUP;
    ...
    REPY_FN_CLEANUP;
}

void pyfunc2() {
    REPY_FN_SETUP;
    ...
    REPY_FN_CLEANUP;
}

void main_func() {
    REPY_PushInterpreter(REPY_MAIN_INTERPRETER);
    pyfunc1();
    pyfunc2();
    REPY_PopInterpreter();
}
```

This ensure that REPY will hold onto the `PyThreadState` for the calls to both `pyfunc1` and `pyfunc2`, only releasing at the call to `REPY_PopInterpreter` in `main_func`. This way, the `PyThreadState` is only ever released once, and the resulting code is MUCH more efficient.

Two additional notes about this concept:

* Again, remember that `REPY_FN` setup and cleanup call `REPY_PushInterpreter` and `REPY_PopInterpreter`. So if `main_func` was made into a function that used `REPY_FN`, this extra push and pop would happen automatically.
* Having other mods installed may mask this issue in your own code, since another mod could have calls to `REPY_PushInterpreter` and `REPY_PopInterpreter` that encapsulate your own. On the other hand, this actually means that running recomp with only your mod and its dependencies is actually the worst case scenario, and so finding instances of this issue should be rather easy.

There are plans to optimize REPY's management of `PyThreadState` in a future update.

## Subinterpreters {#repy_performance_subinterpreters}

Currently, there are some performance issues with how subinterpreters are handled, which significantly exaggerates the thread state release issues from the above section, and similar issues can occur when `REPY_PopInterpreter` merely results in switching interpreters. Until these issues can be fixed (in a future update), subinterpreters should be used sparingly.

## Use Code Caching {#repy_performance_code_caching}

It goes without saying that making the Python interpreter parse the same string of code every time that string needs executing is highly inefficient. In fact, in many cases, the act of parsing the code string can consume more CPU time than the actual execution.

Fortunately, the maxim that "In Python, everything is an object" applies to the parsed executable code as well. By leveraging Python's built-in `compile` method, we hold a obtain a Python `code` object that can be run using `exec` and `eval`. This means that a Python code string can be parsed once and then executed as many times as necessary.

REPY exposes this capability in the C API in many ways, but by far the easiest to use are `REPY_FN_EXEC_CACHE` and the different variants of `REPY_FN_EVAL_CACHE`. These macros parse the Python code string on first execution and store the `code` object as a static variable. That code object can then be used to perform all subsequent executions, which dramatically improves performance in most cases.

In summary, this code block:

```C
REPY_FN_EXEC_CACHE(cache1,
    "some_new_python_variable = old_variable_1 + old_variable_2"
);
```

will be much more performant than this one:

```C
REPY_FN_EXEC_CSTR(
    "some_new_python_variable = old_variable_1 + old_variable_2"
);
```

For further information on preparsing Python code, consider looking at the following functions and macros:

* `REPY_GLOBAL_COMPILE_CACHE`
* `REPY_STATIC_COMPILE_CACHE`
* `REPY_GLOBAL_COMPILE_INCBIN_CACHE`
* `REPY_STATIC_COMPILE_INCBIN_CACHE`
* `REPY_INLINE_COMPILE_CACHE`
* `REPY_INLINE_COMPILE_CACHE_BLOCK`
* `REPY_Compile`
* `REPY_CompileCStr`
* `REPY_CompileCStrN`
* `REPY_Exec`
* `REPY_Eval`
