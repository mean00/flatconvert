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
SET(SRCS fontconvert.c)
include_directories(${FREETYPE2_INCLUDE_DIR})
ADD_DEFINITIONS(${FREETYPE2_CFLAGS})
LINK_DIRECTORIES(${FREETYPE2_LIBRARY_DIR})

MACRO(GEN target src)
    ADD_EXECUTABLE(${target} ${src})
    TARGET_LINK_LIBRARIES(${target}  ${FREETYPE2_LIBRARIES})
ENDMACRO(GEN target src)

GEN(fontconvert fontconvert.c)    
GEN(flatconvert flatconvert.cpp)    
