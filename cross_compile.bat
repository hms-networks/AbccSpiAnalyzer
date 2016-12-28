@echo off
REM This batch file is intended for use on Windows 10 with "Ubuntu Bash on Windows" installed
REM On a new bash-subsystem these are the following steps needed to get started:
REM $sudo apt-get update
REM $sudo apt-get upgrade
REM $sudo apt-get install g++
REM $sudo apt-get install g++-multilib

@echo ----- Entering Ubuntu bash on Windows -----
bash --version
@echo.
@echo python build_analyzer.py
@echo.
bash -c "python build_analyzer.py"
REM pause
