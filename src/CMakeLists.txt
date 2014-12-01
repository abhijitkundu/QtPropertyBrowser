# Tell CMake to run moc when necessary:
#set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(KIT_SRCS
  qtbuttonpropertybrowser.cpp
  qtbuttonpropertybrowser.h
  qteditorfactory.cpp
  qteditorfactory.h
  qtgroupboxpropertybrowser.cpp
  qtgroupboxpropertybrowser.h
  qtpropertybrowser.cpp
  qtpropertybrowser.h
  qtpropertybrowserutils.cpp
  qtpropertybrowserutils_p.h
  qtpropertymanager.cpp
  qtpropertymanager.h
  qttreepropertybrowser.cpp
  qttreepropertybrowser.h
  qtvariantproperty.cpp
  qtvariantproperty.h
  )

SET(KIT_MOC_SRCS
  qtpropertybrowserutils_p.h
  )

SET(KIT_UI_FORMS
  )

SET(KIT_resources
  qtpropertybrowser.qrc
  )
  
if(QT5_FOUND)
  QT5_WRAP_UI(KIT_UI_CPP ${KIT_UI_FORMS})
  QT5_WRAP_CPP(KIT_MOC_CPP ${KIT_MOC_SRCS})
  QT5_ADD_RESOURCES(KIT_QRC_SRCS ${KIT_resources})
ELSE()
  QT4_WRAP_UI(KIT_UI_CPP ${KIT_UI_FORMS})
  QT4_WRAP_CPP(KIT_MOC_CPP ${KIT_MOC_SRCS})
  QT4_ADD_RESOURCES(KIT_QRC_SRCS ${KIT_resources})
ENDIF()

SET(libname ${PROJECT_NAME})
ADD_LIBRARY(${libname} STATIC
  ${KIT_SRCS}
  ${KIT_UI_CPP}
  ${KIT_MOC_CPP}
  ${KIT_QRC_SRCS}
  )

target_include_directories(${libname} PRIVATE ${QT_INCLUDE_DIRS})
target_compile_definitions(${libname} PRIVATE ${QT_COMPILE_DEFS})
set_target_properties(${libname} PROPERTIES POSITION_INDEPENDENT_CODE ON)

  
SET(${PROJECT_NAME}_LINK_LIBRARIES ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(
  ${libname}
  ${${PROJECT_NAME}_LINK_LIBRARIES}
  )

# List of header that should go though moc
SET(KIT_HEADERS_MOC_SRCS
  qtbuttonpropertybrowser.h
  qteditorfactory.h
  qtgroupboxpropertybrowser.h
  qtpropertybrowser.h
  qtpropertybrowserutils_p.h
  qtpropertymanager.h
  qttreepropertybrowser.h
  qtvariantproperty.h
  )

SET(KIT_MOC_CPP)
FOREACH(file ${KIT_HEADERS_MOC_SRCS})
  get_filename_component(filename_we ${file} NAME_WE)
  SET(output_file moc_${filename_we}.cpp)

  IF(QT5_FOUND)
    QT5_GENERATE_MOC(${file} ${output_file})
  ELSE()
    QT4_GENERATE_MOC(${file} ${output_file})
  ENDIF()

  LIST(APPEND KIT_MOC_CPP ${output_file})
ENDFOREACH()

# List of cpp files that should go though moc
SET(KIT_CPP_MOC_SRCS
  qteditorfactory.cpp
  qtpropertymanager.cpp
  qttreepropertybrowser.cpp
  )

FOREACH(file ${KIT_CPP_MOC_SRCS})
  get_filename_component(filename_we ${file} NAME_WE)
  SET(output_file ${filename_we}.moc)

  if(QT5_FOUND)
    QT5_GENERATE_MOC(${file} ${output_file})
  ELSE()
    QT4_GENERATE_MOC(${file} ${output_file})
  ENDIF()
  
  LIST(APPEND KIT_MOC_CPP ${output_file})
ENDFOREACH()

add_custom_target(${libname}GenerateMoc
  DEPENDS ${KIT_MOC_CPP}
  )
add_dependencies(${libname} ${libname}GenerateMoc)
