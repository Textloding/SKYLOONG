file(GLOB_RECURSE CPP_SOURCES "*.cpp")
file(GLOB_RECURSE C_SOURCES "*.c")
list(APPEND C_SOURCES "pet_dog_sprites.c")
list(REMOVE_DUPLICATES C_SOURCES)

idf_component_register(
    SRCS ${CPP_SOURCES} ${C_SOURCES}
    INCLUDE_DIRS "include"
)
