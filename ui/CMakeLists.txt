set(APP_NAME Zyrlo)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick LinguistTools REQUIRED)

set(SRC_FILES
    src/main.cpp
)

set(QML_FILES
    qml/main.qml
)

set(QRC_FILES
    qml.qrc
)

set(TS_FILES translations/Zyrlo_en_US.ts)
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
#set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "l10n")
#qt5_add_translation(QM_FILES ${TS_FILES})


add_executable(${APP_NAME}
    ${SRC_FILES}
    ${QRC_FILES}
    ${QML_FILES}
    ${TS_FILES}
)

target_compile_definitions(${APP_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(${APP_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick

    core
)

target_link_directories(${APP_NAME}
    PUBLIC
        /opt/vc/lib
)

if (${CMAKE_CROSSCOMPILING})
    install(TARGETS ${APP_NAME} RUNTIME DESTINATION "/tmp/")
endif()
