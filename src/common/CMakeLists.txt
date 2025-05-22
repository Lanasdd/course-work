cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

set(TARGET "${PROJECT_COMMON_LIB_NAME}")
custom_add_library_from_current_dir("${TARGET}")
