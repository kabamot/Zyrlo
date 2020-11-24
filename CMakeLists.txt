cmake_minimum_required(VERSION 3.14)

project(Zyrlo LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick LinguistTools REQUIRED)

set(TS_FILES translations/Zyrlo_en_US.ts)

set(SRC_FILES
    src/main.cpp
    src/maincontroller.cpp
    src/maincontroller.h
    src/ocrhandler.cpp
    src/ocrhandler.h
)

set(QML_FILES
    qml/main.qml
)

set(QRC_FILES
    qml.qrc
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${QRC_FILES}
    ${QML_FILES}
    ${TS_FILES}
)

if (${CMAKE_CROSSCOMPILING})
    find_package(OpenCV REQUIRED)
    add_definitions(-DRPI)
else()
    find_package(OpenCV 3.2.0 EXACT REQUIRED PATHS "/home/dilshodm/work/proj/upwork/leon/opencv")
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

find_library(ZYRLO_OCR_LIB ZyrloOcr HINTS /home/dilshodm/work/proj/upwork/leon/doc/Locate/ZyrloOcr/Ubuntu REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick

    ${OpenCV_LIBS}
    ${ZYRLO_OCR_LIB}
)

include_directories(
    3rd/locate
    ${OpenCV_INCLUDE_DIRS}/..
)

target_link_directories(${PROJECT_NAME}
    PUBLIC
        /opt/vc/lib
)

if (${CMAKE_CROSSCOMPILING})
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "/tmp/")
endif()
