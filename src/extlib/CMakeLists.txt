target_sources(${TARGET_NAME} PUBLIC
    "embed_handler.cpp"
    "lib_main.cpp"
    "pyobject_slotmap.cpp"
    "repy_utils.cpp"
)

add_subdirectory("./embedded_modules")
add_subdirectory("./controllers")