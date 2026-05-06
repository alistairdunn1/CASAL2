@echo off
setlocal enabledelayedexpansion
cd BuildSystem

rem call doBuild.bat thirdparty
rem call doBuild.bat thirdparty boost
rem call doBuild.bat thirdparty adolc
rem call doBuild.bat thirdparty betadiff
rem call doBuild.bat thirdparty parser

call doBuild.bat version
if errorlevel 1 ( cd .. & exit /b 1 )

call doBuild.bat release
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat release betadiff
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat release adolc
if errorlevel 1 ( cd .. & exit /b 1 )

call doBuild.bat library release
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat library betadiff
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat library adolc
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat library test
if errorlevel 1 ( cd .. & exit /b 1 )

call doBuild.bat test
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat frontend
if errorlevel 1 ( cd .. & exit /b 1 )

call doBuild.bat documentation
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat rlibrary
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat archive
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat installer
if errorlevel 1 ( cd .. & exit /b 1 )

call doBuild.bat modelrunner
if errorlevel 1 ( cd .. & exit /b 1 )
call doBuild.bat unittests
if errorlevel 1 ( cd .. & exit /b 1 )

cd ..
