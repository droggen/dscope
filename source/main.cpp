/*
   DScope Application

   Copyright (C) 2008,2009:
         Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QApplication>
#include "mainwindow.h"
#include <QStyleFactory>

int main(int argc, char *argv[])
{


    qputenv("QT_USE_ANDROID_NATIVE_STYLE", "0");

    QStringList style = QStyleFactory::keys();
    for(int i=0;i<style.size();i++)
    {
        qDebug("Style: %s\n",style[i].toStdString().c_str());
    }

    //QApplication::setStyle(QStyleFactory::create("android"));       // Crash on android
    //QApplication::setStyle(QStyleFactory::create("windowsVista"));
    //QApplication::setStyle(QStyleFactory::create("Windows"));
    //QApplication::setStyle(QStyleFactory::create("Fusion"));
    //QApplication::setStyle(QStyleFactory::create("Material"));

    QApplication a(argc, argv);

#ifdef Q_OS_ANDROID
    // Helps increase size
    a.setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Ceil);
#endif

    MainWindow w;
    w.show();
    return a.exec();
}

