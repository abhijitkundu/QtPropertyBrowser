
SET(example_name demo)

SET(KIT_SRCS
  main.cpp
  )

SET(KIT_resources
  demo.qrc
  )

IF(QT5_FOUND)
  QT5_ADD_RESOURCES(KIT_QRC_SRCS ${KIT_resources})
ELSE()
  QT4_ADD_RESOURCES(KIT_QRC_SRCS ${KIT_resources})
ENDIF()

ADD_EXECUTABLE(${example_name} ${KIT_SRCS} ${KIT_QRC_SRCS})
TARGET_LINK_LIBRARIES(${example_name} ${PROJECT_NAME} ${QT_TARGETS})
