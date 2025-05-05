cmake_minimum_required(VERSION 3.27)
project(clay_frontend C)
set(CMAKE_C_STANDARD 99)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    mongoose
    GIT_REPOSITORY "https://github.com/cesanta/mongoose.git"
    GIT_TAG "7.13"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib mongoose)

add_library(mongoose_lib STATIC
    ${mongoose_SOURCE_DIR}/mongoose.c
)

target_compile_options(mongoose_lib PRIVATE
    -DMG_HTTP_DIRLIST_TIME_FMT="%Y/%m/%d %H:%M:%S"
    -DMG_ENABLE_LINES=0
    -DMG_ENABLE_IPV6=1
    -DMG_ENABLE_SSI=1
    -DMG_IO_SIZE=8192
    -DMG_ENABLE_DIRECTORY_LISTING=0
    -DMG_ENABLE_FILESYSTEM=0
    -DMG_TLS=MG_TLS_BUILTIN
)

target_include_directories(mongoose_lib PUBLIC ${mongoose_SOURCE_DIR})

include(ExternalProject)
ExternalProject_Add(cjson_project
    GIT_REPOSITORY "https://github.com/DaveGamble/cJSON.git"
    GIT_TAG "v1.7.17"
    CMAKE_ARGS 
        -DBUILD_SHARED_LIBS=OFF 
        -DENABLE_CUSTOM_UNINSTALL=OFF
        -DENABLE_CJSON_TEST=OFF
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

ExternalProject_Get_Property(cjson_project SOURCE_DIR BINARY_DIR)

add_library(cjson STATIC IMPORTED)
set_property(TARGET cjson PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libcjson.a)
add_dependencies(cjson cjson_project)

target_include_directories(cjson INTERFACE ${SOURCE_DIR})

add_executable(clay_frontend main.c)

target_compile_options(clay_frontend PUBLIC)
target_include_directories(clay_frontend PUBLIC .)
target_include_directories(clay_frontend PUBLIC ${SOURCE_DIR})

target_link_libraries(clay_frontend PUBLIC raylib mongoose_lib cjson pthread sqlite3 m)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

add_custom_command(
        TARGET clay_frontend POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources)