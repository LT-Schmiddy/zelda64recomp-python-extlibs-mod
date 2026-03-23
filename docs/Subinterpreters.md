# Subinterpreters {#subinterpreters}

By default, mods that use REPY will run Python code on the main Python interpreter. However, Python supports creating an isolated context for Python code execution in the form of a subinterpreter.

This article only covers information on how to use subinterpreters within REPY. For a more complete discussion of subinterpreters and how they work, consult the official Python documentation:

* [`concurrent.interpreters` — Multiple interpreters in the same process](https://docs.python.org/3/library/concurrent.interpreters.html)
* [Multiple interpreters in a Python process](https://docs.python.org/3/c-api/subinterpreters.html)

## Subinterpreters and REPY C API {#subinterpreters_repy_c}

REPY has API functionality available so that subinterpreters can be easily used within N64Recompiled mod code, leveraging subinterpreter control functions in the Python C API and instead of using Python 3.14's `concurrent.interpreters` module.

Subinterpreter are slow to initialize, and REPY needs to set up additional control structures to manage them efficiently during runtime. As a result, ANY subinterpreter to be made available for use with the REPY C API needs to be declared before REPY initializes. Calling the function `REPY_PreInitRegisterSubinterpreter` during the `REPY_ON_PRE_INIT` event will inform REPY that a mod would like to have a subinterpreter prepared for use. REPY makes a note of this, and returns a `REPY_InterpreterIndex` value that can be used to access the subinterpreter **once REPY has been initialized**. After REPY the main Python interpreter and configures the global control structure for it, each requested subinterpreter will be initialized and have their global control structures created. At this point, the `REPY_ON_CONFIG_INTERPRETER` event will fire, and the subinterpreter will be available use via the index value captured earlier.

## Page Under Construction {#subinterpreters_page_under_construction}

This page is still under construction. At this time, there is currently a significant inefficiency in activating and deactivating subinterpreters through the REPY API, which should be fixed with the release of REPY 2.1.0. This page will be finished at this time. In the meantime, it is recommmended to stick with using the main interpreter for now.
