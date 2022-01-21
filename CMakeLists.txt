cmake_minimum_required(VERSION 3.10)
project(fontconver C CXX)
#
include(FindPkgConfig)

# Locate freetype 2
PKG_CHECK_MODULES(FREETYPE2 freetype2)

IF (NOT FREETYPE2_FOUND)
    MESSAGE(FATAL_ERROR "Cannot find freetype 2")
ENDIF (NOT FREETYPE2_FOUND)
#
add_subdirectory(hs)
#
SET(SRCS fontconvert.c)
include_directories(${FREETYPE2_INCLUDE_DIR})
ADD_DEFINITIONS(${FREETYPE2_CFLAGS})
LINK_DIRECTORIES(${FREETYPE2_LIBRARY_DIR})

ADD_DEFINITIONS("-DCXXOPTS_NO_EXCEPTIONS")
ADD_DEFINITIONS("-g3 -o0")

include_directories(cxxopts/include)
include_directories(hs/heatshrink)



MACRO(GEN target src)
    ADD_EXECUTABLE(${target} ${src} ${ARGN})
    TARGET_LINK_LIBRARIES(${target}  ${FREETYPE2_LIBRARIES} hs )
ENDMACRO(GEN target src)


            
#GEN(fontconvert fontconvert.c )    
GEN(flatconvert flatconvert.cpp flatconvert_engine.cpp flatconvert_compression.cpp)
