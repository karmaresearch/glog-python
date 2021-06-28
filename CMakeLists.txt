cmake_minimum_required (VERSION 2.8.12)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
if (NOT "$ENV{CC}" STREQUAL "")
    set(CMAKE_C_COMPILER "$ENV{CC}")
endif()
if (NOT "$ENV{CXX}" STREQUAL "")
    set(CMAKE_CXX_COMPILER "$ENV{CXX}")
endif()

project (glog-python)
include(ExternalProject)
set(CMAKE_MACOSX_RPATH 1)

#Add homebrew include and libs
EXECUTE_PROCESS(COMMAND brew --prefix RESULT_VARIABLE exitstatus OUTPUT_VARIABLE BREW_PREFIX ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
IF (${exitstatus} EQUAL 0)
    LINK_DIRECTORIES(${BREW_PREFIX}/lib)
    INCLUDE_DIRECTORIES(${BREW_PREFIX}/include)
ENDIF()

#Set to release mode if nothing is specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

#add files to compile
file(GLOB main_SRC
    "src/cplusplus/*.cpp"
    )

#ZLIB
find_package(ZLIB REQUIRED)

#Trident
IF(GLOG_LIB)
    message("Using KOGNAC provided installation: ${KOGNAC_LIB} ${KOGNAC_INC}")

    include_directories(${KOGNAC_INC})
    LINK_DIRECTORIES("${KOGNAC_LIB}")
    add_library(kognac-core SHARED IMPORTED)
    set_property(TARGET kognac-core PROPERTY IMPORTED_LOCATION ${KOGNAC_LIB}/${CMAKE_SHARED_LIBRARY_PREFIX}kognac-core${CMAKE_SHARED_LIBRARY_SUFFIX})

    message("Using Trident provided installation: ${TRIDENT_LIB} ${TRIDENT_INC}")
    include_directories(${TRIDENT_INC})
    include_directories(${TRIDENT_INC}/layers)
    include_directories(${TRIDENT_INC}/../rdf3x/include)
    add_library(trident-core SHARED IMPORTED)
    set_property(TARGET trident-core PROPERTY IMPORTED_LOCATION ${TRIDENT_LIB}/${CMAKE_SHARED_LIBRARY_PREFIX}trident-core${CMAKE_SHARED_LIBRARY_SUFFIX})
    add_library(trident-sparql SHARED IMPORTED)
    set_property(TARGET trident-sparql PROPERTY IMPORTED_LOCATION ${TRIDENT_LIB}/${CMAKE_SHARED_LIBRARY_PREFIX}trident-sparql${CMAKE_SHARED_LIBRARY_SUFFIX})
    LINK_DIRECTORIES("${TRIDENT_LIB}/")

    message("Using GLog provided installation: ${GLOG_LIB} ${GLOG_INC}")
    include_directories(${GLOG_INC})
    add_library(glog-core SHARED IMPORTED)
    set_property(TARGET glog-core PROPERTY IMPORTED_LOCATION ${GLOG_LIB}/${CMAKE_SHARED_LIBRARY_PREFIX}vlog-core${CMAKE_SHARED_LIBRARY_SUFFIX})
    LINK_DIRECTORIES("${GLOG_LIB}/")

ELSE()
    ExternalProject_Add(git-glog
        DOWNLOAD_COMMAND git clone https://github.com/karmaresearch/glog.git
        DOWNLOAD_DIR .
        SOURCE_DIR glog
        BUILD_IN_SOURCE 1
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        INSTALL_COMMAND ""
        )
    ExternalProject_Get_Property(git-glog SOURCE_DIR)
    include_directories(${SOURCE_DIR}/include)

    # KOGNAC
    set(KOGNAC_LIB ${CMAKE_BINARY_DIR}/glog/trident/kognac)
    add_library(kognac-core SHARED IMPORTED)
    add_dependencies(kognac-core git-glog)
    include_directories(${SOURCE_DIR}/trident/kognac/include)
    LINK_DIRECTORIES("${KOGNAC_LIB}")
    set_property(TARGET kognac-core PROPERTY IMPORTED_LOCATION ${KOGNAC_LIB}/${CMAKE_SHARED_LIBRARY_PREFIX}kognac-core${CMAKE_SHARED_LIBRARY_SUFFIX})

    # TRIDENT
    set(TRIDENT_LIB ${CMAKE_BINARY_DIR}/glog/trident)
    add_library(trident-core SHARED IMPORTED)
    add_dependencies(trident-core kognac-core)
    include_directories(${SOURCE_DIR}/trident/include)
    include_directories(${SOURCE_DIR}/trident/rdf3x/include)
    set_property(TARGET trident-core PROPERTY IMPORTED_LOCATION "${TRIDENT_LIB}/${CMAKE_SHARED_LIBRARY_PREFIX}trident-core${CMAKE_SHARED_LIBRARY_SUFFIX}")
    LINK_DIRECTORIES("${TRIDENT_LIB}")

    # TRIDENT SPARQL
    add_library(trident-sparql SHARED IMPORTED)
    add_dependencies(trident-sparql trident-core)
    set_property(TARGET trident-sparql PROPERTY IMPORTED_LOCATION "${TRIDENT_LIB}/${CMAKE_SHARED_LIBRARY_PREFIX}trident-sparql${CMAKE_SHARED_LIBRARY_SUFFIX}")

    # GLOG CORE
    set(GLOG_LIB ${CMAKE_BINARY_DIR}/glog)
    add_library(glog-core SHARED IMPORTED)
    add_dependencies(glog-core trident-sparql)
    set_property(TARGET glog-core PROPERTY IMPORTED_LOCATION "${GLOG_LIB}/${CMAKE_SHARED_LIBRARY_PREFIX}vlog-core${CMAKE_SHARED_LIBRARY_SUFFIX}")
ENDIF()

#SPARSEHASH
find_path(sparsehash sparsehash/sparse_hash_map)
IF (${sparsehash} STREQUAL "sparsehash-NOTFOUND")
    message("Could not find sparsehash. I'm going to use the version from kognac ...")
    include_directories(${KOGNAC_LIB}/external/sparsehash/src/)
ENDIF()

message("I'm going to use LZ4 the version from kognac ${KOGNAC_LIB}")
include_directories(${KOGNAC_LIB}/external/lz4/lib/)
LINK_DIRECTORIES(${KOGNAC_LIB}/external/lz4/lib)
add_library(lz4 STATIC IMPORTED)
set_property(TARGET lz4 PROPERTY IMPORTED_LOCATION ${KOGNAC_LIB}/external/lz4/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lz4${CMAKE_STATIC_LIBRARY_SUFFIX})

#Set compiler options
if((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        set(COMPILE_FLAGS "${COMPILE_FLAGS} -c -fPIC -MD -std=c++11 ${CLANG_ONLY_COMPILE_FLAGS}")
        if(LINUX)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++")
        endif()
    else()
        set(COMPILE_FLAGS "${COMPILE_FLAGS} -c -fPIC -MD -std=c++11 ${GNUCXX_ONLY_COMPILE_FLAGS}")
    endif()
endif()

#standard include
include_directories(include/)

find_package (Python3 COMPONENTS Interpreter Development)
IF(Python3_FOUND)
    INCLUDE_DIRECTORIES(${Python3_INCLUDE_DIRS})
    #message("Python include path = ${Python3_INCLUDE_DIRS}")
ENDIF()
#NumPY
if (Python3_EXECUTABLE)
    # Find out the include path
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c
        "from __future__ import print_function\ntry: import numpy; print(numpy.get_include(), end='')\nexcept:pass\n"
        OUTPUT_VARIABLE __numpy_path)
    # And the version
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c
        "from __future__ import print_function\ntry: import numpy; print(numpy.__version__, end='')\nexcept:pass\n"
        OUTPUT_VARIABLE __numpy_version)
elseif(__numpy_out)
    message(STATUS "Python executable not found.")
endif(Python3_EXECUTABLE)

find_path(PYTHON_NUMPY_INCLUDE_DIR numpy/arrayobject.h
    HINTS "${__numpy_path}" "${Python3_INCLUDE_DIRS}" NO_DEFAULT_PATH)
if(PYTHON_NUMPY_INCLUDE_DIR)
    set(PYTHON_NUMPY_FOUND 1 CACHE INTERNAL "Python numpy found")
    include_directories(${PYTHON_NUMPY_INCLUDE_DIR} SYSTEM)
endif(PYTHON_NUMPY_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NumPy REQUIRED_VARS PYTHON_NUMPY_INCLUDE_DIR
    VERSION_VAR __numpy_version)

add_library(glogp SHARED ${main_SRC})
set_target_properties(glogp PROPERTIES OUTPUT_NAME "glog" PREFIX "" SUFFIX ".so")
set_target_properties(glogp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(glogp glog-core trident-core kognac-core ${Python3_LIBRARIES})
