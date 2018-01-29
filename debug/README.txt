To debug (in Windows using Visual Studio), ensure that the "standalone" 32-bit version of Logic 1.1.18 is placed in this folder under ./Logic

This instance of the logic software is capable of being debugged using Visual Studio's "attach to process" functionality. The debug configuration (32-bit only) will automatically launch the software and attach to the process.

Example Hierarchy (Windows):
- ./Logic/Analyzers/
- ./Logic/Drivers/
- ./Logic/Analyzers/
- ./Logic/Analyzer.dll
- ./Logic/Logic.exe
- ./README.txt (this readme)
