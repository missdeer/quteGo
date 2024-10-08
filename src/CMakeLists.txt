cmake_minimum_required(VERSION 3.21)
project(quteGo VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Xml Svg Sql Multimedia Network PrintSupport LinguistTools)
IF (Qt6_FOUND)
        find_package(Qt6 COMPONENTS Core5Compat Svgwidgets REQUIRED)
ENDIF(Qt6_FOUND)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON) # if you have any .ui files
set(CMAKE_AUTORCC OFF) # if you have any .qrc files

add_compile_definitions(QT_THREAD_SUPPORT)
include_directories(${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS})

find_package(ICU 70.0 COMPONENTS uc i18n REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Boost 1.79.0)

set(HEADERS 
    analyzedlg.h
    autodiagsdlg.h
    config.h
    clickableviews.h
    clockview.h
    dbdialog.h
    evalgraph.h
    figuredlg.h
    gamedialog.h
    gamestable.h
    gametree.h
    gs_globals.h
    igsconnection.h
    clientwin.h
    misc.h
    msg_handler.h
    parser.h
    playertable.h
    qgo_interface.h
    setting.h
    tables.h
    telnet.h
    board.h
    defines.h
    goboard.h
    gogame.h
    helpviewer.h
    imagehandler.h
    komispinbox.h
    mainwindow.h
    normaltools.h
    preferences.h
    qgo.h
    qgtp.h
    newaigamedlg.h
    sgf.h
    sgfpreview.h
    scoretools.h
    sizegraphicsview.h
    slideview.h
    svgbuilder.h
    textview.h
    uihelpers.h
    grid.h
    miscdialogs.h
    variantgamedlg.h
    audio.h
    archivehandler.h
    libarchivehandler.h
    qdbhandler.h
    archivehandlerfactory.h
    archiveitemlistwidget.h
    qdbitemmodel.h
    encodingutils.h
    )
set(SOURCES
    analyzedlg.cpp
    autodiagsdlg.cpp
    clientwin.cpp
    clockview.cpp
    dbdialog.cpp
    evalgraph.cpp
    figuredlg.cpp
    gamedialog.cpp
    gamestable.cpp
    gametree.cpp
    goboard.cc
    gogame.cc
    igsconnection.cpp
    main.cpp
    misc.cpp
    msg_handler.cpp
    parser.cpp
    playertable.cpp
    qgo_interface.cpp
    setting.cpp
    tables.cpp
    telnet.cpp
    board.cpp
    helpviewer.cpp
    imagehandler.cpp
    mainwindow.cpp
    preferences.cpp
    qgo.cpp
    qgtp.cpp
    newaigamedlg.cpp
    sgf2board.cc
    sgfload.cc
    sgfpreview.cpp
    slideview.cpp
    svgbuilder.cpp
    textview.cpp
    grid.cpp
    variantgamedlg.cpp
    audio.cpp
    archivehandler.cpp
    libarchivehandler.cpp
    qdbhandler.cpp
    archivehandlerfactory.cpp
    archiveitemlistwidget.cpp
    qdbitemmodel.cpp
    encodingutils.cpp
    )
set(FORMS 
    gui_dialog.ui
    analyze_gui.ui
    autodiags_gui.ui
    boardwindow_gui.ui
    clientwindow_gui.ui
    dbdialog_gui.ui
    figuredlg_gui.ui
    gameinfo_gui.ui
    newgame_gui.ui
    newlocalgame_gui.ui
    newvariantgame_gui.ui
    scoretools_gui.ui
    normaltools_gui.ui
    preferences_gui.ui
    newaigamedlg_gui.ui
    twoaigamedlg_gui.ui
    sgfpreview.ui
    talk_gui.ui
    textedit_gui.ui
    textview_gui.ui
    slideview_gui.ui
    svgview_gui.ui
    nthmove_gui.ui
    )

IF(WIN32)
    list(APPEND HEADERS sdbhandler.h)
    list(APPEND SOURCES sdbhandler.cpp)
ENDIF()        

qt_add_resources(qrcs q4go.qrc)
set(TS_FILES
    translations/quteGo_zh_CN.ts
    translations/quteGo_zh_TW.ts
    translations/quteGo_cz.ts
    translations/quteGo_de.ts
    translations/quteGo_dk.ts
    translations/quteGo_en_US.ts
    translations/quteGo_es.ts
    translations/quteGo_fr.ts
    translations/quteGo_it.ts
    translations/quteGo_jp.ts
    translations/quteGo_kr.ts
    translations/quteGo_la.ts
    translations/quteGo_nl.ts
    translations/quteGo_pl.ts
    translations/quteGo_pt.ts
    translations/quteGo_ru.ts
    translations/quteGo_tr.ts
    )
IF(APPLE)
set_source_files_properties(
    ${TS_FILES} PROPERTIES OUTPUT_LOCATION 
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/translations
  )
ELSE()
set_source_files_properties(
    ${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/translations)
ENDIF()

IF(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_create_translation(QM_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${TS_FILES}
            OPTIONS -no-obsolete -no-recursive
        )
        qt_add_executable(${PROJECT_NAME}
                ${HEADERS}
                ${SOURCES}
                ${FORMS}
                ${qrcs}
                ${QM_FILES}
                qgo.rc
                )
ELSE()
        qt5_create_translation(QM_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${TS_FILES}
            OPTIONS -no-obsolete -no-recursive
        )
        add_executable(${PROJECT_NAME}
                ${HEADERS}
                ${SOURCES}
                ${FORMS}
                ${qrcs}
                ${QM_FILES}
                qgo.rc
                )
ENDIF()

target_link_libraries(${PROJECT_NAME}  PRIVATE  LibArchive::LibArchive
        ICU::uc ICU::i18n
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::Svg
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::PrintSupport
        )
IF (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE AdvApi32 OleAut32 User32 Shell32 PowrProf Ole32 Uuid)
    if("${CURRENT_COMPILER_ID}" STREQUAL "Clang" AND "${CURRENT_SIMULATE_ID}" STREQUAL "MSVC")
        target_link_options(${PROJECT_NAME} PRIVATE -Wl,-entry:wWinMainCRTStartup)
    endif()
ENDIF()

IF (Qt6_FOUND)
    target_link_libraries(${PROJECT_NAME}  PRIVATE
            Qt${QT_VERSION_MAJOR}::Core5Compat
            Qt${QT_VERSION_MAJOR}::SvgWidgets
            )
ENDIF(Qt6_FOUND)

if(Boost_FOUND)
  # dont use include_directories, use target_include_directories instead to let lupdate skip boost directories
  target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_directories(${PROJECT_NAME} PUBLIC ${Boost_LIBRARY_DIRS})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON
                                                 MACOSX_BUNDLE ON)

# copy sounds directory to output directory
IF(APPLE)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/sounds
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/sounds
    )
ELSE()
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/sounds
      ${CMAKE_CURRENT_BINARY_DIR}/sounds
    )
ENDIF()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_BINDIR}
)
