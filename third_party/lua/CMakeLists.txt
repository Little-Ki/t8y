cmake_minimum_required(VERSION 3.1...3.12)
cmake_policy(SET CMP0077 NEW)
project(lua LANGUAGES C)

enable_language(CXX)

set(LUA_LIB_SRCS 
    "src/lapi.c"
    "src/lcode.c"
    "src/lctype.c"
    "src/ldebug.c"
    "src/ldo.c"
    "src/ldump.c"
    "src/lfunc.c"
    "src/lgc.c"
    "src/llex.c"
    "src/lmem.c"
    "src/lobject.c"
    "src/lopcodes.c"
    "src/lparser.c"
    "src/lstate.c"
    "src/lstring.c"
    "src/ltable.c"
    "src/ltm.c"
    "src/lundump.c"
    "src/lvm.c"
    "src/lzio.c"
    "src/lauxlib.c"
    "src/lbaselib.c"
    "src/lcorolib.c"
    "src/ldblib.c"
    "src/liolib.c"
    "src/lmathlib.c"
    "src/loadlib.c"
    "src/loslib.c"
    "src/lstrlib.c"
    "src/ltablib.c"
    "src/lutf8lib.c"
    "src/linit.c"
    "src/lua.c"
)

set(LUA_LINKED_LIBRARIES)

add_library(lua_internal INTERFACE)

add_library(lua_include INTERFACE)

target_include_directories(lua_include INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lua_internal INTERFACE lua_include)

add_library(lua_static STATIC ${LUA_LIB_SRCS})
target_link_libraries(lua_static PRIVATE lua_internal PUBLIC lua_include)
set_target_properties(lua_static PROPERTIES
    VERSION "${PACKAGE_VERSION}"
)
list(APPEND TARGETS_TO_INSTALL lua_static)
if(NOT BUILD_SHARED_LIBS OR NOT LUA_ENABLE_SHARED)
    add_library(Lua::Library ALIAS lua_static)
endif()
