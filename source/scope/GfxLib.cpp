/*
   GfxLib: QT version

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

// How to:
// 1. Intialize GfxSurface to point to the target widget
// 2. Call GfxInit

#include "GfxLib.h"

#include <QPainter>

QImage GfxImage;						//Internal rendering image 
QPaintDevice *GfxSurface;			// Target paint device (a widget) - must


void GfxInit()
{
	GfxImage = QImage(320,200,QImage::Format_RGB32);
	
}
void GfxGetSize(int *sx,int *sy)
{
	*sx=GfxImage.width();
	*sy=GfxImage.height();
}
void GfxPutPixel(int x,int y,int color)
{
	/*QPainter painter(&GfxImage);
	painter.setPen(color);	
	painter.*/
	GfxImage.setPixel(x,y,color);
	
}
int GfxGetPixel(int x,int y)
{
	//return WHITE;
	//QRgb rgb = 
	return GfxImage.pixel(x,y)&0xffffff;
	
	
}

void GfxFillRectangle(int x1,int y1,int sx, int sy,int color)
{
	QPainter painter(&GfxImage);
	painter.setPen(color);
	painter.fillRect(x1,y1,sx,sy,QBrush(color));
	
}
void GfxLine(int x1,int y1,int x2,int y2,int color)
{
	QPainter painter(&GfxImage);
	painter.setPen(color);
	painter.drawLine(x1,y1,x2,y2);
}
void GfxUpdate()
{
	QPainter p(GfxSurface);
	//p.drawImage(0,0,GfxImage);
	//QRect target(0,0,640,480);
	QRect target(0,0,GfxSurface->width(),GfxSurface->height());
	QRect source(0,0,GfxImage.width(),GfxImage.height());
	//p.drawImage(target,GfxImage,source,Qt::ThresholdDither);
	p.drawImage(target,GfxImage,source,Qt::DiffuseDither|Qt::AvoidDither);
	
		
}


