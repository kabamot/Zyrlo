cmake_minimum_required(VERSION 3.14)

project(ve_platform LANGUAGES C)

set(LIBRARY_NAME tts_ve)

# Enable multithreading in VE
add_definitions(-DEXT_MULTITHREAD)

set(INC_FILES
    platform/inc/vcharconv.h
    platform/inc/vclock.h
    platform/inc/vcritsec.h
    platform/inc/vdata.h
    platform/inc/vfile.h
    platform/inc/vfind.h
    platform/inc/vheap.h
    platform/inc/vinit.h
    platform/inc/vlog.h
    platform/inc/vmap.h
    platform/inc/vplatform.h
    platform/inc/vplatform_tchar.h
    platform/inc/vprintf.h
    platform/inc/vstream.h
    platform/inc/vthread.h
)
set(SRC_FILES
    platform/src/audiovstream.c
    platform/src/audiovstream.h
    platform/src/localvstream.c
    platform/src/localvstream.h
    platform/src/urlvstream.c
    platform/src/urlvstream.h
    platform/src/vbroker.c
    platform/src/vcharconv.c
    platform/src/vclock.c
    platform/src/vcritsec.c
    platform/src/vdata.c
    platform/src/vdatapriv.h
    platform/src/vfile.c
    platform/src/vfilelist.c
    platform/src/vfilelist.h
    platform/src/vfilestore.c
    platform/src/vfind.c
    platform/src/vfindfiles.c
    platform/src/vfindfiles.h
    platform/src/vheap.c
    platform/src/vlog.c
    platform/src/vplatform.c
    platform/src/vstream.c
    platform/src/vthread.c
)

find_package(CURL REQUIRED)

add_library(${LIBRARY_NAME} STATIC
    ${SRC_FILES}
    ${INC_FILES}
)

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/platform/inc
)

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        curl
        pthread
)
