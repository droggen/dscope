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
#include "DScopeQT.h"
#include "Scope.h"
#include "SDL_gfxPrimitives_font.h"


//! Constructor. channelCount is the number of channel. parent is the parent widget, if any. signalInfo is a struct shared by parent that actually holds datas
//DScopeQT::DScopeQT(QPixmap *_pixmap, unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha) :
//DScopeQT::DScopeQT(QPaintDevice *_pixmap, unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha) :
DScopeQT::DScopeQT(QImage *_pixmap, unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool _alpha) :
	Scope(_x,_y,_w,_h),
	alpha(_alpha),
	pixmap(_pixmap),
	paintdevice(_pixmap)
{

}

//! Destructor, cleanup things
DScopeQT::~DScopeQT()
{
	
}


//void DScopeQT::Resize(QPixmap *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h)
//void DScopeQT::Resize(QPaintDevice *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h)
void DScopeQT::Resize(QImage *_pixmap,unsigned _x,unsigned _y,unsigned _w,unsigned _h)
{
	pixmap = _pixmap;
	paintdevice = _pixmap;
	Scope::Resize(_x,_y,_w,_h);
}
void DScopeQT::SetAlpha(bool _alpha)
{
	alpha=_alpha;
}


void DScopeQT::Plot(vector<int> &v,unsigned color)
{
	Scope::Plot(v,color);

}
void DScopeQT::Plot(const vector<vector<int> *> &v,const vector<unsigned> &color)
{
	painter.begin(paintdevice);	
	Scope::Plot(v,color);
	painter.end();
}
void DScopeQT::cleararea()
{
	painter.setClipRect(basex,basey,w,h);
	// Optimized version for 32-bit that keeps the alpha channel unchanged.
	if( pixmap->format()==QImage::Format_RGB32 ||
		pixmap->format()==QImage::Format_ARGB32 ||		
		pixmap->format()==QImage::Format_ARGB32_Premultiplied
		)
	{
		rgb = pixmap->bits();
		bpl = pixmap->bytesPerLine();
		unsigned char *pixel,*pixelline;
		pixel = rgb+4*basex+bpl*basey;
		
		if(alpha)
		{
			// Optimized fadeout
			for(unsigned y=0;y<h;y++)
			{	
				pixelline=pixel;
				for(unsigned x=0;x<w;x++)
				{
					// Read pixel
					unsigned p=*(unsigned *)pixelline;
					// Write divided by 2 (alpha=128) with corrected shifted bits.
					*(unsigned *)pixelline = ((p>>1)&0x7f7f7f);
					pixelline+=4;
				}
				pixel+=bpl;
			}
		}
		else
		{
			// Clear.
			for(unsigned y=0;y<h;y++)
			{	
				pixelline=pixel;
				for(unsigned x=0;x<w;x++)
				{
					// Keep alpha, clear RGB
					*(unsigned *)pixelline = 0;
					pixelline+=4;
				}
				pixel+=bpl;
			}
		}
	}
	else
	{
		// Unoptimized (non 32-bit)
		if(alpha)
		{
			// QT version of fadeout.
			painter.fillRect(basex,basey,w,h,QColor(0,0,0,208));	
		}
		else
		{
			// QT version of clear.
			painter.fillRect(basex,basey,w,h,QColor(0,0,0,255));
		}
	}
}
void DScopeQT::fastPixelColor(int x,int y,unsigned color)
{
	painter.setPen(color);
	painter.drawPoint(basex+x,basey+y);
}
void DScopeQT::lineColor(int x1,int y1,int x2,int y2,unsigned color)
{
	painter.setPen(color);
	painter.drawLine(basex+x1,basey+y1,basex+x2,basey+y2);
}
void DScopeQT::hLineColor(int x1,int y,int x2,unsigned color)
{
	//if(pixmap->format()==QImage::Format_RGB32)
	if( pixmap->format()==QImage::Format_RGB32 ||
		pixmap->format()==QImage::Format_ARGB32 ||
		pixmap->format()==QImage::Format_ARGB32_Premultiplied
		)
	//if(0)
	{
		int xtmp;
		// Sort
		if (x1 > x2) {
			xtmp = x1;
			x1 = x2;
			x2 = xtmp;
		}
		//Check visibility of vline 
		if((y<0) || (y>=h)) 
			return;
		if((x1<0) && (x2<0))
			return;
		if((x1>=w) && (x2>=w))
			return;
		// Clip y 
		if(x1<0)
			x1 = 0;
		if(x2>=w)
			x2=w-1;
		// Rebase
		x1+=basex;
		x2+=basex;
		y+=basey;
		

		unsigned char *pixel,*pixellast;
		pixel = rgb + 4*x1 + bpl*y;
		pixellast = pixel + 4*(x2-x1);
		for(;pixel<=pixellast;pixel+=4)
			*(unsigned *)pixel=color;
	}
	else
	{
		lineColor(x1,y,x2,y,color);	
	}
}
void DScopeQT::fasthLineColor(int x1, int y, int x2, unsigned color)
{
	hLineColor(x1,y,x2,color);
	//lineColor(x1,y,x2,y,color);
}
void DScopeQT::vLineColor(int x,int y1,int y2,unsigned color)
{
		
	//if(pixmap->format()==QImage::Format_RGB32)
	if( pixmap->format()==QImage::Format_RGB32 ||
		pixmap->format()==QImage::Format_ARGB32 ||
		pixmap->format()==QImage::Format_ARGB32_Premultiplied
		)
	//if(0)
	{
		int ytmp;
		// Sort
		if (y1 > y2) {
			ytmp = y1;
			y1 = y2;
			y2 = ytmp;
		}
		//Check visibility of vline 
		if((x<0) || (x>=w)) 
			return;
		if((y1<0) && (y2<0))
			return;
		if((y1>=h) && (y2>=h))
			return;
		// Clip y 
		if(y1<0)
			y1 = 0;
		if(y2>=h)
			y2=h-1;
		// Rebase
		x+=basex;
		y1+=basey;
		y2+=basey;
		// Height
		int h=y2-y1;
		

		//int bpl = 1;
		unsigned char *pixel,*pixellast;
		pixel = rgb + 4*x + bpl*y1;
		pixellast = pixel + bpl*h;
		for(;pixel<=pixellast;pixel+=bpl)
			*(unsigned *)pixel=color;
	}
	else
	{
		//painter.fillRect(x1,y1,1,y2-y1+1,QColor(color));
		lineColor(x,y1,x,y2,color);	
	}

//	lineColor(x,y1,x1,y2,color);	
		
	

	
	
}
void DScopeQT::fastvLineColor(int x, int y1, int y2, unsigned color)
{
	vLineColor(x,y1,y2,color);
	//lineColor(x,y1,x,y2,color);
}
void DScopeQT::fastStringColor(int x,int y,const char *s,unsigned color)
{
/*	painter.setPen(color);
	QFont f("Helvetica");
	f.setPixelSize(11);
	//f.setFixedPitch(true);
	painter.setFont(f);
	painter.drawText(basex+x,basey+y,s);
	
	return;*/
	
	//return;
	
	// Empty string
	if(*s==0)
		return;	
	if(x<0 || y<0)
		return;
	if(y+7>=h)
		return;
	if(x+8*strlen(s)>=w)
		return;
	if(pixmap->format()!=QImage::Format_RGB32)	// RGB32 is optimized
		painter.setPen(color);
	do
	{
		icharacterColor(x,y,*s,color);
		x+=8;
	}
	while(*++s);
	
}
void DScopeQT::icharacterColor(int x,int y,char c,unsigned color)
{
	unsigned char *data;
	data = gfxPrimitivesFontdata+c*8;
	unsigned char mask;
	
	//if(pixmap->format()==QImage::Format_RGB32)
	if( pixmap->format()==QImage::Format_RGB32 ||
		pixmap->format()==QImage::Format_ARGB32 ||
		pixmap->format()==QImage::Format_ARGB32_Premultiplied
		)
	//if(0)
	{
		x+=basex;
		y+=basey;
		unsigned pitch = pixmap->bytesPerLine();
		unsigned char *p = pixmap->bits() + y*pitch + x*4;
		for (int iy = 0; iy < 8; iy++)
		{
			mask=0x80;
			for (int ix = 0; ix < 8; ix++)
			{
				if(*data&mask)
				{
					*(unsigned*)p=color;
				}
				mask>>=1;
				p+=4;
			}
			data++;
			p+=pitch-32;
		}
	}
	else
	{
			
		
		for (int iy = 0; iy < 8; iy++)
		{
			mask=0x80;
			for (int ix = 0; ix < 8; ix++)
			{
				if(*data&mask)
				{
					painter.drawPoint(basex+x+ix,basey+y+iy);
				}
				mask>>=1;
		    }
		    data++;
		}
	}
}
unsigned DScopeQT::fastColor(unsigned color)
{
	return color;
	/*
	if(transparent)
	{
		if(pixmap->format()==QImage::Format_ARGB32_Premultiplied)
		{
			// Premultiplied mode.
			unsigned a,r,g,b;
			//a=(color>>24)&0xff;
			a = 0xD0;
			r=(color>>16)&0xff;
			g=(color>>8)&0xff;
			b=color&0xff;
			//printf("Color %X. argb: %X %X %X %X. ",color,a,r,g,b);
			r = (r*a)/255;
			g = (g*a)/255;
			b = (b*a)/255;
			unsigned c;
			c=(a<<24) | (r<<16) | (g<<8) | (b);
			//printf("scaled color: %X %X %X. result: %X\n",r,g,b,c);
			return c;			
		}
		else
		{
			return 0xD0000000 | (color&0x00FFFFFF);
		}
	}
	else
		return 0xFF000000 | color;*/
}

void DScopeQT::fastStart()
{
	//printf("S %p. T: %d. A: %d. px: %p f:%d x,y,w,h: %d,%d %d,%d\n",this,transparent,alpha,pixmap,pixmap->format(),basex,basey,w,h);
	rgb = pixmap->bits();
	bpl = pixmap->bytesPerLine();
}
void DScopeQT::fastStop()
{
	return;
}


unsigned DScopeQT::ColorBlack()
{
		return 0xff000000;
}
unsigned DScopeQT::ColorWhite()
{

		return 0xffffffff;
}
unsigned DScopeQT::ColorGrey()
{

		return 0xff7f7f7f;
}
unsigned DScopeQT::ColorLGrey()
{
		return 0xff3f3f3f;
}
unsigned DScopeQT::ColorRed()
{
	return 0xffff0000;
}






