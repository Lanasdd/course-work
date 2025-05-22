set(TARGET "aco")

set(USE_TBB_IN_TARGET_PROJECT 1)

string(TOUPPER ${TARGET} UPPER_CASE_TARGET)
string(REPLACE "-" "_" UPPER_CASE_TARGET ${UPPER_CASE_TARGET})

custom_add_executable_from_simple_struct(${TARGET})
custom_add_tbb(${TARGET})

find_package(OpenMP REQUIRED)

target_include_directories(${TARGET} PRIVATE "${PROJECT_COMMON_DIR}")
target_link_libraries(${TARGET} "${PROJECT_COMMON_LIB_NAME};OpenMP::OpenMP_CXX")
target_compile_definitions(${TARGET} PRIVATE "USE_${UPPER_CASE_TARGET}") #define для использования в коде

#if(BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
