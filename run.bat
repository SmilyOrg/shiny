@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\VsDevCmd.bat"

echo Compiling...
::nvcc -O2 -arch=sm_35 -o shiny.exe shiny.cpp
nvcc -O0 ^
	-g -Xcompiler -Zi ^
	-DUSE_WEBSOCKET ^
	-o bin/shiny.exe ^
	--include-path include/ ^
	--include-path src/vendor/ ^
	src/vendor/ujson/ujson.cpp ^
	src/vendor/ujson/double-conversion.cc ^
	src/vendor/civetweb/civetweb.c ^
	src/shiny.cpp ^
	-Xcompiler /openmp -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP ^
	

if %errorlevel% neq 0 exit /b %errorlevel%

echo Running...
bin\shiny