# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2020.3\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2020.3\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\projects\pge\source

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\projects\pge\source\cmake-build-debug

# Include any dependencies generated for this target.
include PGEEditor\CMakeFiles\pge_editor.dir\depend.make

# Include the progress variables for this target.
include PGEEditor\CMakeFiles\pge_editor.dir\progress.make

# Include the compile flags for this target's objects.
include PGEEditor\CMakeFiles\pge_editor.dir\flags.make

PGEEditor\CMakeFiles\pge_editor.dir\src\main.cpp.obj: PGEEditor\CMakeFiles\pge_editor.dir\flags.make
PGEEditor\CMakeFiles\pge_editor.dir\src\main.cpp.obj: ..\PGEEditor\src\main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\projects\pge\source\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object PGEEditor/CMakeFiles/pge_editor.dir/src/main.cpp.obj"
	cd C:\projects\pge\source\cmake-build-debug\PGEEditor
	C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1427~1.291\bin\Hostx86\x86\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoCMakeFiles\pge_editor.dir\src\main.cpp.obj /FdCMakeFiles\pge_editor.dir\ /FS -c C:\projects\pge\source\PGEEditor\src\main.cpp
<<
	cd C:\projects\pge\source\cmake-build-debug

PGEEditor\CMakeFiles\pge_editor.dir\src\main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/pge_editor.dir/src/main.cpp.i"
	cd C:\projects\pge\source\cmake-build-debug\PGEEditor
	C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1427~1.291\bin\Hostx86\x86\cl.exe > CMakeFiles\pge_editor.dir\src\main.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\projects\pge\source\PGEEditor\src\main.cpp
<<
	cd C:\projects\pge\source\cmake-build-debug

PGEEditor\CMakeFiles\pge_editor.dir\src\main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/pge_editor.dir/src/main.cpp.s"
	cd C:\projects\pge\source\cmake-build-debug\PGEEditor
	C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1427~1.291\bin\Hostx86\x86\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\pge_editor.dir\src\main.cpp.s /c C:\projects\pge\source\PGEEditor\src\main.cpp
<<
	cd C:\projects\pge\source\cmake-build-debug

# Object files for target pge_editor
pge_editor_OBJECTS = \
"CMakeFiles\pge_editor.dir\src\main.cpp.obj"

# External object files for target pge_editor
pge_editor_EXTERNAL_OBJECTS =

PGEEditor\pge_editor.exe: PGEEditor\CMakeFiles\pge_editor.dir\src\main.cpp.obj
PGEEditor\pge_editor.exe: PGEEditor\CMakeFiles\pge_editor.dir\build.make
PGEEditor\pge_editor.exe: External\imgui\imgui.lib
PGEEditor\pge_editor.exe: PGEOS\pge_os.lib
PGEEditor\pge_editor.exe: PGEGraphics\pge_graphics.lib
PGEEditor\pge_editor.exe: PGEEditor\CMakeFiles\pge_editor.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\projects\pge\source\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable pge_editor.exe"
	cd C:\projects\pge\source\cmake-build-debug\PGEEditor
	"C:\Program Files\JetBrains\CLion 2020.3\bin\cmake\win\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\pge_editor.dir --rc=C:\PROGRA~2\WI3CF2~1\10\bin\100177~1.0\x86\rc.exe --mt=C:\PROGRA~2\WI3CF2~1\10\bin\100177~1.0\x86\mt.exe --manifests  -- C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1427~1.291\bin\Hostx86\x86\link.exe /nologo @CMakeFiles\pge_editor.dir\objects1.rsp @<<
 /out:pge_editor.exe /implib:pge_editor.lib /pdb:C:\projects\pge\source\cmake-build-debug\PGEEditor\pge_editor.pdb /version:0.0  /machine:X86 /debug /INCREMENTAL /subsystem:console  d3d11.lib ..\External\imgui\imgui.lib ..\PGEOS\pge_os.lib ..\PGEGraphics\pge_graphics.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<
	cd C:\projects\pge\source\cmake-build-debug

# Rule to build all files generated by this target.
PGEEditor\CMakeFiles\pge_editor.dir\build: PGEEditor\pge_editor.exe

.PHONY : PGEEditor\CMakeFiles\pge_editor.dir\build

PGEEditor\CMakeFiles\pge_editor.dir\clean:
	cd C:\projects\pge\source\cmake-build-debug\PGEEditor
	$(CMAKE_COMMAND) -P CMakeFiles\pge_editor.dir\cmake_clean.cmake
	cd C:\projects\pge\source\cmake-build-debug
.PHONY : PGEEditor\CMakeFiles\pge_editor.dir\clean

PGEEditor\CMakeFiles\pge_editor.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\projects\pge\source C:\projects\pge\source\PGEEditor C:\projects\pge\source\cmake-build-debug C:\projects\pge\source\cmake-build-debug\PGEEditor C:\projects\pge\source\cmake-build-debug\PGEEditor\CMakeFiles\pge_editor.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : PGEEditor\CMakeFiles\pge_editor.dir\depend

