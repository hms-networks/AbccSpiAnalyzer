import os, glob, platform

# Find out if we're running on mac or linux and set the dynamic library extension
dylib_ext = ""
arch = ""
if platform.system().lower() == "darwin":
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
if platform.system().lower() == "darwin":
    release_path = "./plugins/OSX/"
    if not os.path.exists( release_path ):
        os.makedirs( release_path )
        print("Made OS plugin directory")
else:
    # Regardless of architecture always build the 32-bit variant
    release_path32 = "./plugins/Linux32/"
    if not os.path.exists( release_path32 ):
        os.makedirs( release_path32 )
        print("Made OS plugin directory")
    if arch == "64":
        release_path = "./plugins/Linux64/"
        if not os.path.exists( release_path ):
            os.makedirs( release_path )
            print("Made OS plugin directory")
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
o_files = glob.glob( "*.o" );
o_files.extend( glob.glob( "*" + dylib_ext ) )
for o_file in o_files:
    os.remove( o_file )
os.chdir( "../../.." )

# Find all the cpp files in /source folder
os.chdir( "source" )
cpp_files = glob.glob( "*.cpp" );
os.chdir( ".." )

# Specify the search paths/dependencies/options for gcc
include_paths = [ "./include" ]
link_paths = [ "./lib" ]
if arch == "64":
    link_dependencies = [ "-lAnalyzer" + arch ]
    link_dependencies32 =  [ "-lAnalyzer" ]
else:
    link_dependencies = [ "-lAnalyzer" ] # Refers to libAnalyzer.dylib or libAnalyzer.so

debug_compile_flags = "-O0 -w -c -fpic -g"
release_compile_flags = "-O3 -w -c -fpic"

# Loop through all the cpp files, build up the gcc command line, and attempt to compile each cpp file
for cpp_file in cpp_files:
    command = "g++ "

    #Include paths
    for path in include_paths:
        command += "-I\"" + path + "\" "

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

    debug_command = command
    debug_command += debug_compile_flags
    debug_command += " -o " + "\"" + debug_path + cpp_file.replace( ".cpp", ".o" ) + "\"" #the output file
    debug_command += " \"" + "source/" + cpp_file + "\"" #the cpp file to compile

    # Run the commands from the command line
    print(release_command)
    os.system( release_command )
    if dylib_ext != ".dylib":
        if arch == "64":
            print(release_command32)
            os.system( release_command32 )
    print(debug_command)
    os.system( debug_command )

#Lastly, link
command = "g++ "

#Add the library search paths
for link_path in link_paths:
    command += "-L\"" + link_path + "\" "

command32 = command

#Add libraries to link against
for link_dependency in link_dependencies:
    command += link_dependency + " "

if arch == "64":
    for link_dependency in link_dependencies32:
        command32 += link_dependency + " "


# Make a dynamic (shared) library (.so/.dylib)
if dylib_ext == ".dylib":
    command += "-dynamiclib "
else:
    command += "-shared "
    command32 += "-shared "

# Figure out what the name of this analyzer is
analyzer_name = ""
for cpp_file in cpp_files:
    if cpp_file.endswith( "Analyzer.cpp" ):
        analyzer_name = cpp_file.replace( "Analyzer.cpp", "" )
        break

# The files to create (.so/.dylib files)
if dylib_ext == ".dylib":
    release_command = command + "-o \"" + release_path + "lib" + analyzer_name + "Analyzer.dylib\" "
    debug_command = command + "-o \"" + debug_path + "lib" + analyzer_name + "Analyzer.dylib\" "
else:
    release_command = command + "-o \"" + release_path + "lib" + analyzer_name + "Analyzer" + arch + ".so\" "
    if arch == "64":
        release_command32 = command32 + "-m32 " + "-o \"" + release_path32 + "lib" + analyzer_name + "Analyzer" + ".so\" "
    debug_command = command + "-o \"" + debug_path + "lib" + analyzer_name + "Analyzer" + arch + ".so\" "

# Add all the object files to link
for cpp_file in cpp_files:
    release_command += "\"" + release_path + cpp_file.replace( ".cpp", ".o" ) + "\" "
    if arch == "64":
        release_command32 += "\"" + release_path32 + cpp_file.replace( ".cpp", ".o" ) + "\" "
    debug_command += "\"" + debug_path + cpp_file.replace( ".cpp", ".o" ) + "\" "

# Run the commands from the command line
print( release_command )
os.system( release_command )
if dylib_ext != ".dylib":
    if arch == "64":
        print( release_command32 )
        os.system( release_command32 )
print( debug_command )
os.system( debug_command )

