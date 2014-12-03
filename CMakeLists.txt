CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT(QtPropertyBrowser)

########################### Misc. Configs ##############################
# Add custom cmake files folder
SET (PROJECT_CMAKE_DIR ${PROJECT_SOURCE_DIR}/cmake)
SET (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_CMAKE_DIR}")

# Add custom Compile Settings and flags
INCLUDE(CompileSettings)

# Add custom Install Settings
INCLUDE(InstallSettings)

##################### Look for required libraries ######################

# Add QT dependencies
INCLUDE(AddQt)
IF (NOT QT_FOUND) 
  MESSAGE(FATAL "We Need some QT")
ENDIF()

######################### Add Primary Targets ##########################
ADD_SUBDIRECTORY(src)

########################## Add the Examples ############################
OPTION (BUILD_EXAMPLES "Build Examples" ON)
IF(BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF()

######################### Installation Stuff ###########################
INCLUDE(InstallProjectConfig)

###################### Add uninstall target ############################
ADD_CUSTOM_TARGET(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${PROJECT_CMAKE_DIR}/cmake_uninstall.cmake)

##################### Show Configuration Summary ######################
MESSAGE(STATUS "===============================================================")
MESSAGE(STATUS "================  Configuration Summary  ======================")
MESSAGE(STATUS "Project Name:       ${PROJECT_NAME}")
MESSAGE(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Build type Flags:   ${CMAKE_BUILD_TYPE_FLAGS}")
MESSAGE(STATUS "C++ compile flags:  ${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "Install Path:       ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Build Examples:     ${BUILD_EXAMPLES}")
IF(QT_FOUND )
  MESSAGE(STATUS "QT Version:         ${QT_VERSION_STRING}")
  MESSAGE(STATUS "QT_TARGETS:         ${QT_TARGETS}")
  MESSAGE(STATUS "QT_ALL_TARGETS:     ${QT_ALL_TARGETS}")
  MESSAGE(STATUS "QT_INCLUDE_DIRS:    ${QT_INCLUDE_DIRS}")
  MESSAGE(STATUS "QT_COMPILE_DEFS:    ${QT_COMPILE_DEFS}")  
ENDIF()
