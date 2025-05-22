
function(configure_target_profile TARGET)
    string(TOLOWER ${CMAKE_CXX_COMPILER_ID} compiler_id)

    if(compiler_id STREQUAL "msvc")
        target_compile_definitions(${TARGET} PRIVATE "HELPER_CXX_COMPILER_IS_MSVC") #define для использования в коде
    elseif(compiler_id STREQUAL "gnu")
        target_compile_definitions(${TARGET} PRIVATE "HELPER_CXX_COMPILER_IS_GCC") #define для использования в коде
    elseif (compiler_id MATCHES "clang")
        target_compile_definitions(${TARGET} PRIVATE "HELPER_CXX_COMPILER_IS_CLANG") #define для использования в коде
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            target_compile_definitions(${TARGET} PRIVATE "HELPER_CXX_COMPILER_FRONTEND_VARIANT_IS_MSVC") #define для использования в коде
        elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
            target_compile_definitions(${TARGET} PRIVATE "HELPER_CXX_COMPILER_FRONTEND_VARIANT_IS_GNU") #define для использования в коде
        endif()
    endif()

    if(NOT SET_UP_CONFIGURATIONS_DONE)
        set(SET_UP_CONFIGURATIONS_DONE TRUE)
        get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
        if(is_multi_config)
            if(NOT "ASan" IN_LIST CMAKE_CONFIGURATION_TYPES)
                list(APPEND CMAKE_CONFIGURATION_TYPES ASan)
            endif()
            if(NOT "TSan" IN_LIST CMAKE_CONFIGURATION_TYPES)
                list(APPEND CMAKE_CONFIGURATION_TYPES TSan)
            endif()
            if(NOT "MSan" IN_LIST CMAKE_CONFIGURATION_TYPES)
                list(APPEND CMAKE_CONFIGURATION_TYPES MSan)
            endif()
            if(NOT "ProfileAndCoverage" IN_LIST CMAKE_CONFIGURATION_TYPES)
                list(APPEND CMAKE_CONFIGURATION_TYPES ProfileAndCoverage)
            endif()
        else()
            set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel;ASan;TSan;MSan;ProfileAndCoverage" CACHE STRING "" FORCE)

            set(allowedBuildTypes Debug Release RelWithDebInfo MinSizeRel ASan TSan MSan ProfileAndCoverage)
            set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

            if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
                message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
            endif()
        endif()
    endif() #SET_UP_CONFIGURATIONS_DONE

    #warnings flags - for all profiles
    if(compiler_id STREQUAL "msvc")
        target_compile_options(${TARGET} PRIVATE "/Wall;/W3")
    elseif(compiler_id STREQUAL "gnu")
        target_compile_options(${TARGET} PRIVATE "-Wall;-Wextra;-Wabi=15;-Wpedantic;-Wshadow;-Wformat=2;-Wfloat-equal;-Wconversion;-Wlogical-op;-Wshift-overflow=2;-Wduplicated-cond;-Wcast-qual;-Wcast-align;-Werror=reorder")
    elseif (compiler_id MATCHES "clang")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            target_compile_options(${TARGET} PRIVATE "-Wall;-WX;-Wno-c++98-compat;-Wno-c++98-compat-extra-semi;-Wno-unsafe-buffer-usage")
        elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
            target_compile_options(${TARGET} PRIVATE "-Wall;-Wextra;-Wpedantic;-Wshadow;-Wformat=2;-Wfloat-equal;-Wconversion;-Wlong-long;-Wshift-overflow;-Wcast-qual;-Wcast-align;-Werror=reorder")
        endif()
    endif()

    # optimizations flags
    if(compiler_id STREQUAL "msvc")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Debug>:/Od;/GL-;/ZI>")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Release>:/O2;/GL>")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RelWithDebInfo>:/O2;/GL;/Zi>")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:MinSizeRel>:/O1;/GL>")
        target_compile_options(${TARGET} PRIVATE "$<$<OR:$<CONFIG:ASan>,$<CONFIG:TSan>,$<CONFIG:MSan>>:/O2;/GL;/Zi>")
    elseif(compiler_id STREQUAL "gnu")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Debug>:-O0;-g>")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Release>:-O3>")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RelWithDebInfo>:-O3;-g>")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:MinSizeRel>:-Os>")
        target_compile_options(${TARGET} PRIVATE "$<$<OR:$<CONFIG:ASan>,$<CONFIG:TSan>,$<CONFIG:MSan>>:-O1;-g>")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:ProfileAndCoverage>:-O0;-g>")
     elseif(compiler_id MATCHES "clang")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Debug>:-g>")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Release>:-O3>")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RelWithDebInfo>:-O3;-g>")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:MinSizeRel>:-Oz>")
            target_compile_options(${TARGET} PRIVATE "$<$<OR:$<CONFIG:ASan>,$<CONFIG:TSan>,$<CONFIG:MSan>>:-O1;-g>")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:ProfileAndCoverage>:-g>")
        else()
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Debug>:-O0;-g>")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Release>:-O3>")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RelWithDebInfo>:-O3;-g>")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:MinSizeRel>:-Oz>")
            target_compile_options(${TARGET} PRIVATE "$<$<OR:$<CONFIG:ASan>,$<CONFIG:TSan>,$<CONFIG:MSan>>:-O1;-g>")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:ProfileAndCoverage>:-O0;-g>")
        endif()
    endif()

    #definitions
    if(compiler_id STREQUAL "msvc")
        target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:_DEBUG>")
    elseif(compiler_id STREQUAL "gnu")
        target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:_GLIBCXX_DEBUG>")
    elseif(compiler_id MATCHES "clang")#|appleclang")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:_DEBUG>")
        elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
            target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:_LIBCPP_DEBUG=1;_GLIBCXX_DEBUG;_GLIBCXX_DEBUG_PEDANTIC>")
        endif()
    endif()

    #sanitize
    if(compiler_id STREQUAL "msvc")
        set (ASAN_VCASAN_DEBUGGING 1)
        target_compile_options(${TARGET} PRIVATE "$<$<OR:$<CONFIG:ASan>,$<CONFIG:TSan>,$<CONFIG:MSan>>:/fsanitize=address;/fsanitize=fuzzer;/fsanitize-address-use-after-return;/fno-sanitize-address-vcasan-lib>")
    elseif(compiler_id STREQUAL "gnu" OR compiler_id MATCHES "clang")
        ### address-sanitizer

        #target_compile_definitions(${TARGET} PRIVATE "$<$<OR:$<CONFIG:ASan>,$<CONFIG:TSan>,$<CONFIG:MSan>>:USE_ADDRESS_SANITIZER>")
        #set(CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:detect_invalid_pointer_pairs=1:detect_leaks=1 ${CMAKE_CXX_COMPILER_LAUNCHER})

        if(compiler_id STREQUAL "gnu")
             #set(ALSAN_COMP_FLAGS "-fsanitize=address;-fsanitize-address-use-after-scope;-fno-omit-frame-pointer;-fno-optimize-sibling-calls;-fno-sanitize-recover=all;-fsanitize=float-divide-by-zero;-fsanitize=float-cast-overflow;-fno-sanitize=null;-fno-sanitize=alignment")
             set(ALSAN_COMP_FLAGS "-fsanitize=address;-fsanitize-address-use-after-scope;-fno-omit-frame-pointer;-fno-optimize-sibling-calls;-fno-sanitize-recover=all;-fsanitize=float-divide-by-zero;-fsanitize=float-cast-overflow;-fno-sanitize=null;-fno-sanitize=alignment;-fsanitize=pointer-compare;-fsanitize=pointer-subtract")
        elseif(compiler_id MATCHES "clang")#|appleclang")
            if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
                set(ALSAN_COMP_FLAGS "-fsanitize=address;-fsanitize-address-use-after-scope;-fno-sanitize-recover=all;-fsanitize=float-divide-by-zero;-fsanitize=float-cast-overflow;-fno-sanitize=null;-fno-sanitize=alignment")
            elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
                 set(ALSAN_COMP_FLAGS "-fsanitize=address;-fsanitize-address-use-after-scope;-fno-omit-frame-pointer;-mno-omit-leaf-frame-pointer;-fno-optimize-sibling-calls;-fno-sanitize-recover=all;-fsanitize=float-divide-by-zero;-fsanitize=float-cast-overflow;-fno-sanitize=null;-fno-sanitize=alignment")
            endif()
        endif() 

        set(ALSAN_LINK_FLAGS "-fsanitize=address;-fsanitize=leak")

        ### undefined-sanitizer

        if(compiler_id STREQUAL "gnu")
            set(UBSAN_COMP_FLAGS "-fsanitize=undefined;-fsanitize=shift;-fsanitize=shift-exponent;-fsanitize=shift-base;-fsanitize=integer-divide-by-zero;-fsanitize=unreachable;-fsanitize=vla-bound;-fsanitize=null;-fsanitize=return;-fsanitize=signed-integer-overflow;-fsanitize=bounds;-fsanitize=bounds-strict;-fsanitize=alignment;-fsanitize=object-size;-fsanitize=float-divide-by-zero;-fsanitize=float-cast-overflow;-fsanitize=nonnull-attribute;-fsanitize=returns-nonnull-attribute;-fsanitize=bool;-fsanitize=enum;-fsanitize=vptr;-fsanitize=pointer-overflow;-fsanitize=builtin")
            set(UBSAN_LINK_FLAGS "-fsanitize=undefined")
        else()
            set(UBSAN_COMP_FLAGS "-fsanitize=undefined;-fsanitize=integer;-fsanitize=nullability")
            set(UBSAN_LINK_FLAGS "-fsanitize=undefined;-fsanitize=integer;-fsanitize=nullability")
        endif()

        ### thread-sanitizer

        if(compiler_id STREQUAL "gnu")
            set(TSAN_COMP_FLAGS "-fsanitize=thread;-fPIE;-pie;-fno-omit-frame-pointer;-fno-optimize-sibling-calls")
        else()
            set(TSAN_COMP_FLAGS "-fsanitize=thread;-fPIE;-fno-omit-frame-pointer;-fno-optimize-sibling-calls")
        endif()

        set(TSAN_LINK_FLAGS "-fsanitize=thread")

        ### memory-sanitizer

        set(MSAN_COMP_FLAGS "-fsanitize=memory;-fsanitize-memory-track-origins=2;-fPIE;-fno-omit-frame-pointer;-fno-optimize-sibling-calls")
        set(MSAN_LINK_FLAGS "-fsanitize=memory")

        ### add sanitizers options
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:ASan>:${ALSAN_COMP_FLAGS};${UBSAN_COMP_FLAGS}>")

        if((compiler_id MATCHES "clang") AND (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
            ## https://learn.microsoft.com/ru-ru/cpp/sanitizers/asan-runtime?view=msvc-170
            ## https://devblogs.microsoft.com/cppblog/msvc-address-sanitizer-one-dll-for-all-runtime-configurations/
           # find_library(NAMES clang_rt.asan_dynamic-x86_64.lib
           # PATHS "$ENV{ProgramFiles)}/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64"
            #"$ENV{ProgramFiles)}/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.39.33519/lib/x64")
            #target_link_directories(${TARGET} PRIVATE "$ENV{ProgramFiles)}/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64")
            if (CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "DLL")
                target_link_libraries(${TARGET} PRIVATE "$<$<CONFIG:ASan>:clang_rt.asan_dynamic-x86_64;clang_rt.asan_dynamic_runtime_thunk-x86_64;clang_rt.ubsan_standalone-x86_64>")
                target_link_options(${TARGET} PRIVATE "$<$<CONFIG:ASan>:${ALSAN_LINK_FLAGS};${UBSAN_LINK_FLAGS} /wholearchive:clang_rt.asan_dynamic_runtime_thunk-x86_64.lib>")
            else()            
                target_link_libraries(${TARGET} PRIVATE "$<$<CONFIG:ASan>:clang_rt.asan_dynamic-x86_64;clang_rt.asan_static_runtime_thunk-x86_64;clang_rt.ubsan_standalone-x86_64>")
                target_link_options(${TARGET} PRIVATE "$<$<CONFIG:ASan>:${ALSAN_LINK_FLAGS};${UBSAN_LINK_FLAGS} /wholearchive:clang_rt.asan_static_runtime_thunk-x86_64.lib>")                
            endif()
        else()
            target_link_options(${TARGET} PRIVATE "$<$<CONFIG:ASan>:${ALSAN_LINK_FLAGS};${UBSAN_LINK_FLAGS}>")
        endif()

        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:TSan>:${TSAN_COMP_FLAGS};${UBSAN_COMP_FLAGS}>")
        target_link_options(${TARGET} PRIVATE "$<$<CONFIG:TSan>:${TSAN_COMP_FLAGS};${UBSAN_LINK_FLAGS}>")

        if(compiler_id MATCHES "clang")
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:MSan>:${MSAN_COMP_FLAGS};${UBSAN_COMP_FLAGS}>")
            target_link_options(${TARGET} PRIVATE "$<$<CONFIG:MSan>:${MSAN_LINK_FLAGS};${UBSAN_LINK_FLAGS}>")
        endif()

        set(ENV{ASAN_SYMBOLIZER_PATH} /usr/bin/llvm-symbolizer)
        set(ENV{MSAN_SYMBOLIZER_PATH} /usr/bin/llvm-symbolizer)
        set(ENV{TSAN_SYMBOLIZER_PATH} /usr/bin/llvm-symbolizer)
        set(ENV{LSAN_SYMBOLIZER_PATH} /usr/bin/llvm-symbolizer)
    endif()

    #Profiler - Perf (on Linux), DTrace (on macOS), Performance Tools for Visual Studio Profiler utility VSInstr.exe (on Windows) profile-and-coverage ProfileAndCoverage
    if(compiler_id STREQUAL "msvc")
        target_link_options(${TARGET} PRIVATE "$<$<CONFIG:ProfileAndCoverage>:/PROFILE>")
    elseif(compiler_id STREQUAL "gnu")
        set(COVERAGE_COMP_FLAGS "--coverage;-fprofile-abs-path") # --coverage is a synonym for -fprofile-arcs -ftest-coverage (when compiling) and -lgcov (when linking)
        set(COVERAGE_LINK_FLAGS "--coverage") # --coverage is a synonym for -fprofile-arcs -ftest-coverage (when compiling) and -lgcov (when linking)
        set(PROFILER_COMP_FLAGS "-fno-omit-frame-pointer")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:ProfileAndCoverage>:${PROFILER_COMP_FLAGS};${COVERAGE_COMP_FLAGS}>")
        target_link_options(${TARGET} PRIVATE "$<$<CONFIG:ProfileAndCoverage>:${COVERAGE_LINK_FLAGS}>")
    elseif(compiler_id MATCHES "clang")
        set(COVERAGE_COMP_FLAGS "--coverage;-fprofile-instr-generate;-fcoverage-mapping")
        set(COVERAGE_LINK_FLAGS "--coverage")
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:ProfileAndCoverage>:${PROFILER_COMP_FLAGS};${COVERAGE_COMP_FLAGS}>")
        target_link_options(${TARGET} PRIVATE "$<$<CONFIG:ProfileAndCoverage>:${COVERAGE_LINK_FLAGS}>")
    endif()

    #warnings flags - for all profiles
    if(compiler_id STREQUAL "msvc")
        target_compile_options(${TARGET} PRIVATE "/permissive-")
        #elseif(compiler_id STREQUAL "gnu" OR compiler_id MATCHES "clang")
        #    target_compile_options(${TARGET} PRIVATE " -fpermissive")
    endif()

endfunction(configure_target_profile)

### add executable target
function(custom_add_executable_ex TARGET TARGET_SRCS TARGET_INCLUDE TARGET_VERSION TARGET_CXX_STANDARD)
    IF(WIN32)
        set_source_files_properties("${TARGET_SRCS}" PROPERTIES LANGUAGE CXX)
    ENDIF(WIN32)

    add_executable(${TARGET} "${TARGET_SRCS}")

    set_target_properties(${TARGET} PROPERTIES
            OUTPUT_NAME "${TARGET}"
            VERSION "${TARGET_VERSION}"
            CXX_STANDARD ${TARGET_CXX_STANDARD}
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
            )

    target_include_directories(${TARGET} PRIVATE "${TARGET_INCLUDE}")

    configure_target_profile(${TARGET})
endfunction()

function(custom_add_executable_from_current_dir_with_default_name RETURN_TARGET)
    get_filename_component(TARGET "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    custom_add_executable_from_current_dir(${TARGET})
    set(${RETURN_TARGET} ${TARGET} PARENT_SCOPE)
endfunction()

function(custom_add_executable_from_current_dir TARGET)
    file(GLOB_RECURSE TARGET_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

    custom_add_executable_ex("${TARGET}" "${TARGET_SRCS}" "${CMAKE_CURRENT_SOURCE_DIR}" ${TARGET_PROJECT_VERSION}
            ${TARGET_PROJECT_CXX_STANDARD})
endfunction()

function(custom_add_executable_from_simple_struct TARGET)
    file(GLOB_RECURSE TARGET_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

    custom_add_executable_ex("${TARGET}" "${TARGET_SRCS}" "${CMAKE_CURRENT_SOURCE_DIR}/include" ${TARGET_PROJECT_VERSION}
            ${TARGET_PROJECT_CXX_STANDARD})
endfunction()

function(custom_add_executable_from_dir TARGET TARGET_SRCS TARGET_INCLUDE)
    custom_add_executable_ex("${TARGET}" "${TARGET_SRCS}" "${TARGET_INCLUDE}" ${TARGET_PROJECT_VERSION}
            ${TARGET_PROJECT_CXX_STANDARD})
endfunction()

### add library target
function(custom_add_library_ex TARGET TARGET_SRCS TARGET_INCLUDE TARGET_VERSION TARGET_CXX_STANDARD)
    IF(WIN32)
        set_source_files_properties("${TARGET_SRCS}" PROPERTIES LANGUAGE CXX)
    ENDIF(WIN32)

    add_library(${TARGET} "${TARGET_SRCS}")

    set_target_properties("${TARGET}" PROPERTIES
            OUTPUT_NAME "${TARGET}"
            VERSION "${TARGET_VERSION}"
            CXX_STANDARD "${TARGET_CXX_STANDARD}"
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
            )

    target_include_directories("${TARGET}" PRIVATE "${TARGET_INCLUDE}")

    configure_target_profile("${TARGET}")
endfunction()

function(custom_add_library_from_current_dir TARGET)
    file(GLOB_RECURSE TARGET_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

    custom_add_library_ex("${TARGET}" "${TARGET_SRCS}" "${CMAKE_CURRENT_SOURCE_DIR}" ${TARGET_PROJECT_VERSION}
            ${TARGET_PROJECT_CXX_STANDARD})
endfunction()

### add all directories recurse
function(custom_add_all_subdirectories)
    custom_add_all_subdirectories_ex("[\\/](\\..*)|(\\$\\$.*\\$\\$)$")
endfunction()

function(custom_add_all_subdirectories_ex EXCLUDE_PATTERN)
    file(GLOB TARGET_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")
    foreach(TARGET_DIR ${TARGET_DIRS})
        IF(IS_DIRECTORY ${TARGET_DIR} AND NOT (${TARGET_DIR} MATCHES "${EXCLUDE_PATTERN}"))
            add_subdirectory(${TARGET_DIR})
        endif()
    endforeach()
endfunction()
