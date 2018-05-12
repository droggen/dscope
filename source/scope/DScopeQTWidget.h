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


#ifndef __DSCOPEQTWIDGET_H
#define __DSCOPEQTWIDGET_H

#include <QWidget>
#include <QColor>
#include <QPoint>
#include <QRect>
#include <QPicture>
#include <QPainter>
#include <valarray>
#include <vector>
#include <map>



#include "DScopeQT.h"


class DScopeQTWidget : public QWidget, public DScopeQT
{
	Q_OBJECT
	

public:
	DScopeQTWidget(unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha=false,QWidget *parent = 0);
	~DScopeQTWidget();

private:
	//QPixmap pixmap;
	QImage pixmap;

protected:

	

	// events
	void paintEvent(QPaintEvent *event);
	void mousePressEvent ( QMouseEvent * event );
	void wheelEvent (QWheelEvent * event);
		

};






#endif
