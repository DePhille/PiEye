# ----------------------------------------------------------------------------
#   Basic Configuration
# ----------------------------------------------------------------------------
CMAKE_MINIMUM_REQUIRED(VERSION 3.6.0)

PROJECT(PiEye
		VERSION 1.0
		LANGUAGES CXX)

# ----------------------------------------------------------------------------
# Options
# ----------------------------------------------------------------------------

SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type [Release|Debug]")
SET(PIEYE_LOG_LEVEL "4" CACHE STRING "Minimum log level for PiEye library [1=TRACE|2=DEBUG|3=INFO|4=WARN|5=ERROR|6=OFF]")
SET(TEST_LOG_LEVEL "2" CACHE STRING "Minimum log level for test [1=TRACE|2=DEBUG|3=INFO|4=WARN|5=ERROR|6=OFF]")
SET_PROPERTY(CACHE PIEYE_LOG_LEVEL PROPERTY STRINGS 1 2 3 4 5 6)
SET_PROPERTY(CACHE TEST_LOG_LEVEL PROPERTY STRINGS 1 2 3 4 5 6)

# ----------------------------------------------------------------------------
# Build type flags
# ----------------------------------------------------------------------------
IF(NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE "Release" )
ENDIF()

SET(FLAGS_COMMON "-Wno-pedantic -Wall -Wno-variadic-macros -std=c++0x -Wl,--no-as-needed")
SET(FLAGS_RELEASE "-g0 -O3")
SET(FLAGS_DEBUG "-g3 -O0")
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_COMMON} ${FLAGS_DEBUG}")
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_COMMON} ${FLAGS_RELEASE}")
ENDIF()

# ----------------------------------------------------------------------------
# Try to use CCache
# ----------------------------------------------------------------------------
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# ----------------------------------------------------------------------------
#   Find Dependencies
# ----------------------------------------------------------------------------

# MMAL libraries
FIND_LIBRARY(mmalcore_LIBS NAMES mmal_core PATHS /opt/vc/lib)
FIND_LIBRARY(mmalutil_LIBS NAMES mmal_util PATHS /opt/vc/lib)
FIND_LIBRARY(mmal_LIBS NAMES mmal PATHS /opt/vc/lib)
IF( (NOT mmal_LIBS ) OR (NOT mmalutil_LIBS) OR (NOT mmalcore_LIBS) )
    MESSAGE(FATAL_ERROR "Could not find mmal libraries")
ENDIF()

# MMAL headers
include_directories("/opt/vc/include/")

# OpenCV
FIND_PACKAGE(OpenCV REQUIRED)
IF(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})  
ENDIF()

# ----------------------------------------------------------------------------
#   Sources
# ----------------------------------------------------------------------------

ADD_SUBDIRECTORY("PiEye")
ADD_SUBDIRECTORY("PiEyeTest")

SET (PIEYE_SRC src/main
    src/PiEye
    src/PiEyeImpl
    src/BufferLock
	src/Wait
    src/EzLogger
)

#add_executable(PiEyeTest ${PIEYE_SRC})

#TARGET_LINK_LIBRARIES(PiEyeTest ${mmalcore_LIBS} ${mmalutil_LIBS} ${mmal_LIBS} ${OpenCV_LIBS})

# ----------------------------------------------------------------------------
# display status message for important variables
# ----------------------------------------------------------------------------
MESSAGE( STATUS )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS "General configuration for ${PROJECT_NAME}")
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS )
MESSAGE( STATUS "Options:")
MESSAGE( STATUS "CMAKE_BUILD_TYPE:          ${CMAKE_BUILD_TYPE}")
MESSAGE( STATUS "PIEYE_LOG_LEVEL:           ${PIEYE_LOG_LEVEL}")
MESSAGE( STATUS "TEST_LOG_LEVEL:            ${TEST_LOG_LEVEL}")
MESSAGE( STATUS )
MESSAGE( STATUS "Compiler:"                   "${CMAKE_COMPILER}"   "${CMAKE_CXX_COMPILER}")
MESSAGE( STATUS "Using CCACHE:              ${CCACHE_FOUND}")
MESSAGE( STATUS "C++ flags (Common):        ${FLAGS_COMMON}")
MESSAGE( STATUS "C++ flags (Release):       ${FLAGS_RELEASE}")
MESSAGE( STATUS "C++ flags (Debug):         ${FLAGS_DEBUG}")
MESSAGE( STATUS "C++ flags:                 ${CMAKE_CXX_FLAGS}")

MESSAGE( STATUS )
MESSAGE( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE( STATUS )
