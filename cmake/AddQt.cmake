if(NOT QT_FOUND)
# --- QT5 ---
  find_package(Qt5Core QUIET)
  if(Qt5Core_FOUND)
    find_package(Qt5Gui QUIET)
    find_package(Qt5Widgets QUIET)
    if(Qt5Gui_FOUND AND Qt5Widgets_FOUND)
      set(QT5_FOUND ON)
      set(QT_FOUND  ON)    
    endif()
  endif()
endif()

if(NOT QT_FOUND)
# --- QT4 ---
  find_package(Qt4 QUIET)
  if(QT4_FOUND)
    set(QT_FOUND TRUE)
  endif()
endif()



if(QT_FOUND)
  if(NOT QT5_FOUND)
    message ( STATUS "Using Qt4")    
    set( QT_VERSION_STRING "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
    set( QT_TARGETS "Qt4::QtGui")
  else()
    message ( STATUS "Using Qt5")
    set( QT_VERSION_STRING ${Qt5Core_VERSION_STRING})
    set( QT_TARGETS "Qt5::Gui;Qt5::Widgets")
  endif()

else()  
  message ( STATUS "Qt Not Found")
endif()


#include some custom CMake util functions
INCLUDE(CommonCMakeUtils)

extract_target_properties(QT_TARGETS_DEPENDENCIES QT_TARGETS INTERFACE_LINK_LIBRARIES)
set (QT_ALL_TARGETS ${QT_TARGETS} ${QT_TARGETS_DEPENDENCIES})
list(REMOVE_DUPLICATES QT_ALL_TARGETS)
extract_target_properties(QT_INCLUDE_DIRS QT_ALL_TARGETS INTERFACE_INCLUDE_DIRECTORIES)
extract_target_properties(QT_COMPILE_DEFS QT_ALL_TARGETS INTERFACE_COMPILE_DEFINITIONS)

IF(QT_FOUND )
  MESSAGE(STATUS "QT Version:         ${QT_VERSION_STRING}")
  MESSAGE(STATUS "QT_TARGETS:         ${QT_TARGETS}")
  MESSAGE(STATUS "QT_ALL_TARGETS:     ${QT_ALL_TARGETS}")
  MESSAGE(STATUS "QT_INCLUDE_DIRS:    ${QT_INCLUDE_DIRS}")
  MESSAGE(STATUS "QT_COMPILE_DEFS:    ${QT_COMPILE_DEFS}")  
ENDIF()