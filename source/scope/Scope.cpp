/*
	Scope - Fast digital oscilloscope

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
/*
	Version: 
	1.1	2008-08-31
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scope.h"
#include "precisetimer.h"

#include <QtDebug>

//#define printf qDebug 

/******************************************************************************
******************************************************************************/

void itoa(int val, char *str)
{
	static char num[] = "0123456789";
	int sign=0;
	char aux;
	char *wstr=str;
	if(val<0)
	{
		val=-val;
		sign=1;		
	}
	
	do 
	{
		*wstr++ = num[val%10]; 
	}
	while(val/=10);
	
	if(sign) *wstr++='-';
	*wstr=0;	
	// Reverse string
	wstr--;
	while(wstr>str)
		aux=*wstr, *wstr--=*str, *str++=aux;
}


Scope::Scope(unsigned _x,unsigned _y,unsigned _w,unsigned _h)
	: title("")
{
	Resize(_x,_y,_w,_h);
	
	minxspace=32;
	minyspace=24;
	
	VAuto=true;
	vpp=1;
	vpp_m=1;
	vpp_s=1;
	vpp_m_i=0;
	
	HZoomReset();
	
	highlight=false;
}
Scope::~Scope()
{
	
}
void Scope::Resize(unsigned _x,unsigned _y,unsigned _w,unsigned _h)
{
	basex=_x;
	basey=_y;
	w=_w;
	h=_h;	
}
void Scope::HZoomPrint()
{
	printf("Zoom: spp_m: %u, spp_m_i: %u, spp_s: %u\n",spp_m,spp_m_i,spp_s);
	printf("Zoom: %lf\n",spp_s==1?((double)spp_m):(1.0/spp_m));
}
void Scope::HZoomReset()
{
	spp_m=1;
	spp_s=1;		// spp_s=1 -> spp is >=1; spp_s=0 -> spp is <1
	spp_m_i=0;
	spp_zf[0]=4;
	spp_zf[1]=5;
	spp_zf[2]=4;
}
// Zoom in: sample per pixel decreases
void Scope::HZoomin()
{
	if(spp_m==1 && spp_s==1)
		spp_s=0;
	if(spp_m_i==0) spp_m_i=2; else spp_m_i--;
	if(spp_s==0)
		spp_m=(spp_m*spp_zf[spp_m_i])>>1;
	else
		spp_m=(spp_m<<1)/spp_zf[spp_m_i];
	//printf("After zoomin:\n");
	//HZoomPrint();
}
// Zoom out: sample per pixel increases
void Scope::HZoomout()
{
	if(spp_s==0)
		spp_m=(spp_m<<1)/spp_zf[spp_m_i];
	else
		spp_m=(spp_m*spp_zf[spp_m_i])>>1;
	spp_m_i++; if(spp_m_i>2) spp_m_i=0;
	if(spp_m==1 && spp_s==0)
		spp_s=1;
	//printf("After zoomout:\n");
	//HZoomPrint();
}
// Set horizontal zoom factor
void Scope::HZoom(int z)
{
	// Positive z: zoomin. z=10: -> 1/10=0.1 sample per pixel 
	// Negative z: zoomout. z=-5: -> 5 sample per pixel 
	int factor,power,s;
	//printf("HZoom with z=%d\n",z);
	if(z==0)
		z=-1;
	if(z==1)
		z=-1;
	if(z>0)
	{
		s=FindSmallestAbove(z,factor,power);		
		spp_s = 0;
		if(factor==1) spp_m_i = 0;
		if(factor==2) spp_m_i = 2;
		if(factor==5) spp_m_i = 1;
	}
	else
	{
		s=FindSmallestAbove(-z,factor,power);
		spp_s = 1;
		if(factor==1) spp_m_i = 0;
		if(factor==2) spp_m_i = 1;
		if(factor==5) spp_m_i = 2;
	}
	spp_m = s;		
	//printf("Smallest above: %d; factor: %d; power: %d\n",s, factor,power);
	//HZoomPrint();
}

/*
	Find the smallest N,n, N e [1,2,5]*10^n | N*10^n>=target
	Returns: the smallest value and the factor
*/
int Scope::FindSmallestAbove(int target,int &factor,int &power)
{
	// Divide by 10 until equal 0, find minimum n
	int N,n,t;
	t=target;
	n=0;
	while((t/10)!=0)
	{
		t/=10;
		n++;
	}
	t=1;
	for(int i=0;i<n;i++)
		t*=10;
	// smallest N e [1,2,5] | N > target/10^n
	
	if(t>=target)
		N=1;		// only useful in case target=1
	else
		if(2*t>=target)
			N=2;
		else
			if(5*t>=target)
				N=5;
			else {N=1;n++;t*=10;}
	factor = N;
	power = n;
	return N*t;
}

/*
	Finds the smallest tick spacing which leads to at least minxspace pixel spacing
	Returns samples spacing that matches the requirements
*/
int Scope::XsTickSpacing(int &factor,int &power)
{
	// Assumes the sample per pixel is >=1, i.e. spp_s==1
	
	// There is minxspace*spp samples in the minimum space
	// need to find a number of samples for which the pixel spacing is higher than minxspace.... 
	// This number of samples must be one of [1,2,5]*10^n
	// If spp>1: 
	// 	find the smallest N,n, N e [1,2,5]*10^n | N*10^n>minxspace*spp_m
	// if spp<1:
	// 	find the smallest N,n, N e [1,2,5]*10^n | N*10^n>minxspace/spp_m
	// Note: makes no sense to draw sub-sample grid - therefore with extreme zoom no grid is shown
	int target;
	if(spp_s==1)
		target=minxspace*spp_m;
	else
	{
		target=minxspace/spp_m;
		if(minxspace%spp_m!=0)
			target++;
	}
	return FindSmallestAbove(target,factor,power);
}
/*
	Finds the smallest vertical tick spacing which leads to at least minyspace pixel spacing
	Return samples spacing that matches the requirements.
*/
int Scope::YvTickSpacing(int &factor,int &power)
{
	int target;

	if(vpp_s==1)
		target=minyspace*vpp;
	else
	{
		target=minyspace/vpp;
		if(minyspace%vpp!=0)
			target++;
	}
	return FindSmallestAbove(target,factor,power);
}
void Scope::SetVAuto()
{
	VAuto=true;
}
void Scope::vpp2vpp_m()
{
	int factor,power;
	vpp_m=FindSmallestAbove(vpp,factor,power);
	if(vpp_s==1)
	{
		if(factor==1) vpp_m_i=0;
		if(factor==2) vpp_m_i=1;
		if(factor==5) vpp_m_i=2;
	}
	else
	{
		if(factor==1) vpp_m_i=0;
		if(factor==2) vpp_m_i=2;
		if(factor==5) vpp_m_i=1;
	}
	//printf("Closest match to vpp %d: vpp_m: %d vpp_m_i: %d\n",vpp,vpp_m,vpp_m_i);
	vpp=vpp_m;
	//printf("New vpp: %d\n",vpp);
}
void Scope::SetVRange(int min,int max)
{
	VAuto=false;
	vmin=min;
	vmax=max;	

	vrange2vpp(vmin,vmax,vpp,vpp_s);
	
	//printf("SetVRange. vmin: %d vmax: %d vpp: %d vpp_s: %d\n",vmin,vmax,vpp,vpp_s);
	
}
void Scope::VZoomPrint()
{
	printf("Zoom:vpp_m: %u, vpp_m_i: %u, vpp_s: %u  vpp: %d\n",vpp_m,vpp_m_i,vpp_s,vpp);
	printf("Zoom: %lf\n",vpp_s==1?((double)vpp_m):(1.0/vpp_m));
}
void Scope::VZoomin()
{
	VAuto=false;
	vpp2vpp_m();
	vmax=vpp2vmax(vpp,vpp_s);
	if(vpp_m==1 && vpp_s==1)
		vpp_s=0;
	if(vpp_m_i==0) vpp_m_i=2; else vpp_m_i--;
	if(vpp_s==0)
		vpp_m=(vpp_m*spp_zf[vpp_m_i])>>1;
	else
		vpp_m=(vpp_m<<1)/spp_zf[vpp_m_i];
	printf("After zoomin:\n");
	vpp=vpp_m;
	vmax = vpp2vmax(vpp,vpp_s);
	VZoomPrint();
}
void Scope::VZoomout()
{
	VAuto=false;
	vpp2vpp_m();
	vmax=vpp2vmax(vpp,vpp_s);
	if(vpp_s==0)
		vpp_m=(vpp_m<<1)/spp_zf[vpp_m_i];
	else
		vpp_m=(vpp_m*spp_zf[vpp_m_i])>>1;
	vpp_m_i++; if(vpp_m_i>2) vpp_m_i=0;
	if(vpp_m==1 && vpp_s==0)
		vpp_s=1;
	printf("After zoomout:\n");
	vpp=vpp_m;
	vmax = vpp2vmax(vpp,vpp_s);
	VZoomPrint();
}

void Scope::Highlight()
{
	highlight=true;
}
void Scope::NoHighlight()
{
	highlight=false;
}




int Scope::vpp2vmax(int vpp,int vpp_s)
{
	int vmax;
	if(vpp_s==1)
	{
		vmax = h*vpp/2;
		vmin = -vmax;
	}	
	else
	{
		vmax = h/vpp/2;
		vmin = -vmax;
	}
	return vmax;
}
void Scope::vrange2vpp(int vmin,int vmax,int &vpp,int &vpp_s)
{
	// Define the vertical values per pixel (vpp)
	if(vmax-vmin>=h-1)
	{
		vpp = (vmax-vmin)/(h-1);
		vpp++;	// fast round up
		vpp_s=1;
	}
	else
	{
		vpp = (h-1)/(vmax-vmin);
		vpp_s=0;
	}
	if(vpp==1)
		vpp_s=1;
}
void Scope::Plot(vector<int> &data,unsigned color)
{	
	vector<vector<int> *> v;
	v.push_back(&data);
	vector<unsigned> vc;
	vc.push_back(color);
	Plot(v,vc);
}

void Scope::Plot(const vector<vector<int> *> &vv,const vector<unsigned> &color)
{
	cleararea();
	
	unsigned np;
	
	if(vv.size()==0)		// if no data to plot make sure we plot the scope frames and axis, but not the trace.
		np = 0;
	else		
		np = vv[0]->size();
	
	unsigned nsonscreen;	// number of samples on screen
	if(spp_s==1)
		nsonscreen=w*spp_m;
	else
		nsonscreen=w/spp_m+2;	// round up even if goes out of window
	//printf("w: %d, spp_s: %d spp_m: %d, nsonscreen: %d\n",w,spp_s, spp_m,nsonscreen);
	
	if(np<nsonscreen)
		nsonscreen=np;
		
	if(VAuto)
	{
		if(np==0)
			vmax=vmin=0;
		else
		{
			vmax=vmin=vv[0]->operator[](np-1);
			// Find the max/min and do autoscale.
			for(unsigned iv=0;iv<vv.size();iv++)
			{
				for(int s=0;s<nsonscreen;s++)
				{
					int value = vv[iv]->operator[](np-1-s);
					if(value<vmin)
						vmin=value;
					if(value>vmax)
						vmax=value;
				}
			}
		}
		if(vmax==vmin)	{ vmax++; vmin--;}	
		vrange2vpp(vmin,vmax,vpp,vpp_s);
	}
	
	
	
	fastStart();
	unsigned fcolor_axis = fastColor(ColorWhite());
	unsigned fcolor_minor = fastColor(ColorLGrey());
	unsigned fcolor_major = fastColor(ColorGrey());
	unsigned fcolor_text = fastColor(ColorGrey());
	unsigned fcolor_border;
	if(highlight)
		fcolor_border=fastColor(ColorRed());
	else
		fcolor_border = fcolor_axis;
	
	DrawHGrid(fcolor_axis,fcolor_minor,fcolor_major,fcolor_text);
	DrawVGrid(fcolor_axis,fcolor_minor,fcolor_major,fcolor_text);
	
	
	
	
	for(unsigned ni=0;ni<vv.size();ni++)
	{
		unsigned fcolor = fastColor(color[ni]);
		int lastx,lasty;
		const vector<int> &v = *vv[ni];
		for(int s=0;s<nsonscreen;s++)
		{
			int value = v[np-1-s];
							
			if(s==0)
			{
				lastx=s2x(s);
				lasty=v2y(value);
			}
			else
			{
				int newx=s2x(s);
				int newy=v2y(value);
				// Comments: Nokia N800
				// single diagonal line (48fps) is faster than 2 lineColor (33fps), and only slightly faster than vLineColor and hLineColor (43fps)
				//lineColor(lastx,lasty,newx,newy,color);
				//lineColor(lastx,lasty,lastx,newy,color);
				//lineColor(lastx,newy,newx,newy,color);
				//vLineColor(lastx,lasty+ni,newy,color);		// beware: bug in SDL_gfx 2.0.16 and earlier
				//hLineColor(lastx,newy+ni,newx,color);		// beware: bug in SDL_gfx 2.0.16 and earlier
				
				
				
				fastvLineColor(lastx,lasty,newy,fcolor);
				if(spp_s==0 )
					fasthLineColor(lastx,newy,newx,fcolor);
					
				
				if(spp_s==0 )//&& spp_m>2)
				{
					fastPixelColor(newx+1,newy+1,fcolor_axis);
					fastPixelColor(newx+1,newy-1,fcolor_axis);
					fastPixelColor(newx-1,newy+1,fcolor_axis);
					fastPixelColor(newx-1,newy-1,fcolor_axis);
				}
				lastx=newx;
				lasty=newy;
			}
		}
	}

	// Draw frame
	
	fasthLineColor(0,0,w-1,fcolor_border);
	fasthLineColor(0,h-1,w-1,fcolor_border);
	fastvLineColor(0,0,h-1,fcolor_border);
	fastvLineColor(w-1,0,h-1,fcolor_border);
		
	// Draw title
	if(title.length())
	{
		unsigned x=w/10;
		unsigned y=4;
		fastStringColor(x,y,title.c_str(),ColorWhite());
	}
	
	
	fastStop();
	
	
}

int Scope::s2x(int s)
{
	if(spp_s==0)
		// multiply
		return w-1-s*spp_m;
	else
		return w-1-s/spp_m;
	
}
int Scope::v2y(int v)
{
	if(vpp_s==1)
		//return (vmax-v)/vpp;
		return (vmax-v)/vpp + (h-(vmax-vmin)/vpp)/2;		// Center by placing [vmax;vmin] at equal distance from border
	else
		//return (vmax-v)*vpp;
		return (vmax-v)*vpp + (h-(vmax-vmin)*vpp)/2;
}

void Scope::DrawHGrid(unsigned fcolor_axis,unsigned fcolor_minor,unsigned fcolor_major,unsigned fcolor_text)
{
	// Draw
	int factor,power;
	int xs = XsTickSpacing(factor,power);
	//int xs=50;
	//factor=5; power=1;
	
	int i=0;
	while(s2x(i*xs)>=0)
	{
		// Get the color
		unsigned color = fcolor_minor;
		if(((i*factor)%10)==0)
			color=fcolor_major;
		if(i==0)
			color=fcolor_axis;
		
		//lineColor(s2x(i*xs),0,s2x(i*xs),h,color);
		fastvLineColor(s2x(i*xs),0,h-1,color);
		//fastvLineColor(s2x(i*xs),0,h,color);
		
		
		char s[256];
		//sprintf(s,"%d",-i*xs);
		itoa(-i*xs,s);
		//if(strlen(s)>5)
			//sprintf(s,"%dE%d",-factor*i,power);
		int sx = strlen(s)*8;
		int xpos = s2x(i*xs)-strlen(s)*4;
		if(xpos+sx>w) xpos=w-sx;
		if(xpos<0) xpos=0;
		int ypos=v2y(0)-8;
		if(ypos<0) ypos=0;
		if(ypos>=h) ypos=h-1-8;
		fastStringColor(xpos,ypos,s,fcolor_text);
		
		
		i++;
	}	
}

void Scope::DrawVGrid(unsigned fcolor_axis,unsigned fcolor_minor,unsigned fcolor_major,unsigned fcolor_text)
{
	// Draw
	
	int factor,power;
	int yv = YvTickSpacing(factor,power);
	//int yv=50;
	//factor=5; power=1;
	//printf("YvTickSpacing: %u, factor: %u\n",yv,factor);
	
	int i;
	// Start from bottom
	if(vpp_s==1)
		i = (vmax-h*vpp)/yv;
	else
		i = (vmax-h/vpp)/yv;
	//printf("vmax/min: %d %d. h: %d. vpp:%d vpp_m:%d vpp_s:%d i:%d yv:%d v2y(i*yv):%d\n",vmax,vmin,h,vpp,vpp_m,vpp_s,i,yv,v2y(i*yv));
	while(v2y(i*yv)>=0)
	{
		// Get the color
		unsigned color = fcolor_minor;
		if(((i*factor)%10)==0)
			color=fcolor_major;
		if(i==0)
			color=fcolor_axis;
		//lineColor(0,v2y(i*yv),w,v2y(i*yv),color);
		fasthLineColor(0,v2y(i*yv),w-1,color);
		//fasthLineColor(0,v2y(i*yv),w,color);
		
		// Draw the text here
		
		char s[256];
		//sprintf(s,"%d",i*yv);
		itoa(i*yv,s);		// slightly faster
		fastStringColor(0,v2y(i*yv)-4,s,fcolor_text);
		
		
		
		i++;
	}
	


}


void Scope::SetTitle(const string &t)
{
	title=t;
}









