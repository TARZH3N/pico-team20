# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/pico-nexmon

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/pico-nexmon/build

# Utility rule file for nexmon_firmware.

# Include any custom commands dependencies for this target.
include ioctl_test/CMakeFiles/nexmon_firmware.dir/compiler_depend.make

# Include the progress variables for this target.
include ioctl_test/CMakeFiles/nexmon_firmware.dir/progress.make

ioctl_test/CMakeFiles/nexmon_firmware:
	cd /home/ubuntu/pico-nexmon/nexmon/patches/bcm43439a0/7_95_49_2271bb6/nexmon && bash -c "source /home/ubuntu/pico-nexmon/nexmon/setup_env.sh && make w43439A0_7_95_49_00_combined.h"

nexmon_firmware: ioctl_test/CMakeFiles/nexmon_firmware
nexmon_firmware: ioctl_test/CMakeFiles/nexmon_firmware.dir/build.make
.PHONY : nexmon_firmware

# Rule to build all files generated by this target.
ioctl_test/CMakeFiles/nexmon_firmware.dir/build: nexmon_firmware
.PHONY : ioctl_test/CMakeFiles/nexmon_firmware.dir/build

ioctl_test/CMakeFiles/nexmon_firmware.dir/clean:
	cd /home/ubuntu/pico-nexmon/build/ioctl_test && $(CMAKE_COMMAND) -P CMakeFiles/nexmon_firmware.dir/cmake_clean.cmake
.PHONY : ioctl_test/CMakeFiles/nexmon_firmware.dir/clean

ioctl_test/CMakeFiles/nexmon_firmware.dir/depend:
	cd /home/ubuntu/pico-nexmon/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/pico-nexmon /home/ubuntu/pico-nexmon/ioctl_test /home/ubuntu/pico-nexmon/build /home/ubuntu/pico-nexmon/build/ioctl_test /home/ubuntu/pico-nexmon/build/ioctl_test/CMakeFiles/nexmon_firmware.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : ioctl_test/CMakeFiles/nexmon_firmware.dir/depend

