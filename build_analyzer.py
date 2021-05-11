#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import glob
import platform
import subprocess

"""
Builds the plugin for the current platform and when appropriate for both x86
and x64 architectures.
"""

# If MSBuild is not included in system path, full path can be specified here
MSBUILD_EXE = "MSBuild.exe"
VS_PROJECT_PATH = "project/vs2019/AbccSpiAnalyzer.vcxproj"
RELEASE_TARGET = "-p:Configuration=Release"
DEBUG_TARGET = "-p:Configuration=Debug"
X64_ARCH = "-p:Platform=x64"
X86_ARCH = "-p:Platform=x86"

GNU_CPP_STD = "c++17"

MAC_EXT = ".dylib"
WIN_EXT = ".dll"
LINUX_EXT = ".so"
CPP_EXT = ".cpp"
OBJ_EXT = ".o"

COMPILER = "g++ "
CROSS_COMPILE_32BIT_FLAG = "-m32 "
DYNAMIC_LIB_FLAG = "-dynamiclib "
SHARED_LIB_FLAG = "-shared "

DEBUG_FOLDER = "Debug"

# Specify the search paths/dependencies/options for gcc
INCLUDE_PATHS = ["./sdk/release/include"]
LINK_PATHS = ["./sdk/release/lib"]
INCLUDE_PATHS_DBG = ["./sdk/debug/include"]
LINK_PATHS_DBG = ["./sdk/debug/lib"]

windows_platform = False
mac_platform = False
linux_platform = False
platform_64bit = False


def _build() -> None:
    '''
    Top-level build routine. Builds the plugin for one of the supported host platforms.
    '''

    global mac_platform
    global windows_platform
    global linux_platform
    global platform_64bit

    dylib_ext = ""

    if platform.system().lower() == "windows":
        windows_platform = True
        dylib_ext = WIN_EXT

        if platform.architecture()[0] == "64bit":
            platform_64bit = True

    elif platform.system().lower() == "darwin":
        mac_platform = True
        dylib_ext = MAC_EXT

    else:
        linux_platform = True
        dylib_ext = LINUX_EXT

        if platform.architecture()[0] == "64bit":
            platform_64bit = True

    print("Running on " + platform.system())

    [release_path, release_path32] = _get_release_paths()

    _make_release_dirs(release_path, release_path32)
    _clean_release_dir(release_path, dylib_ext)

    if not mac_platform and platform_64bit:
        _clean_release_dir(release_path32, dylib_ext)

    # The routines called below call exit() and will not return.
    if windows_platform:
        _msbuild()
    else:
        _gnu_build(release_path, release_path32)


def _msbuild() -> None:
    '''
    Routine to compile the plugin using MSBuild.
    '''

    build_error = False

    # Simply call MSBuild for each pre-configured build target in the
    # pre-configured project.
    if platform_64bit:
        command = [MSBUILD_EXE, VS_PROJECT_PATH, RELEASE_TARGET, X64_ARCH]
        retcode = subprocess.call(command)
        build_error |= (retcode != 0)

        command = [MSBUILD_EXE, VS_PROJECT_PATH, DEBUG_TARGET, X64_ARCH]
        retcode = subprocess.call(command)
        build_error |= (retcode != 0)

    command = [MSBUILD_EXE, VS_PROJECT_PATH, RELEASE_TARGET, X86_ARCH]
    retcode = subprocess.call(command)
    build_error |= (retcode != 0)

    command = [MSBUILD_EXE, VS_PROJECT_PATH, DEBUG_TARGET, X86_ARCH]
    retcode = subprocess.call(command)
    build_error |= (retcode != 0)

    exit(build_error)


def _gnu_build(release_path: str, release_path32: str = None, dylib_ext: str = LINUX_EXT) -> None:
    '''
    Routine to compile the plugin using G++.

    Parameters
    ----------
    release_path: str
        The output path of the compiled plugin.
    release_path32 : str
        The path where the optional 32-bit plugin will be written to. This
        option shall only be used if on a 64-bit host platform. On a 32-bit
        platform, release_path represents the output path for the plugin.
    dylib_ext: str
        The extension used for the compiled plugin.

    Returns
    ----------
    None
    '''

    build_error = False

    if platform_64bit:
        arch = "64"
    else:
        arch = ""

    debug_path = release_path + DEBUG_FOLDER + "/"

    cpp_files = _get_cpp_file_list()

    if platform_64bit:
        link_dependencies = ["-lAnalyzer" + arch]
        link_dependencies32 = ["-lAnalyzer"]
    else:
        link_dependencies = ["-lAnalyzer"]

    debug_compile_flags = f"-O0 -w -c -fpic -g -std={GNU_CPP_STD} "
    release_compile_flags = f"-O3 -w -c -fpic -std={GNU_CPP_STD} "

    for cpp_file in cpp_files:
        #
        # Generate the command strings for compiling object files
        #
        command = COMPILER
        command_dbg = COMPILER
        obj_file = cpp_file.replace(CPP_EXT, OBJ_EXT)

        for path in INCLUDE_PATHS:
            command += f"-I\"{path}\" "

        for path in INCLUDE_PATHS_DBG:
            command_dbg += f"-I\"{path}\" "

        release_command = command
        release_command += release_compile_flags
        release_command += f"-o \"{release_path}{obj_file}\" "
        release_command += f"\"source/{cpp_file}\""

        if not mac_platform:
            if platform_64bit:
                release_command32 = command
                release_command32 += release_compile_flags
                release_command32 += f"{CROSS_COMPILE_32BIT_FLAG}-o \"{release_path32}{obj_file}\""
                release_command32 += f" \"source/{cpp_file}\""

        debug_command = command_dbg
        debug_command += debug_compile_flags

        if mac_platform:
            debug_command += CROSS_COMPILE_32BIT_FLAG

        debug_command += f"-o \"{debug_path}{obj_file}\" "
        debug_command += f"\"source/{cpp_file}\" -D _DEBUG"

        #
        # Compile object files
        #

        print(release_command)
        retcode = os.system(release_command)
        build_error |= _error_returned(retcode)

        if not mac_platform:
            if platform_64bit:
                print(release_command32)
                retcode = os.system(release_command32)
                build_error |= _error_returned(retcode)

        print(debug_command)
        retcode = os.system(debug_command)
        build_error |= _error_returned(retcode)

    #
    # Generate the command strings for linking object files
    #

    command = COMPILER
    command_dbg = command

    for link_path in LINK_PATHS:
        command += f"-L\"{link_path}\" "

    for link_path in LINK_PATHS_DBG:
        command_dbg += f"-L\"{link_path}\" "

    command32 = command

    for link_dependency in link_dependencies:
        command += link_dependency + " "
        command_dbg += link_dependency + " "

    if platform_64bit:
        for link_dependency in link_dependencies32:
            command32 += link_dependency + " "

    if mac_platform:
        command += DYNAMIC_LIB_FLAG
        command_dbg += DYNAMIC_LIB_FLAG
    else:
        command += SHARED_LIB_FLAG
        command32 += SHARED_LIB_FLAG
        command_dbg += SHARED_LIB_FLAG

    # Figure out what the name of this analyzer is
    analyzer_name = ""

    for cpp_file in cpp_files:
        if cpp_file.endswith("Analyzer.cpp"):
            analyzer_name = cpp_file.replace("Analyzer.cpp", "")
            break

    # The files to create (.so/.dylib files)
    if mac_platform:
        release_command = command + \
            f"-o \"{release_path}lib{analyzer_name}Analyzer{MAC_EXT}\" "
        debug_command = command_dbg + CROSS_COMPILE_32BIT_FLAG + \
            f"-o \"{debug_path}lib{analyzer_name}Analyzer{MAC_EXT}\" "
    else:
        release_command = command + \
            f"-o \"{release_path}lib{analyzer_name}Analyzer{arch}{LINUX_EXT}\" "

        if platform_64bit:
            release_command32 = command32 + CROSS_COMPILE_32BIT_FLAG + \
                f"-o \"{release_path32}lib{analyzer_name}Analyzer{LINUX_EXT}\" "
        debug_command = command_dbg + \
            f"-o \"{debug_path}lib{analyzer_name}Analyzer{arch}{LINUX_EXT}\" "

    # Add all the object files to link
    for cpp_file in cpp_files:
        obj_file = cpp_file.replace(CPP_EXT, OBJ_EXT)
        release_command += f"\"{release_path}{obj_file}\" "

        if platform_64bit:
            release_command32 += f"\"{release_path32}{obj_file}\" "
        debug_command += f"\"{debug_path}{obj_file}\" "

    #
    # Compile dynamic libraries
    #

    print(release_command)
    retcode = os.system(release_command)
    build_error |= _error_returned(retcode)

    if not mac_platform:
        if platform_64bit:
            print(release_command32)
            retcode = os.system(release_command32)
            build_error |= _error_returned(retcode)

        print(debug_command)
        retcode = os.system(debug_command)
        build_error |= _error_returned(retcode)

    else:
        # Only build 32-bit debug library on versions prior to 10.14,
        # newer versions of XCode do not support compiling for 32-bit.
        ver, _, _ = platform.mac_ver()
        ver = float('.'.join(ver.split('.')[:2]))

        if ver < 10.14:
            print(debug_command)
            retcode = os.system(debug_command)
            build_error |= _error_returned(retcode)

    exit(build_error)


def _error_returned(ret_code: int) -> bool:
    '''
    Determines if the return code from a system-call indicates an error.
    '''
    return ret_code != 0


def _get_cpp_file_list() -> list:
    '''
    Returns a list of all the cpp files in /source folder.
    '''
    os.chdir("source")
    cpp_files = glob.glob("*.cpp")
    os.chdir("..")

    return cpp_files


def _get_release_paths() -> list:
    '''
    Returns a list of up to two paths indicating where the compiled output
    artifacts will be written to. One a 32-bit host system there will only be
    one path. On 64-bit systems, with the exception to macOS, there will be
    two paths, one for 32-bit and one for 64-bit. The first element in the
    list will always exist and represent the path that corresponds to the
    native host operating system.
    '''
    release_path_native = None
    release_path32 = None

    if windows_platform:
        if platform_64bit:
            release_path_native = "./plugins/Win64/"
            release_path32 = "./plugins/Win32/"
        else:
            release_path_native = "./plugins/Win32/"

    elif mac_platform:
        release_path_native = "./plugins/OSX/"
    else:
        if platform_64bit:
            release_path_native = "./plugins/Linux64/"
            release_path32 = "./plugins/Linux32/"
        else:
            release_path_native = "./plugins/Linux32/"

    return [release_path_native, release_path32]


def _make_release_dirs(release_path: str, release_path32: str =None) -> None:
    '''
    The specified directory will be created if it does not exist.
    Also, if the platform is compatible with debugging, the associated debug
    path will be created if it does not already exist.

    Parameters
    ----------
    release_path: str
        The output path of the compiled plugin.
    release_path32 : str
        The path where the optional 32-bit plugin will be written to. This
        option shall only be used if on a 64-bit host platform. On a 32-bit
        platform, release_path represents the output path for the plugin.
    dylib_ext: str
        The extension used for the compiled plugin.

    Returns
    ----------
    None
    '''

    if release_path != None and not(windows_platform):
        # NOTE: Unlike other supported targets, the Windows 32-bit debug is
        # sent to the ./debug folder. The visual studio configuration handles
        # this special case.
        release_path += DEBUG_FOLDER

    if release_path32 != None:
        release_path32 += DEBUG_FOLDER

    if release_path != None and not os.path.exists(release_path):
        os.makedirs(release_path)

        if mac_platform:
            print("Made OS plugin directory")
        elif platform_64bit:
            print("Made 64-bit OS plugin directory")
        else:
            print("Made 32-bit OS plugin directory")

    if not mac_platform and release_path32 != None and not os.path.exists(release_path32):
        os.makedirs(release_path32)
        print("Made 32-bit OS plugin directory")


def _clean_release_dir(release_path: str, dylib_ext: str) -> None:
    '''
    Clean the specified folder of output artifacts.

    Parameters
    ----------
    release_path: str
        The output path of the compiled plugin to clean.
    dylib_ext: str
        The extension used for the compiled plugin.

    Returns
    ----------
    None
    '''

    if release_path == None:
        return

    cwd = os.getcwd()
    os.chdir(release_path)

    if windows_platform:
        o_files = glob.glob("./**/*.exp", recursive=True)
        o_files.extend(glob.glob("./**/*.iobj", recursive=True))
        o_files.extend(glob.glob("./**/*.ipdb", recursive=True))
        o_files.extend(glob.glob("./**/*.ilk", recursive=True))
        o_files.extend(glob.glob("./**/*.pdb", recursive=True))
        o_files.extend(glob.glob("./**/*.lib", recursive=True))
    else:
        o_files = glob.glob("./**/*.o", recursive=True)

    o_files.extend(glob.glob("./**/*" + dylib_ext, recursive=True))

    for o_file in o_files:
        os.remove(o_file)

    os.chdir(cwd)


if __name__ == "__main__":
    _build()
