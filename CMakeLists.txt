cmake_minimum_required(VERSION 3.16)

project(ClientFlow VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets Charts Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    statistic.h statistic.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ClientFlow
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        database.h database.cpp
        config.h config.cpp
        clientwindow.h clientwindow.cpp
        clientwindow.ui
        settingsdialog.h settingsdialog.cpp settingsdialog.ui
        statistic.h statistic.cpp
        validation/validationrule.cpp validation/validationrule.h
        validation/notemptyrule.h validation/notemptyrule.cpp
        validation/emailrule.h validation/emailrule.cpp
        validation/minlengthrule.h validation/minlengthrule.cpp
        validation/validatorconfigloader.h validation/validatorconfigloader.cpp
        validationrulemodel.h validationrulemodel.cpp
        validationruleeditor.h validationruleeditor.cpp
        validationruledialog.h validationruledialog.cpp


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ClientFlow APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ClientFlow SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ClientFlow
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ClientFlow PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Core Qt6::Sql Qt6::Charts Qt6::Gui)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ClientFlow)
endif()
set_target_properties(ClientFlow PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ClientFlow
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ClientFlow)
endif()
