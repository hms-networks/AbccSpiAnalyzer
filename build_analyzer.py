#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os, glob, platform
import subprocess

"""
Builds the plugin for the current platform and when appropriate for both x86
and x64 architectures.
"""

# If MSBUild is not included in system path, full path can be specified here
MSBUILD_EXE = "MSBuild.exe"
VS_PROJECT_PATH = "project/vs2019/AbccSpiAnalyzer.vcxproj"
RELEASE_TARGET = "-p:Configuration=Release"
DEBUG_TARGET = "-p:Configuration=Debug"
X64_ARCH = "-p:Platform=x64"
X86_ARCH = "-p:Platform=x86"

def _build() -> None:
    build_error = False

    # Find out if we're running on mac or linux and set the dynamic library extension
    dylib_ext = ""
    arch = ""
    if platform.system().lower() == "windows":
        dylib_ext = ".dll"
        tmp = platform.architecture()
        if tmp[0] == "64bit":
            arch = "64"
    elif platform.system().lower() == "darwin":
        dylib_ext = ".dylib"
    else:
        dylib_ext = ".so"
        tmp = platform.architecture()
        if tmp[0] == "64bit":
            arch = "64"

    print("Running on " + platform.system())

    release_path = ""

    # Make sure the release folder exists
    if not os.path.exists( "plugins" ):
        os.makedirs( "plugins" )
    if platform.system().lower() == "windows":
        # Simply call MSBuild for each pre-configured build target in the
        # pre-configured project.
        if arch == "64":
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
    elif platform.system().lower() == "darwin":
        release_path = "./plugins/OSX/"
        if not os.path.exists( release_path ):
            os.makedirs( release_path )
            print("Made OS plugin directory")
    else:
        # Regardless of architecture always build the 32-bit variant
        release_path32 = "./plugins/Linux32/"
        if not os.path.exists( release_path32 ):
            os.makedirs( release_path32 )
            print("Made 32-bit OS plugin directory")
        if arch == "64":
            release_path = "./plugins/Linux64/"
            if not os.path.exists( release_path ):
                os.makedirs( release_path )
                print("Made 64-bit OS plugin directory")
        else:
            release_path = "./plugins/Linux32/"



    if platform.system().lower() != "darwin":
        if arch == "64":
            # For 64-bit linux systems, perform an additional clean on the 32-bit release folder
            os.chdir( release_path32 )
            o_files = glob.glob( "*.o" )
            o_files.extend( glob.glob( "*" + dylib_ext ) )
            for o_file in o_files:
                os.remove( o_file )
            os.chdir( "../.." )

    # Clean the release folder of .o/.so object files
    os.chdir( release_path )
    o_files = glob.glob( "*.o" )
    o_files.extend( glob.glob( "*" + dylib_ext ) )
    for o_file in o_files:
        os.remove( o_file )

    # Make sure the debug folder exists
    debug_path = ""
    if not os.path.exists( "Debug" ):
        os.makedirs( "Debug" )
    os.chdir( "Debug" )
    debug_path = release_path + "Debug/"

    # Clean the debug folder of .o/.so object files
    o_files = glob.glob( "*.o" )
    o_files.extend( glob.glob( "*" + dylib_ext ) )
    for o_file in o_files:
        os.remove( o_file )
    os.chdir( "../../.." )

    # Find all the cpp files in /source folder
    os.chdir( "source" )
    cpp_files = glob.glob( "*.cpp" )
    os.chdir( ".." )

    # Specify the search paths/dependencies/options for gcc
    include_paths = [ "./sdk/release/include" ]
    link_paths = [ "./sdk/release/lib" ]
    include_paths_dbg = [ "./sdk/debug/include" ]
    link_paths_dbg = [ "./sdk/debug/lib" ]

    if arch == "64":
        link_dependencies = [ "-lAnalyzer" + arch ]
        link_dependencies32 =  [ "-lAnalyzer" ]
    else:
        link_dependencies = [ "-lAnalyzer" ] # Refers to libAnalyzer.dylib or libAnalyzer.so

    debug_compile_flags = "-O0 -w -c -fpic -g -std=c++11"
    release_compile_flags = "-O3 -w -c -fpic -std=c++11"

    # Loop through all the cpp files, build up the gcc command line, and attempt to compile each cpp file
    for cpp_file in cpp_files:
        command = "g++ "
        command_dbg = "g++ "

        #Include paths
        for path in include_paths:
            command += "-I\"" + path + "\" "

        for path in include_paths_dbg:
            command_dbg += "-I\"" + path + "\" "

        release_command = command
        release_command += release_compile_flags
        release_command += " -o " + "\"" + release_path + cpp_file.replace( ".cpp", ".o" ) + "\"" #the output file
        release_command += " \"" + "source/" + cpp_file + "\"" #the cpp file to compile

        if dylib_ext != ".dylib":
            if arch == "64":
                release_command32 = command
                release_command32 += release_compile_flags
                release_command32 += " -m32 -o " + "\"" + release_path32 + cpp_file.replace( ".cpp", ".o" ) + "\"" #the output file
                release_command32 += " \"" + "source/" + cpp_file + "\"" #the cpp file to compile

        debug_command = command_dbg
        debug_command += debug_compile_flags
        if dylib_ext == ".dylib":
            debug_command += " -m32"
        debug_command += " -o " + "\"" + debug_path + cpp_file.replace( ".cpp", ".o" ) + "\"" #the output file
        debug_command += " \"" + "source/" + cpp_file + "\"" + " -D _DEBUG" #the cpp file to compile

        # Run the commands from the command line
        print(release_command)
        retcode = os.system(release_command)
        build_error |= (retcode != 0)
        if dylib_ext != ".dylib":
            if arch == "64":
                print(release_command32)
                retcode = os.system(release_command32)
                build_error |= (retcode != 0)
        print(debug_command)
        retcode = os.system(debug_command)
        build_error |= (retcode != 0)

    #Lastly, link
    command = "g++ "
    command_dbg = command

    #Add the library search paths
    for link_path in link_paths:
        command += "-L\"" + link_path + "\" "

    for link_path in link_paths_dbg:
        command_dbg += "-L\"" + link_path + "\" "

    command32 = command

    #Add libraries to link against
    for link_dependency in link_dependencies:
        command += link_dependency + " "
        command_dbg += link_dependency + " "

    if arch == "64":
        for link_dependency in link_dependencies32:
            command32 += link_dependency + " "


    # Make a dynamic (shared) library (.so/.dylib)
    if dylib_ext == ".dylib":
        command += "-dynamiclib "
        command_dbg += "-dynamiclib "
    else:
        command += "-shared "
        command32 += "-shared "
        command_dbg += "-shared "

    # Figure out what the name of this analyzer is
    analyzer_name = ""
    for cpp_file in cpp_files:
        if cpp_file.endswith( "Analyzer.cpp" ):
            analyzer_name = cpp_file.replace( "Analyzer.cpp", "" )
            break

    # The files to create (.so/.dylib files)
    if dylib_ext == ".dylib":
        release_command = command + "-o \"" + release_path + "lib" + analyzer_name + "Analyzer.dylib\" "
        debug_command = command_dbg + "-m32 -o \"" + debug_path + "lib" + analyzer_name + "Analyzer.dylib\" "
    else:
        release_command = command + "-o \"" + release_path + "lib" + analyzer_name + "Analyzer" + arch + ".so\" "
        if arch == "64":
            release_command32 = command32 + "-m32 " + "-o \"" + release_path32 + "lib" + analyzer_name + "Analyzer" + ".so\" "
        debug_command = command_dbg + "-o \"" + debug_path + "lib" + analyzer_name + "Analyzer" + arch + ".so\" "

    # Add all the object files to link
    for cpp_file in cpp_files:
        release_command += "\"" + release_path + cpp_file.replace( ".cpp", ".o" ) + "\" "
        if arch == "64":
            release_command32 += "\"" + release_path32 + cpp_file.replace( ".cpp", ".o" ) + "\" "
        debug_command += "\"" + debug_path + cpp_file.replace( ".cpp", ".o" ) + "\" "

    # Run the commands from the command line
    print( release_command )
    retcode = os.system(release_command)
    build_error |= (retcode != 0)
    if dylib_ext != ".dylib":
        if arch == "64":
            print( release_command32 )
            retcode = os.system(release_command32)
            build_error |= (retcode != 0)
        print( debug_command )
        retcode = os.system(debug_command)
        build_error |= (retcode != 0)
    else:
        # Only build 32-bit debug library on versions prior to 10.14,
        # newer versions of XCode do not support compiling for 32-bit.
        ver, _, _ = platform.mac_ver()
        ver = float('.'.join(ver.split('.')[:2]))
        if ver < 10.14:
            print( debug_command )
            retcode = os.system(debug_command)
            build_error |= (retcode != 0)

    exit(build_error)

if __name__ == "__main__":
    _build()

