@echo off
del /q dscopeqt-src.zip
zip -r dscopeqt-src.zip source\* -x *.svn*
zip -r dscopeqt-src.zip examples\* -x *.svn*
zip -r dscopeqt-src.zip LICENSE
zip -r dscopeqt-src.zip README.md
