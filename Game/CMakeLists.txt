file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

if(NOT GAME_SOURCES)
    message(FATAL_ERROR "No .cpp files found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

add_executable(Game ${GAME_SOURCES} ${GAME_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES ${GAME_SOURCES} ${GAME_HEADERS})

target_include_directories(Game PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Game PRIVATE Engine SDL2::SDL2main)

if(MSVC)
    target_compile_options(Game PRIVATE /W4 /permissive- /MP)
endif()

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:Game>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/images"  "$<TARGET_FILE_DIR:Game>/images"
    COMMENT "Copying shader and image assets")

set_target_properties(Game PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Game>")