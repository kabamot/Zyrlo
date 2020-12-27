cmake_minimum_required(VERSION 3.14)

set(LIB_NAME core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DQT_STATICPLUGIN)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core LinguistTools TextToSpeech Multimedia Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core LinguistTools TextToSpeech Multimedia Concurrent REQUIRED)

set(INC_FILES
    include/maincontroller.h
    include/ocrhandler.h
    include/textpage.h
    include/paragraph.h
)

set(SRC_FILES
    src/maincontroller.cpp
    src/ocrhandler.cpp
    src/textpage.cpp
    src/paragraph.cpp

    src/cerence/cerencetts.cpp
    src/cerence/cerencetts.h
)

#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_library(${LIB_NAME}
    ${SRC_FILES}
    ${INC_FILES}
)

if (${CMAKE_CROSSCOMPILING})
    find_package(OpenCV REQUIRED)
    add_definitions(-DRPI)
else()
    find_package(OpenCV 3.2.0 EXACT REQUIRED PATHS "/home/dilshodm/work/proj/upwork/leon/opencv")
endif()

target_compile_definitions(${LIB_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

find_library(ZYRLO_OCR_LIB ZyrloOcr HINTS /home/dilshodm/work/proj/upwork/leon/doc/Locate/ZyrloOcr/Ubuntu REQUIRED)
find_library(VE_LIB ve HINTS /home/dilshodm/work/proj/upwork/leon/doc/csdk/lib REQUIRED)

add_subdirectory(3rd/tts_ve)

target_link_libraries(${LIB_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::TextToSpeech
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::Concurrent
        ${OpenCV_LIBS}

    PRIVATE
        ${ZYRLO_OCR_LIB}
        ${VE_LIB}
        tts_ve
)

target_include_directories(${LIB_NAME}
    PUBLIC
        include
        ${OpenCV_INCLUDE_DIRS}/..
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rd/doctest
        3rd/locate
)

target_link_directories(${LIB_NAME}
    PUBLIC
        /opt/vc/lib
)
