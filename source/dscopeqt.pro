# -------------------------------------------------
# Project created by QtCreator 2009-06-24T16:42:50
# -------------------------------------------------


QT += core gui
QT += serialport network widgets
QT += bluetooth
# Compatibility for QRegExp of Qt5


ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-source

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG += c++11


INCLUDEPATH += .
TARGET = DScopeQT
TEMPLATE = app
SOURCES += main.cpp \
    precisetimer.cpp \
    cio.cpp \
    FrameParser/FrameParser3.cpp \
    scope/Scopes.cpp \
    scope/Scope.cpp \
    scope/GfxLib.cpp \
    scope/DScopesQTWidget.cpp \
    scope/DScopesQT.cpp \
    scope/DScopeQTWidget.cpp \
    scope/DScopeQT.cpp \
    helpwindow.cpp \
    portwindow.cpp \
    mainwindow.cpp \
    helper.cpp \
    dlinereader.cpp \
    parse.cpp \
    iodev.cpp \
    btwindow.cpp \
    dterminal.cpp
HEADERS += \
    precisetimer.h \
    cio.h \
    FrameParser/FrameParser3.h \
    scope/SDL_gfxPrimitives_font.h \
    scope/Scopes.h \
    scope/Scope.h \
    scope/GfxLib.h \
    scope/DScopesQTWidget.h \
    scope/DScopesQT.h \
    scope/DScopeQTWidget.h \
    scope/DScopeQT.h \
    helpwindow.h \
    portwindow.h \
    mainwindow.h \
    helper.h \
    dlinereader.h \
    parse.h \
    iodev.h \
    btwindow.h \
    dterminal.h
FORMS += \
    helpwindow.ui \
    portwindow.ui \
    mainwindow.ui \
    btwindow.ui
OTHER_FILES += howto.html \
    dscope.rc
RESOURCES += resources.qrc


# icon
win32: RC_FILE = dscope.rc


# Developer mode
#DEFINES += DEVELMODE



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#DISTFILES += \
#   android-source/AndroidManifest.xml \
#   android-source/gradle/wrapper/gradle-wrapper.jar \
#   android-source/gradlew \
#   android-source/res/values/libs.xml \
#   android-source/build.gradle \
#   android-source/gradle/wrapper/gradle-wrapper.properties \
#   android-source/gradlew.bat

DISTFILES += \
   android-source/AndroidManifest.xml \
   android-source/res/values/libs.xml

