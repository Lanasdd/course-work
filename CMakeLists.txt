##Yakovlev Denis 2021.03
cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

option(USE_TBB_IN_TARGET_PROJECT "1")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Select the MSVC runtime library (The allowed values are: MultiThreaded, MultiThreadedDLL, MultiThreadedDebug, MultiThreadedDebugDLL).
# selected MultiThreadedDebugDLL for Debug or RelWithDebInfo config and MultiThreadedDLL to over
if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug,RelWithDebInfo>:Debug>DLL")
endif()

set(TARGET_PROJECT_NAME "Hse" CACHE INTERNAL "TARGET_PROJECT_NAME")
set(TARGET_PROJECT_CXX_STANDARD 23 CACHE INTERNAL "TARGET_PROJECT_CXX_STANDARD")

string(TOUPPER ${TARGET_PROJECT_NAME} TARGET_PROJECT_UPPER_CASE_NAME)
string(REPLACE "-" "_" TARGET_PROJECT_UPPER_CASE_NAME ${TARGET_PROJECT_UPPER_CASE_NAME})
string(TOLOWER ${TARGET_PROJECT_NAME} TARGET_PROJECT_LOWER_CASE_NAME)
string(REPLACE "-" "_" TARGET_PROJECT_LOWER_CASE_NAME ${TARGET_PROJECT_LOWER_CASE_NAME})

set(TARGET_PROJECT_VERSION_MAJOR "0")
set(TARGET_PROJECT_VERSION_MINOR "0")
set(TARGET_PROJECT_VERSION_PATCH "1")
set(TARGET_PROJECT_VERSION
        "${TARGET_PROJECT_VERSION_MAJOR}.${TARGET_PROJECT_VERSION_MINOR}.${TARGET_PROJECT_VERSION_PATCH}"
        CACHE INTERNAL "TARGET_PROJECT_VERSION")

set(PROJECT_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}/src/common" CACHE INTERNAL "PROJECT_COMMON_DIR")
set(PROJECT_COMMON_LIB_NAME "${TARGET_PROJECT_LOWER_CASE_NAME}_common" CACHE INTERNAL "PROJECT_COMMON_LIB_NAME")

project(${TARGET_PROJECT_NAME}
        LANGUAGES C;CXX)

# set BUILD_TESTING=ON,
enable_testing()

# объявления include будут видны в подпроектах, включённых через add_subdirectory
include(cmake/yds_custom_functions.cmake)
include(cmake/yds_add_tbb.cmake)
include(cmake/TBBGet.cmake)
include(cmake/TBBBuild.cmake)

add_subdirectory(src/common)
add_subdirectory(src/apps)
add_subdirectory(src/lessons)
add_subdirectory(src/contests)

#if(BUILD_TESTING)
#    add_subdirectory(tests/apps_test)
#    add_subdirectory(tests/lessons_test)
#endif()
