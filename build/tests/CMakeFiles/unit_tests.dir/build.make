# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lev/Zyrlo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lev/Zyrlo/build

# Include any dependencies generated for this target.
include tests/CMakeFiles/unit_tests.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/unit_tests.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/unit_tests.dir/flags.make

tests/CMakeFiles/unit_tests.dir/main.cpp.o: tests/CMakeFiles/unit_tests.dir/flags.make
tests/CMakeFiles/unit_tests.dir/main.cpp.o: ../tests/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lev/Zyrlo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/unit_tests.dir/main.cpp.o"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/unit_tests.dir/main.cpp.o -c /home/lev/Zyrlo/tests/main.cpp

tests/CMakeFiles/unit_tests.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/unit_tests.dir/main.cpp.i"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lev/Zyrlo/tests/main.cpp > CMakeFiles/unit_tests.dir/main.cpp.i

tests/CMakeFiles/unit_tests.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/unit_tests.dir/main.cpp.s"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lev/Zyrlo/tests/main.cpp -o CMakeFiles/unit_tests.dir/main.cpp.s

tests/CMakeFiles/unit_tests.dir/test_textpage.cpp.o: tests/CMakeFiles/unit_tests.dir/flags.make
tests/CMakeFiles/unit_tests.dir/test_textpage.cpp.o: ../tests/test_textpage.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lev/Zyrlo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object tests/CMakeFiles/unit_tests.dir/test_textpage.cpp.o"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/unit_tests.dir/test_textpage.cpp.o -c /home/lev/Zyrlo/tests/test_textpage.cpp

tests/CMakeFiles/unit_tests.dir/test_textpage.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/unit_tests.dir/test_textpage.cpp.i"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lev/Zyrlo/tests/test_textpage.cpp > CMakeFiles/unit_tests.dir/test_textpage.cpp.i

tests/CMakeFiles/unit_tests.dir/test_textpage.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/unit_tests.dir/test_textpage.cpp.s"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lev/Zyrlo/tests/test_textpage.cpp -o CMakeFiles/unit_tests.dir/test_textpage.cpp.s

tests/CMakeFiles/unit_tests.dir/test_paragraph.cpp.o: tests/CMakeFiles/unit_tests.dir/flags.make
tests/CMakeFiles/unit_tests.dir/test_paragraph.cpp.o: ../tests/test_paragraph.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lev/Zyrlo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object tests/CMakeFiles/unit_tests.dir/test_paragraph.cpp.o"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/unit_tests.dir/test_paragraph.cpp.o -c /home/lev/Zyrlo/tests/test_paragraph.cpp

tests/CMakeFiles/unit_tests.dir/test_paragraph.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/unit_tests.dir/test_paragraph.cpp.i"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lev/Zyrlo/tests/test_paragraph.cpp > CMakeFiles/unit_tests.dir/test_paragraph.cpp.i

tests/CMakeFiles/unit_tests.dir/test_paragraph.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/unit_tests.dir/test_paragraph.cpp.s"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lev/Zyrlo/tests/test_paragraph.cpp -o CMakeFiles/unit_tests.dir/test_paragraph.cpp.s

tests/CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.o: tests/CMakeFiles/unit_tests.dir/flags.make
tests/CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.o: ../tests/test_ocrhandler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lev/Zyrlo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object tests/CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.o"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.o -c /home/lev/Zyrlo/tests/test_ocrhandler.cpp

tests/CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.i"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lev/Zyrlo/tests/test_ocrhandler.cpp > CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.i

tests/CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.s"
	cd /home/lev/Zyrlo/build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lev/Zyrlo/tests/test_ocrhandler.cpp -o CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.s

# Object files for target unit_tests
unit_tests_OBJECTS = \
"CMakeFiles/unit_tests.dir/main.cpp.o" \
"CMakeFiles/unit_tests.dir/test_textpage.cpp.o" \
"CMakeFiles/unit_tests.dir/test_paragraph.cpp.o" \
"CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.o"

# External object files for target unit_tests
unit_tests_EXTERNAL_OBJECTS =

unit_tests: tests/CMakeFiles/unit_tests.dir/main.cpp.o
unit_tests: tests/CMakeFiles/unit_tests.dir/test_textpage.cpp.o
unit_tests: tests/CMakeFiles/unit_tests.dir/test_paragraph.cpp.o
unit_tests: tests/CMakeFiles/unit_tests.dir/test_ocrhandler.cpp.o
unit_tests: tests/CMakeFiles/unit_tests.dir/build.make
unit_tests: core/libcore.a
unit_tests: /home/lev/Qt/5.15.2/gcc_64/lib/libQt5TextToSpeech.so.5.15.2
unit_tests: /home/lev/Qt/5.15.2/gcc_64/lib/libQt5Multimedia.so.5.15.2
unit_tests: /home/lev/Qt/5.15.2/gcc_64/lib/libQt5Network.so.5.15.2
unit_tests: /home/lev/Qt/5.15.2/gcc_64/lib/libQt5Gui.so.5.15.2
unit_tests: /home/lev/Qt/5.15.2/gcc_64/lib/libQt5Concurrent.so.5.15.2
unit_tests: /home/lev/Qt/5.15.2/gcc_64/lib/libQt5Core.so.5.15.2
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_stitching.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_aruco.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_bgsegm.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_bioinspired.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_ccalib.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_dnn_objdetect.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_dnn_superres.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_dpm.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_highgui.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_face.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_freetype.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_fuzzy.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_hdf.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_hfs.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_img_hash.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_line_descriptor.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_quality.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_reg.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_rgbd.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_saliency.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_shape.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_stereo.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_structured_light.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_phase_unwrapping.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_superres.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_optflow.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_surface_matching.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_tracking.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_datasets.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_plot.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_text.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_dnn.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_ml.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_videostab.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_videoio.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_viz.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_ximgproc.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_video.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_xobjdetect.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_imgcodecs.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_objdetect.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_calib3d.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_features2d.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_flann.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_xphoto.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_photo.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_imgproc.so.4.2.0
unit_tests: /usr/lib/x86_64-linux-gnu/libopencv_core.so.4.2.0
unit_tests: /mnt/hgfs/SharedFolder/ZyrloOcr/ZyrloOcr.so
unit_tests: /mnt/hgfs/SharedFolder/TTS/csdk-v2-linux_x86_64/csdk/lib/libve.so
unit_tests: core/3rd/tts_ve/libtts_ve.a
unit_tests: tests/CMakeFiles/unit_tests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lev/Zyrlo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable ../unit_tests"
	cd /home/lev/Zyrlo/build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/unit_tests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/unit_tests.dir/build: unit_tests

.PHONY : tests/CMakeFiles/unit_tests.dir/build

tests/CMakeFiles/unit_tests.dir/clean:
	cd /home/lev/Zyrlo/build/tests && $(CMAKE_COMMAND) -P CMakeFiles/unit_tests.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/unit_tests.dir/clean

tests/CMakeFiles/unit_tests.dir/depend:
	cd /home/lev/Zyrlo/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lev/Zyrlo /home/lev/Zyrlo/tests /home/lev/Zyrlo/build /home/lev/Zyrlo/build/tests /home/lev/Zyrlo/build/tests/CMakeFiles/unit_tests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/unit_tests.dir/depend

