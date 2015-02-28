find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
ADD_DEFINITIONS(${QT_DEFINITIONS})
FIND_PACKAGE( OpenGL REQUIRED )
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Gui_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5OpenGL_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(.)

# Q_OBJECT handling
set(KBX_MOC_HDRS
  config.hpp
  game_widget.hpp
  main_window.hpp
  preferences_dialog.hpp
  new_game_dialog.hpp
  )
QT5_WRAP_CPP(KBX_MOC_OUTFILES ${KBX_MOC_HDRS})

# ressource binding
SET(KBX_RESOURCES ${CMAKE_SOURCE_DIR}/res.qrc)
QT5_ADD_RESOURCES(KBX_RESOURCES_RCC ${KBX_RESOURCES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# UI creation
SET(KBX_FORMS ${CMAKE_SOURCE_DIR}/ui/mainWindow.ui ${CMAKE_SOURCE_DIR}/ui/aboutDialog.ui ${CMAKE_SOURCE_DIR}/ui/preferencesDialog.ui ${CMAKE_SOURCE_DIR}/ui/newGameDialog.ui)
QT5_WRAP_UI(KBX_FORMS_HEADERS ${KBX_FORMS})

ADD_EXECUTABLE(Kubix
  kubix.cpp
  config.cpp
  main_window.cpp
  preferences_dialog.cpp
  new_game_dialog.cpp
  engine.cpp
  game_widget.cpp
  models.cpp
  tools.cpp
  serialization.cpp
  ${KBX_MOC_OUTFILES}
  ${KBX_FORMS_HEADERS}
  ${KBX_RESOURCES_RCC}
  )

install(TARGETS Kubix RUNTIME DESTINATION bin)

TARGET_LINK_LIBRARIES(
  Kubix
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5OpenGL_LIBRARIES}
  ${OPENGL_LIBRARIES}
  )
