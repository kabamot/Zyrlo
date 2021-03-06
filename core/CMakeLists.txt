cmake_minimum_required(VERSION 3.14)

set(LIB_NAME core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DQT_STATICPLUGIN)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core LinguistTools TextToSpeech Multimedia Concurrent Bluetooth REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core LinguistTools TextToSpeech Multimedia Concurrent Bluetooth REQUIRED)

set(INC_FILES
    include/maincontroller.h
    include/ocrhandler.h
    include/textpage.h
    include/paragraph.h
    include/textposition.h
    include/zyrlocamera.h
    include/OFMotionDetector.h
    include/BaseComm.h
    include/BTComm.h
    include/bluetoothhandler.h
    3rd/tinyxml/tinystr.h
    3rd/tinyxml/tinyxml.h
    include/translator.h
    include/uinput.h
    include/kbdinputinjector.h
    include/Buttons.h
)

set(SRC_FILES
    src/maincontroller.cpp
    src/ocrhandler.cpp
    src/textpage.cpp
    src/paragraph.cpp
    src/hwhandler.cpp
    src/hwhandler.h
    src/zyrlocamera.cpp
    src/OFMotionDetector.cpp
    src/BaseComm.cpp
    src/BTComm.cpp
    src/bluetoothhandler.cpp
    src/kbdinputinjector.cpp

    src/cerence/cerencetts.cpp
    src/cerence/cerencetts.h
    src/cerence/cerencetts_const.h
    src/cerence/positionmapper.cpp
    src/cerence/positionmapper.h

#    src/espeak/espeaktts.cpp
#    src/espeak/espeaktts.h

    src/zyrlotts.cpp
    src/zyrlotts.h

    src/ttsaudiolayer.cpp
    src/ttsaudiolayer.h

    3rd/tinyxml/tinystr.cpp
    3rd/tinyxml/tinyxml.cpp
    3rd/tinyxml/tinyxmlerror.cpp
    3rd/tinyxml/tinyxmlparser.cpp
    src/translator.cpp
    src/usbcomm.cpp
    src/usbcomm.h
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
    find_package(OpenCV REQUIRED PATHS "/home/dilshodm/work/proj/upwork/leon/opencv")
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
        Qt${QT_VERSION_MAJOR}::Bluetooth
        ${OpenCV_LIBS}
        opencv_world
        wiringPi
        bluetooth
        mp3lame
 #       espeak

    PRIVATE
        ${ZYRLO_OCR_LIB}
        ${VE_LIB}
        tts_ve
)

target_include_directories(${LIB_NAME}
    PUBLIC
        include
        ${OpenCV_INCLUDE_DIRS}/..
        3rd/tinyxml
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rd/doctest
        3rd/locate
)

target_link_directories(${LIB_NAME}
    PUBLIC
        /opt/vc/lib
)
