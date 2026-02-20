target_sources(${TARGET_NAME} PUBLIC
    "controller.cpp"
    "embed_handler.cpp"
    "lib_main.cpp"
    "pyobject_slotmap.cpp"
    "subinterpreter.cpp"
)

add_subdirectory("./embedded_modules")