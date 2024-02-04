file(GLOB_RECURSE CPP_SOURCES "*.cpp")

idf_component_register(
    SRCS ${CPP_SOURCES} "lv_font_chinese_16.c" "lv_font_big_32.c" "symbol_16.c"
    INCLUDE_DIRS ""
    "include"
)
 
