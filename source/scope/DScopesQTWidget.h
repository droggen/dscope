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


#ifndef __DSCOPESQTWIDGET_H
#define __DSCOPESQTWIDGET_H

#include <QWidget>
#include <QColor>
#include <QPoint>
#include <QRect>
#include <QPicture>
#include <QImage>
#include <QPixmap>
#include <QPainter>
//#include <QtOpenGl/QGLWidget>
#include <valarray>
#include <vector>
#include <map>

#include "DScopesQT.h"





class DScopesQTWidget : public QWidget, public DScopesQT
//class DScopesQTWidget : public QGLWidget, public DScopesQT
{
	Q_OBJECT
	

public:
    DScopesQTWidget(unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha=false,unsigned scale=1,QWidget *parent = 0);
	~DScopesQTWidget();
	double rt;
	double pt;

    void setScale(unsigned scale);
private:
	//QPixmap pixmap;
	QImage pixmap;
	unsigned basex,basey;
    unsigned scale;
	

	

protected:
	// events
	virtual void paintEvent(QPaintEvent *event);
	virtual void mousePressEvent ( QMouseEvent * event );
	virtual void wheelEvent (QWheelEvent * event);
	virtual void resizeEvent(QResizeEvent *event);

};




#endif
