FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
SET(QT_USE_QTOPENGL TRUE)

FIND_PACKAGE( OpenGL REQUIRED )
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(.)

# Q_OBJECT handling
set(KBX_MOC_HDRS
  config.hpp
  game_widget.hpp
  main_window.hpp
  preferences_dialog.hpp
  new_game_dialog.hpp
)
QT4_WRAP_CPP(KBX_MOC_OUTFILES ${KBX_MOC_HDRS})

# ressource binding
SET(KBX_RESOURCES ../res.qrc)
QT4_ADD_RESOURCES(KBX_RESOURCES_RCC ${KBX_RESOURCES})

# UI creation
SET(KBX_FORMS ../ui/mainWindow.ui ../ui/aboutDialog.ui ../ui/preferencesDialog.ui ../ui/newGameDialog.ui)
QT4_WRAP_UI(KBX_FORMS_HEADERS ${KBX_FORMS})

ADD_EXECUTABLE(../Kubix
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

TARGET_LINK_LIBRARIES(../Kubix
  ${QT_LIBRARIES}
  ${QT_QTOPENGL_LIBRARIES}
  ${OPENGL_LIBRARIES}
)
