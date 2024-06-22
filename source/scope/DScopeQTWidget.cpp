/*
   DScopeQT: Digital Scope QT widget
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


#include <QPainter>
#include <QPaintEvent>
#include <QMessageBox>
#include <QSizePolicy>
#include <QMenu>
#include <QPixmap>
#include <QDateTime>
#include <QApplication>
#include <QLineEdit>
#include <QStringList>
#include <QSettings>
#include <QtDebug>
#include <QtGlobal>
#include <QFont>
#include "DScopeQTWidget.h"


////////////////////////////////////
DScopeQTWidget::DScopeQTWidget(unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha,QWidget *parent) :
    QWidget(parent),
    DScopeQT(0,0,0,_w,_h,_alpha)
{
    // create the pixmap
    //pixmap = QPixmap(_w,_h);
    pixmap = QImage(_w,_h,QImage::Format_RGB32);
    DScopeQT::Resize(&pixmap,0,0,_w,_h);
}
DScopeQTWidget::~DScopeQTWidget()
{
}

//! Implementation of painting. event is the zone to redraw
void DScopeQTWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QRect validRect = rect() & event->rect();
    p.setClipRect(validRect);
    //p.drawPixmap(basex,basey,pixmap);
    p.drawImage(basex,basey,pixmap);

}


void DScopeQTWidget::mousePressEvent ( QMouseEvent * event )
{
    switch(event->button())
    {
        case Qt::MiddleButton:
            switch(event->modifiers())
            {
                case Qt::NoModifier:
                    SetVAuto();
                    break;
                case Qt::ShiftModifier:
                    HZoomReset();
                    break;
                default:
                    event->ignore();
                    return;
            }
            break;
        default:
            event->ignore();
            return;
    }
    event->accept();
}
void DScopeQTWidget::wheelEvent (QWheelEvent * event)
{
    if(event->angleDelta().y()>0)
    {
        if(event->modifiers()==Qt::NoModifier)
            VZoomin();
        if(event->modifiers()==Qt::ShiftModifier)
            HZoomin();
    }
    else
    {
        if(event->modifiers()==Qt::NoModifier)
            VZoomout();
        if(event->modifiers()==Qt::ShiftModifier)
            HZoomout();
    }
    event->accept();
}









