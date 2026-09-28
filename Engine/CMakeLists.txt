file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
    
if(NOT ENGINE_SOURCES)
    message(FATAL_ERROR "No .cpp files found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

add_library(Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_features(Engine PUBLIC cxx_std_20)
if(WIN32)
    target_compile_definitions(Engine PUBLIC PLATFORM_WINDOWS)
endif()

# PUBLIC so Game can include SDL/glm/glad headers through Engine's headers
target_link_libraries(Engine PUBLIC SDL2::SDL2-static glad_gles2 glm::glm stb::stb)

if(MSVC)
    target_compile_options(Engine PRIVATE /W4 /permissive- /MP)
endif()