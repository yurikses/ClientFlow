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
        validation/onlysymbolsrule.h validation/onlysymbolsrule.cpp
        validation/onlynumsrule.h validation/onlynumsrule.cpp
    )
else()
    if(ANDROID)
        add_library(ClientFlow SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(ClientFlow
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ClientFlow PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Core Qt6::Sql Qt6::Charts Qt6::Gui)

set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ClientFlow)
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

if(WIN32 AND NOT MSYS)
    add_custom_command(TARGET ClientFlow POST_BUILD
        COMMAND ${Qt6_DIR}/../../../bin/windeployqt.exe
        --dir \"$<TARGET_FILE_DIR:ClientFlow>\"
        $<TARGET_FILE:ClientFlow>
        COMMENT "Deploying Qt dependencies with windeployqt..."
    )
endif()

# Финализация (только для Qt6)
qt_finalize_target(ClientFlow)
