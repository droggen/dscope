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

#include "DScopesQTWidget.h"
#include "precisetimer.h"

#define format QImage::Format_RGB32
//#define format QImage::Format_ARGB32_Premultiplied



/*
    Benchmarks Debug
                            normal											fs
                    plot	render	paint	repaint				plot	render	paint	repaint
    ARGB32_pre		21.3	5.2		12.3	17.1				59.9	21.9	44.4	66.2
                    21		4.4		12.0	17.1				60.2	22.5	43.9	66.0

    ARGB32			21.4	6.0		12.4	17.3				59.0	22.6	43.7	66.2
                    20.9	5.0		12.2	17.1				59.4	21.9	44.4	66.1

    RGB32			21.3	18.7	14.3	30.9				59.7	14.7	52.1	67.9
                    21.0	18.7	14.4	30.8				67.9	14.9	53.6	67.9

    RGB16			16.1	12.7	20.3	33.5				46.7	24.2	75.2	99.8
                    16.2	12.8	20.2	33.5				38.9	24.8	74.7	99.5

    RGB666			26.1	9.2		23.9	33.5				77.4	19.3	91.8	108.9
                    26.1	8.9		24.1	33.4				76.7	20.1	92.4	112.5

    RGB888			31.0	13.3	19.8	33.4				97.0	24.2	74.2	99.3
                    31.0	13.2	19.8	33.4				96.7	24.1	75.4	98.2

    Benchmarks Release
                            normal											fs
                    plot	render	paint	repaint				plot	render	paint	repaint
    RGB32			10.8	3.8		12.9	17.5				42.4	17.8	48.8	66.8

    with direct string, h/vline
                            normal											fs
                    plot	render	paint	repaint				plot	render	paint	repaint
    RGB32			6.25	0				16.59				28.3
                                                                31.2


    Benchmark Release


*/


DScopesQTWidget::DScopesQTWidget(unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha,unsigned scale,QWidget *parent) :
    QWidget(parent),
    DScopesQT(&pixmap,_w/scale,_h/scale,_alpha),
    //pixmap(_w,_h),
    pixmap(_w/scale,_h/scale,format),
    //pixmap(_w,_h,QImage::Format_Indexed8),
    basex(_x),
    basey(_y),
    scale(scale)
{
    setMinimumSize(QSize(320, 200));
    setCursor(QCursor(Qt::CrossCursor));
    QPainter painter;
    painter.begin(&pixmap);
    painter.fillRect(_x,_y,w,h,Qt::black);
    painter.end();
    rt=pt=0;
}
DScopesQTWidget::~DScopesQTWidget()
{

}
void DScopesQTWidget::paintEvent(QPaintEvent *event)
{
    //printf("DScopesQTWidget::paintEvent valid rect: %d %d %d %d\n",event->rect().left(),event->rect().top(),event->rect().width(),event->rect().height());
    //double t1,t2;
    /*t1=PreciseTimer::QueryTimer();
    Render();
    t2=PreciseTimer::QueryTimer();
    rt = 0.9*rt+0.1*(t2-t1);
    */


    //t1=PreciseTimer::QueryTimer();
    QPainter p(this);
    QRect validRect = rect() & event->rect();
    p.setClipRect(validRect);

    // Must draw the image with a rescaling
    //p.drawImage(basex,basey,*surface);
    p.drawImage(basex,basey,surface->scaled(surface->width()*scale,surface->height()*scale));

    //t2=PreciseTimer::QueryTimer();
    //pt = 0.9*pt+0.1*(t2-t1);
    //printf("Render: %.04lf Paint: %.04lf Total: %.04lf\n",rt,pt,rt+pt);
    //printf("paint 1: %.5lf ms\n",(t2-t1)*1000.0);


}

void DScopesQTWidget::mousePressEvent ( QMouseEvent * event )
{
    int mousex,mousey;
    mousex=event->x()/scale;
    mousey=event->y()/scale;
    switch(event->button())
    {
        case Qt::LeftButton:
            if(mousex<basex || mousey<basey)
                break;
            if( mousex>=basex+w || mousey>=basey+h)
                break;
            IPoint(mousex-basex,mousey-basey);
            break;
        case Qt::MidButton:
            switch(event->modifiers())
            {
                case Qt::NoModifier:
                    IVZoomAuto();
                    break;
                case Qt::ShiftModifier:
                    IHZoomReset();
                    break;
                default:
                    event->ignore();
                    return;
            }
            break;
        case Qt::RightButton:
            IMagnify();
            break;
        default:
            event->ignore();
            return;
    }
    event->accept();
}
void DScopesQTWidget::wheelEvent (QWheelEvent * event)
{
    if(event->delta()>0)
    {
        if(event->modifiers()==Qt::NoModifier)
            IVZoomIn();
        if(event->modifiers()==Qt::ShiftModifier)
            IHZoomIn();
    }
    else
    {
        if(event->modifiers()==Qt::NoModifier)
            IVZoomOut();
        if(event->modifiers()==Qt::ShiftModifier)
            IHZoomOut();
    }
    event->accept();
}

/*
    RGB32 faster ARGB32
    RGB16 fast
    RGB888 slightly slower RBG16 in nonglow mode. 50% slower in glow mode.
    ARGB32_Premultiplied about same as RGB16 (much faster ARGB32)
*/
void DScopesQTWidget::resizeEvent(QResizeEvent *event)
{
    pixmap = pixmap.scaled(QSize(event->size().width()/scale,event->size().height()/scale));
    DScopesQT::Resize(&pixmap,event->size().width()/scale,event->size().height()/scale,alpha);
}

void DScopesQTWidget::setScale(unsigned scale)
{
    // Set the scale
    DScopesQTWidget::scale=scale;
    // Resize everything
    //pixmap = pixmap.scaled(QSize(size().width()/scale,size().height()/scale));
    //DScopesQT::Resize(&pixmap,size().width()/scale,size().height()/scale,alpha);

}



