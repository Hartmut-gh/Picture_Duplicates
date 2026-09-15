cmake_minimum_required(VERSION 3.16)

project(Picture_Duplicates VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Concurrent LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Concurrent LinguistTools)
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

set(TS_FILES
    translations/Picture_Duplicates_en_US.ts
    translations/Picture_Duplicates_en_GB.ts
    translations/Picture_Duplicates_de_DE.ts
)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Picture_Duplicates
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        folderset.h

        duplicatemodel.h duplicatemodel.cpp
        imagebox.h imagebox.cpp
        nomanualparametersdialog.h nomanualparametersdialog.cpp nomanualparametersdialog.ui
    )

    set(QM_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/Picture_Duplicates_en_US.qm"
        "${CMAKE_CURRENT_BINARY_DIR}/Picture_Duplicates_en_GB.qm"
        "${CMAKE_CURRENT_BINARY_DIR}/Picture_Duplicates_de_DE.qm"
    )

    message(STATUS "TS_FILES: ${TS_FILES}")
    message(STATUS "QM_FILES: ${QM_FILES}")

    qt_add_translations(Picture_Duplicates TS_FILES
        TS_FILES ${TS_FILES}
        LRELEASE_OPTIONS -idbased
        QM_FILES_OUTPUT_VARIABLE QM_FILES
    )

    file(GLOB ALL_QM_FILES "${CMAKE_CURRENT_BINARY_DIR}/*.qm")
    foreach(qm ${ALL_QM_FILES})
        file(SIZE ${qm} QM_SIZE)
        message(STATUS "QM found: ${qm} (${QM_SIZE} bytes)")
    endforeach()

    # ================================================================================================
    # TRANSLATION DIR abhängig on Flatpak oder nicht definieren
    # ================================================================================================
    message(STATUS "Definiere TRANSLATION_DIR - FLATPAK_BUILD: ${FLATPAK_BUILD}")
    if(FLATPAK_BUILD)
        message(STATUS "Definiere TRANSLATION_DIR auf: ${CMAKE_CURRENT_BINARY_DIR}/translations")
        target_compile_definitions(Picture_Duplicates PRIVATE
    TRANSLATION_DIR="/app/share/Picture_Duplicates/translations"
            )
        else()
            message(STATUS "Definiere TRANSLATION_DIR auf: ${CMAKE_BINARY_DIR}/translations")
            target_compile_definitions(Picture_Duplicates PRIVATE
                TRANSLATION_DIR="${CMAKE_BINARY_DIR}/translations"
            )
        endif()
        message(STATUS "TRANSLATION_DIR: ${TRANSLATION_DIR}")
    # ================================================================================================
    add_custom_command(TARGET Picture_Duplicates POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E make_directory
             $<TARGET_FILE_DIR:Picture_Duplicates>/translations

         COMMAND ${CMAKE_COMMAND} -E copy_if_different
             ${QM_FILES}
             $<TARGET_FILE_DIR:Picture_Duplicates>/translations/
    )

    target_sources(Picture_Duplicates PRIVATE
        Picture_Duplicates.qrc
    )



# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Picture_Duplicates APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Picture_Duplicates SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Picture_Duplicates
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Picture_Duplicates PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Sql
    ${OpenCV_LIBS}
)

# ====================================================================================================
# qInfo Ausgaben unterdrücken:
# ====================================================================================================
target_compile_definitions(Picture_Duplicates PRIVATE
    $<$<CONFIG:Release>:QT_NO_INFO_OUTPUT>
)
# ====================================================================================================

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Picture_Duplicates)
endif()
set_target_properties(Picture_Duplicates PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Picture_Duplicates
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Picture_Duplicates)
endif()




add_custom_target(appimage
    COMMENT "Building AppImage for Picture_Duplicates"
    COMMAND ${CMAKE_COMMAND} -E echo "Building AppImage for Picture_Duplicates"

    COMMAND ${CMAKE_COMMAND} -E echo "make_directory ${CMAKE_BINARY_DIR}/AppDir/usr/bin"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/AppDir/usr/bin


    COMMAND ${CMAKE_COMMAND} -E echo "copy $<TARGET_FILE:Picture_Duplicates> ${CMAKE_BINARY_DIR}/AppDir/usr/bin/"

    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Picture_Duplicates>
        ${CMAKE_BINARY_DIR}/AppDir/usr/bin/


    COMMAND ${CMAKE_COMMAND} -E echo "copy ${CMAKE_SOURCE_DIR}/Picture_Duplicates.desktop ${CMAKE_BINARY_DIR}/AppDir/"

    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/Picture_Duplicates.desktop
        ${CMAKE_BINARY_DIR}/AppDir/


    COMMAND ${CMAKE_COMMAND} -E echo "copy ${CMAKE_SOURCE_DIR}/resources/Picture_Duplicates.png ${CMAKE_BINARY_DIR}/AppDir/"

    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/resources/Picture_Duplicates.png
        ${CMAKE_BINARY_DIR}/AppDir/


    COMMAND ${CMAKE_COMMAND} -E echo "copy ${CMAKE_SOURCE_DIR}/resources/Picture_Duplicates.svg ${CMAKE_BINARY_DIR}/AppDir/"

    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/resources/Picture_Duplicates.svg
        ${CMAKE_BINARY_DIR}/AppDir/


    COMMAND /home/standard/Applications/Development/linuxdeploy-x86_64.AppImage
        --appdir ${CMAKE_BINARY_DIR}/AppDir
        --desktop-file ${CMAKE_BINARY_DIR}/AppDir/Picture_Duplicates.desktop
        --icon-file ${CMAKE_BINARY_DIR}/AppDir/Picture_Duplicates.png
        --plugin qt
        --output appimage

    COMMAND ${CMAKE_COMMAND} -E echo
            "copy ${CMAKE_BINARY_DIR}/Picture_Duplicates-x86_64.AppImage \"/home/standard/Applications_own/Meine Programme/\""

    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/Picture_Duplicates-x86_64.AppImage
        "/home/standard/Applications_own/Meine Programme/"

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)


# Für FLATPAK-Installation: --------------------------------------------------------------------
install(
    FILES de.hartmutgh.Picture_Duplicates.metainfo.xml
    DESTINATION share/metainfo
)
install(
    FILES io.github.hartmut_gh.Picture_Duplicates.metainfo.xml
    DESTINATION share/metainfo
)

message(STATUS "TRANSLATION_DIR: ${TRANSLATION_DIR}")
install(FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/Picture_Duplicates/translations
)

install(
    FILES resources/icons/Picture_Duplicates.svg
    DESTINATION share/icons/hicolor/scalable/apps
    RENAME de.hartmutgh.Picture_Duplicates.svg
)
install(
    FILES resources/icons/Picture_Duplicates.svg
    DESTINATION share/icons/hicolor/scalable/apps
    RENAME io.github.hartmut_gh.Picture_Duplicates.svg
)
install(
    FILES resources/icons/64x64/Picture_Duplicates.png
    DESTINATION share/icons/hicolor/64x64/apps
    RENAME de.hartmutgh.Picture_Duplicates.png
)
install(
    FILES resources/icons/128x128/Picture_Duplicates.png
    DESTINATION share/icons/hicolor/128x128/apps
    RENAME de.hartmutgh.Picture_Duplicates.png
)
install(
    FILES resources/icons/256x256/Picture_Duplicates.png
    DESTINATION share/icons/hicolor/256x256/apps
    RENAME de.hartmutgh.Picture_Duplicates.png
)
install(
    FILES resources/icons/512x512/Picture_Duplicates.png
    DESTINATION share/icons/hicolor/512x512/apps
    RENAME de.hartmutgh.Picture_Duplicates.png
)

install(
    FILES de.hartmutgh.Picture_Duplicates.desktop
    DESTINATION share/applications
)
install(
    FILES io.github.hartmut_gh.Picture_Duplicates.desktop
    DESTINATION share/applications
)
# ----------------------------------------------------------------------------------------------

set(COPY_FLATPAK_COMMAND)
if(EXISTS "${CMAKE_SOURCE_DIR}/scripts/copy-flatpak.sh")
    set(COPY_FLATPAK_COMMAND
        COMMAND "${CMAKE_SOURCE_DIR}/scripts/copy-flatpak.sh"
    )
endif()


add_custom_target(flatpak
    COMMAND flatpak-builder --repo=flatpak-repo --force-clean build-flatpak de.hartmutgh.Picture_Duplicates.yml
    COMMAND flatpak build-bundle flatpak-repo Picture_Duplicates.flatpak de.hartmutgh.Picture_Duplicates
    ${COPY_FLATPAK_COMMAND}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)


add_custom_target(flatpak-install
    COMMAND flatpak install --user -y  Picture_Duplicates.flatpak
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# add_dependencies(Picture_Duplicates release_translations)
target_sources(Picture_Duplicates PRIVATE ${QM_FILES})
add_dependencies(flatpak Picture_Duplicates)
add_dependencies(flatpak-install flatpak)
