# Library
###################################################################################################
INCLUDE_DIRECTORIES("include")
SET (PIEYE_SRC
    src/PiEye
    src/PiEyeImpl
    src/BufferLock
	src/Wait
    src/EzLogger
    src/EzMessage
)

ADD_LIBRARY(PiEye ${PIEYE_SRC})
TARGET_LINK_LIBRARIES(PiEye ${mmalcore_LIBS} ${mmalutil_LIBS} ${mmal_LIBS})
TARGET_COMPILE_DEFINITIONS(PiEye PRIVATE EZLOG_LEVEL=${PIEYE_LOG_LEVEL})



# Installation
###################################################################################################
SET (PIEYE_INCLUDE
	include/EzLogger.h
	include/EzMessage.h
	include/Log.hpp
	include/PiEye.h
	include/PiEyeException.hpp
	include/SensorMode.hpp
    include/AwbMode.hpp
    include/Encoding.hpp
)
SET_TARGET_PROPERTIES(PiEye PROPERTIES PUBLIC_HEADER "${PIEYE_INCLUDE}")
INSTALL(TARGETS PiEye
        EXPORT PiEye
        LIBRARY DESTINATION "lib"
		ARCHIVE DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "include/PiEye"
)



# Uninstall
###################################################################################################
CONFIGURE_FILE(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)



# Cmake packaging
###################################################################################################
INSTALL(EXPORT PiEye DESTINATION "lib/cmake/PiEye")
INSTALL(FILES PiEyeConfig.cmake DESTINATION "lib/cmake/PiEye")