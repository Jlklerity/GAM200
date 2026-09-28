cmake_minimum_required(VERSION 3.15)
project(GAM200 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(ImportDependencies)
importDependencies()

add_subdirectory(Engine)
add_subdirectory(Game)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Game)