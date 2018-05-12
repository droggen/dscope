@echo off

echo Packaging...

IF "%1"=="" GOTO HAVE_0

rd /q /s windows
del /q dscopeqt-win.zip
md windows
%1\windeployqt.exe build-dscopeqt-Desktop_Qt_5_8_0_MinGW_32bit-Release\release\DScopeQT.exe --dir windows --force
copy build-dscopeqt-Desktop_Qt_5_8_0_MinGW_32bit-Release\release\DScopeQT.exe windows
copy "%1\libstdc++-6.dll" windows
copy %1\libwinpthread-1.dll windows
copy %1\libgcc_s_dw2-1.dll windows


cd windows
zip -r ..\dscopeqt-win.zip * -x *.svn*
cd..
zip -r dscopeqt-win.zip examples\* -x *.svn*

echo Packaging done

exit /b

:HAVE_0
echo Specify the path to qt. Example: %0 C:\Qt\5.8\mingw53_32\bin
