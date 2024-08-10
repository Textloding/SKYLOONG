file(GLOB_RECURSE CPP_SOURCES "*.cpp")
file(GLOB_RECURSE C_SOURCES "*.c")

idf_component_register(
    SRCS ${CPP_SOURCES} ${C_SOURCES}
    INCLUDE_DIRS "include"
)
 
