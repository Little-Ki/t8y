cmake_minimum_required(VERSION 3.1)

project(t8y)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

set(EXECUTABLE_NAME ${PROJECT_NAME})

if (ANDROID)
	set(EXECUTABLE_NAME main)
	add_library(${EXECUTABLE_NAME} SHARED)
else()
	add_executable(${EXECUTABLE_NAME})
endif()

if(APPLE)
    target_sources("${EXECUTABLE_NAME}" PRIVATE "asset/logo.png")
endif()

if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
	set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

add_compile_options(/source-charset:utf-8 /execution-charset:utf-8)


include_directories(${PROJECT_SOURCE_DIR}/third_party)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/text_editor)
file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
# aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

target_sources(${EXECUTABLE_NAME} 
PRIVATE 
    main.cpp
    asset/iosLaunchScreen.storyboard
    asset/t8y.appxManifest
    ${SRC_LIST}
)

target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_20)

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/Lua EXCLUDE_FROM_ALL)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3::SDL3 lua_static)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES 
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/asset/Info.plist.in"

    # in Xcode, create a Scheme in the schemes dropdown for the app.
    XCODE_GENERATE_SCHEME TRUE
    # Identification for Xcode
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.ravbug.sdl3-sample"
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.ravbug.sdl3-sample"
	XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")
