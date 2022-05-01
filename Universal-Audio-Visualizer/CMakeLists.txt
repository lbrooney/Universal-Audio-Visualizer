cmake_minimum_required(VERSION 3.5)

project(Universal-Audio-Visualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS OpenGLWidgets Gui Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGLWidgets Gui Core)

include_directories(Headers/
                    Shaders/
                    Sources/
					lib/)

set(PROJECT_SOURCES
        Sources/main.cpp
        Sources/mainwindow.cpp
        Headers/mainwindow.h
        Headers/Shape.h
        Sources/mainwindow.ui
        Headers/oglwidget.h
        Sources/oglwidget.cpp
        Shaders/vertex.glsl
        Shaders/fragment.glsl
        Headers/Square.h
        Headers/Triangle.h
        Headers/Circle.h
        Headers/Cube.h
        Headers/Sphere.h
        Shaders.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Universal-Audio-Visualizer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Universal-Audio-Visualizer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Universal-Audio-Visualizer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Universal-Audio-Visualizer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Universal-Audio-Visualizer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets PRIVATE Qt${QT_VERSION_MAJOR}::Gui PRIVATE Qt${QT_VERSION_MAJOR}::Core)

set_target_properties(Universal-Audio-Visualizer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Universal-Audio-Visualizer)
endif()
