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


#ifndef __DSCOPEQT_H
#define __DSCOPEQT_H

#include <QWidget>
#include <QColor>
#include <QPoint>
#include <QRect>
#include <QPicture>
#include <QPainter>
#include <valarray>
#include <vector>
#include <map>

#include "Scope.h"



class DScopeQT : public Scope
{
public:
	//DScopeQT(QPixmap *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha=false);
	//DScopeQT(QPaintDevice *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha=false);
	DScopeQT(QImage *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha=false);
	~DScopeQT();
	 
	 

    virtual void Plot(std::vector<int> &v,unsigned color=0xffffff);
    virtual void Plot(const std::vector<std::vector<int> *> &v,const std::vector<unsigned> &color);
	//virtual void Resize(QPixmap *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h);
	//virtual void Resize(QPaintDevice *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h);
	virtual void Resize(QImage *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h);
	virtual void SetAlpha(bool _alpha);
	//virtual void SetTransparency(bool _transparent);
	
	
	unsigned getx() {
		return basex;};
	unsigned gety() {
		return basey;
	};
	
	//bool transparent;
		
protected:
// Scope 
	virtual void cleararea();
	virtual void fastPixelColor(int x,int y,unsigned color);
	virtual void lineColor(int x1,int y1,int x2,int y2,unsigned color=0xffffff);
	virtual void hLineColor(int x1,int y1,int x2,unsigned color=0xffffff);
	virtual void fasthLineColor(int x1, int y, int x2, unsigned color);
	virtual void vLineColor(int x1,int y1,int y2,unsigned color=0xffffff);
	virtual void fastvLineColor(int x, int y1, int y2, unsigned color);
	virtual void fastStringColor(int x,int y,const char *s,unsigned color=0xffffff);
	virtual void fastStart();
	virtual unsigned fastColor(unsigned color);
	virtual void fastStop();
	virtual void icharacterColor(int x,int y,char c,unsigned color);
	
	
	
private:
	bool alpha;

	QPicture picture;
	QPainter painter;
	//QPixmap *pixmap;
	//QPaintDevice *pixmap;
	QImage *pixmap;
	unsigned char *rgb;
	int bpl;
	
	QPaintDevice *paintdevice;

	virtual unsigned ColorBlack();
	virtual unsigned ColorWhite();
	virtual unsigned ColorGrey();
	virtual unsigned ColorLGrey();
	virtual unsigned ColorRed();	
};




#endif
