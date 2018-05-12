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
#include "DScopesQT.h"
#include "DScopeQT.h"

#include "precisetimer.h"

DScopesQT::DScopesQT(QImage *s,unsigned _w,unsigned _h,bool _alpha)
	: Scopes(_w,_h)
{
	surface=s;
	alpha=_alpha;
	dirty=true;
}

DScopesQT::~DScopesQT()
{
	DeleteScopes();
}

void DScopesQT::Render()
{
	painter.begin(surface);
	unsigned n;


	for(unsigned i=0;i<handles.size();i++)
	{
		// do the rendering/blitting with the magnified item at last.
		if(i<hscope)
			n=i;
		else
		{
			if(i<handles.size()-1)
				n=i+1;
			else
				n=hscope;	
		}
		//printf("Render scope %d. surface[%d]=%p. %d %d\n",n,n,surfaces[n],static_cast<DScopeQT*>(scopes[n].scope)->getx(),static_cast<DScopeQT*>(scopes[n].scope)->gety());
		if(surfaces[n])	// If no surface we assume a direct draw.
		{	
			if(!transparentmagnify)
			{
				
				//double t2,t1;
				//t1 = PreciseTimer::QueryTimer();
				painter.drawImage(scopes[n].x,scopes[n].y,*surfaces[n]);
				//t2 = PreciseTimer::QueryTimer();
				//printf("Drawimage time: %.3lf ms. Format of src: %d. Format of dst: %d\n",(t2-t1)*1000,surfaces[n]->format(),surface->format());
			}
			else
			{
				//double t2,t1;
				//t1 = PreciseTimer::QueryTimer();
				
				// Render with transparency.
				unsigned w,h;
				w=surfaces[n]->width();
				h=surfaces[n]->height();
				
				unsigned char *rgbd = surface->bits();
				unsigned bpld = surface->bytesPerLine();
				unsigned char *rgbs = surfaces[n]->bits();
				unsigned bpls = surfaces[n]->bytesPerLine();
				
				rgbd += 4*scopes[n].x + bpld*scopes[n].y;
				
				for(unsigned y=0;y<h;y++)
				{
					unsigned *ppd,*pps;
					ppd=(unsigned*)rgbd;		// Pointer to destination
					pps=(unsigned*)rgbs;		// Pointer to source
					for(unsigned x=0;x<w;x++)
					{
						unsigned rs,gs,bs,ps;
						//*ppd=*pps;
						ps=*pps;
						rs=(ps>>16)&0xff;
						gs=(ps>>8)&0xff;
						bs=ps&0xff;
						unsigned rd,gd,bd,pd;
						pd=*ppd;
						rd=(pd>>16)&0xff;
						gd=(pd>>8)&0xff;
						bd=pd&0xff;
						
						rd = (rs+rs+rs+rd)>>2;
						gd = (gs+gs+gs+gd)>>2;
						bd = (bs+bs+bs+bd)>>2;
						//rd = (rs+rs+rs+rs+rs+rs+rs+rd)>>3;
						//gd = (gs+gs+gs+gs+gs+gs+gs+gd)>>3;
						//bd = (bs+bs+bs+bs+bs+bs+bs+bd)>>3;
						
						pd = (rd<<16) | (gd<<8) | (bd);
						
						*ppd=pd;
						
						
						ppd++;
						pps++;
					}
					rgbd+=bpld;
					rgbs+=bpls;
				}

				//t2 = PreciseTimer::QueryTimer();
				//printf("Drawimage time: %.3lf ms. Format of src: %d. Format of dst: %d\n",(t2-t1)*1000,surfaces[n]->format(),surface->format());
			}
		}
	}	
	
	// Clear non-drawn areas. 
	if(dirty)
	{
		// Construct a path to clip
		QRegion region(0,0,surface->width(),surface->height());
		// Iterate all the scopes
		for(unsigned i=0;i<handles.size();i++)
		{
			QRegion r(scopes[i].x,scopes[i].y,scopes[i].w,scopes[i].h);
			region=region.subtracted(r);	
		}
		painter.setClipRegion(region);
		painter.fillRect(0,0,surface->width(),surface->height(),Qt::black);
		dirty=false;		
	}
	
	painter.end();

}


void DScopesQT::DeleteScopes()
{
	for(unsigned i=0;i<scopes.size();i++)
		DeleteScope(i);
}

void DScopesQT::Clear()
{
	// Mark that we need to clear the surface. 
	// This is done during the next Render call and only clears the areas of the surface not drawn on by scopes.
	dirty=true;
}


void DScopesQT::DeleteScope(unsigned handle)
{
	if(surfaces[handle])
	{
		delete surfaces[handle];
		surfaces[handle]=0;
	}
	surfaces.erase(handle);
	Scopes::DeleteScope(handle);
}

void DScopesQT::CreateResizeScope(unsigned handle,unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool direct)
{
	scopes[handle].x=_x;
	scopes[handle].y=_y;
	scopes[handle].w=_w;
	scopes[handle].h=_h;

	if(surfaces[handle])
	{
		delete surfaces[handle];
		surfaces[handle]=0;
	}
	//printf("CreateResizeScope %d %d;%d-%d;%d direct: %d\n",handle,_x,_y,_w,_h,direct);
	if(direct==false)
	{
		surfaces[handle] = new QImage(_w,_h,QImage::Format_RGB32);
		surfaces[handle]->fill(0x00);
		
		if(scopes[handle].scope==0) 
			scopes[handle].scope = new DScopeQT(surfaces[handle],0,0,_w,_h,alpha);
		else
			static_cast<DScopeQT*>(scopes[handle].scope)->Resize(surfaces[handle],0,0,_w,_h);
	}
	else
	{
		if(scopes[handle].scope==0) 
			scopes[handle].scope = new DScopeQT(surface,_x,_y,_w,_h,alpha);
		else
			static_cast<DScopeQT*>(scopes[handle].scope)->Resize(surface,_x,_y,_w,_h);
	}
}
void DScopesQT::Resize(QImage *s,unsigned nw,unsigned nh,bool _alpha)
{
	surface = s;
	alpha=_alpha;
	for(int i=0;i<handles.size();i++)
		static_cast<DScopeQT*>(scopes[handles[i]].scope)->SetAlpha(alpha);
	Scopes::Resize(nw,nh);
}
void DScopesQT::SetAlpha(bool _alpha)
{
	alpha=_alpha;
	for(int i=0;i<handles.size();i++)
		static_cast<DScopeQT*>(scopes[handles[i]].scope)->SetAlpha(alpha);
}




