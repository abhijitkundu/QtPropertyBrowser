# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(_SRCS
  qtbuttonpropertybrowser.cpp
  qteditorfactory.cpp
  qtgroupboxpropertybrowser.cpp
  qtpropertybrowser.cpp
  qtpropertybrowserutils.cpp
  qtpropertymanager.cpp
  qttreepropertybrowser.cpp
  qtvariantproperty.cpp
  )

file(GLOB _IMPL_HDRS *.h)
file(GLOB _PUBLIC_HDRS Qt*)

set(_UI_FORMS
  )

set(_RESOURCES
  qtpropertybrowser.qrc
  )
  
if(QT5_FOUND)
  QT5_WRAP_UI(_UI_SRCS ${_UI_FORMS})
  QT5_ADD_RESOURCES(_QRC_SRCS ${_RESOURCES})
else()
  QT4_WRAP_UI(_UI_SRCS ${_UI_FORMS})
  QT4_ADD_RESOURCES(_QRC_SRCS ${_RESOURCES})
endif()


set(TARGET_NAME ${PROJECT_NAME})

add_library(${TARGET_NAME} STATIC
  ${_SRCS}
  ${_UI_SRCS}
  ${_QRC_SRCS}
  ${_IMPL_HDRS}
  )

target_include_directories(${TARGET_NAME} PRIVATE ${QT_INCLUDE_DIRS})
target_compile_definitions(${TARGET_NAME} PRIVATE ${QT_COMPILE_DEFS})
set_target_properties(${TARGET_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(${TARGET_NAME}_LINK_LIBRARIES ${QT_TARGETS})
target_link_libraries(
  ${TARGET_NAME}
  ${${TARGET_NAME}_LINK_LIBRARIES}
  )

add_dependencies(${TARGET_NAME} ${QT_TARGETS})

######################### Installation Stuff ###########################
include(GenerateExportHeader)
generate_export_header(${TARGET_NAME})

set_target_properties(${TARGET_NAME} PROPERTIES
    PUBLIC_HEADER "${_PUBLIC_HDRS};${_IMPL_HDRS}"
    )

install(TARGETS ${TARGET_NAME}
  EXPORT ${TARGET_NAME}Targets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" 
  COMPONENT dev
)

