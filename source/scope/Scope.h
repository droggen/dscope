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
	This library defines the 'Scope' Abstract Base Class. This class implements a fast digital oscilloscope. Features are:
		- Multiple plots per scopes (with a corresponding color)
		- Title
		- Axis lines: minor, major and zero
		- Axis tick labels 
		- User-defined vertical range, or automatic fit (value per pixels)
		- User-defined horizontal range (samples per pixels)
		
	Instanciable classes derive from Scope - see e.g. SDLScope for SDL rendering.
	
	Limitations:
		- Only vector<int> can be passed
		- Overflow in scaling maths with large values (ok for up to 24-bit input values) - does not lead to crash but to graphic wraparound
	
*/	
/*
	Principles: 
		- Data storage - rendering separation: data is not stored within this class. This implies that plots must be redrawn when the screen is cleared
		- No floating point operations. 
		-	Scaling: vpp and spp mean "value per pixel" and "sample per pixel". 
			xxx_m is the magnitude. xxx_s indicates whether the number is a fraction. E.g. xxx_m=5 xxx_s=1 -> xxx=5. xxx_m=5 xxx_s=0 -> xxx=1/5
			Mathematical operations check _s and perform either a multiplication or division by xxx_m to obtain the desired pixel results.
			
		-	Rounding: no fancy rounding, other than rounding to lowest integer (intrinsic behavior of idiv).
		
		
		Scope is an abstract base class - it has no rendering capabilities.
		Derive this class and implement the following pure virtual functions to provide rendering:
			void cleararea();
			void fastPixelColor(int x,int y,unsigned color);
			void lineColor(int x1,int y1,int x2,int y2,unsigned color=0xffffff);
			void hLineColor(int x1,int y1,int x2,unsigned color=0xffffff);
			void fasthLineColor(int x1, int y, int x2, unsigned color);
			void vLineColor(int x1,int y1,int y2,unsigned color=0xffffff);
			void fastvLineColor(int x, int y1, int y2, unsigned color);
			void fastStringColor(int x,int y,char *s,unsigned color=0xffffff);
			void fastStart();
			void fastStop();
			unsigned fastColor(unsigned color);
			
		The functions prefixed by fast are called withing a fastStart()/fastStop() block. This block can lock the drawing surface
		and provide faster rendering. In case no such function is available, fastStart()/fastStop can do nothing, and the fast functions
		can call the normal non-fast methods.
*/
/*
History:
	1.0	2008-02-06	First release.	
	1.1	2008-08-31	Virtual function for color
							Bugfix in hgrid/vgrid: one pixel too wide/tall lines.
*/
/*
	BUG:
	*)
	DrawVGrid draws horizontal lines way out of the target display area when calling VZoomout
	Occurs when: 1 vgrid unit corresponds to units: 2, 5,10,20 . 
	Critical - lots of line go out - possibly some overflow number. 
	
	Solution:
		Bug in DrawVGrid
			i = vmax-h/vpp;	##CORRECT###i = (vmax-h/vpp)/yv;#### 
	
	
	2) The data plot line goes outside above the display area, for zoom: 1 line=1unit.
	3) Text on the vertical axis goes outside above the display area: only 1 unit - solved with clipping.
	
	
	BUG: when zooming too much (only one sample in window), the horizontal plot lines appears interrupted.
	Changed the rounding up to plot minimum 2 samples. 	nsonscreen=w/spp_m+2;	// round up even if goes out of window
	
*/

#ifndef __SCOPE_H
#define __SCOPE_H

#include <vector>
#include <string>





void itoa(int val, char *str);

class Scope
{
	
	private:
		bool VAuto;
		// H zoom factors
		unsigned spp_zf[3],spp_m_i,spp_s,spp_m;
		// H tick spacing
		unsigned minxspace,minyspace;
		// V resolution
		int vpp,vpp_s;
		int vpp_m,vpp_m_i;
		int vmin,vmax;
		bool highlight;
        std::string title;
		
		virtual unsigned ColorBlack()=0;
		virtual unsigned ColorWhite()=0;
		virtual unsigned ColorGrey()=0;
		virtual unsigned ColorLGrey()=0;
		virtual unsigned ColorRed()=0;
	protected:
		int basex,basey;
		int w,h;
		
		virtual void cleararea() = 0;
		virtual void fastPixelColor(int x,int y,unsigned color)=0;
		virtual void lineColor(int x1,int y1,int x2,int y2,unsigned color=0xffffff)=0;
		virtual void hLineColor(int x1,int y1,int x2,unsigned color=0xffffff)=0;
		virtual void fasthLineColor(int x1, int y, int x2, unsigned color)=0;
		virtual void vLineColor(int x1,int y1,int y2,unsigned color=0xffffff)=0;
		virtual void fastvLineColor(int x, int y1, int y2, unsigned color)=0;
		virtual void fastStringColor(int x,int y,const char *s,unsigned color=0xffffff)=0;
		virtual void fastStart() {};
		virtual unsigned fastColor(unsigned color) {return color;};
		virtual void fastStop()=0;
		
		
	public:
	
		/*struct Plots 
		{
			Plots() {data=0;color=0;};
			const vector<int> &data;
			unsigned color;
		};*/
	
		Scope(unsigned _x,unsigned _y,unsigned _w,unsigned _h);
		virtual ~Scope();
		
		virtual void Resize(unsigned _x,unsigned _y,unsigned _w,unsigned _h);
		
		virtual void DrawHGrid(unsigned fcolor_axis,unsigned fcolor_minor,unsigned fcolor_major,unsigned fcolor_text);
		void DrawVGrid(unsigned fcolor_axis,unsigned fcolor_minor,unsigned fcolor_major,unsigned fcolor_text);
		
		virtual int FindSmallestAbove(int target,int &factor,int &power);
		virtual int XsTickSpacing(int &multiplier,int &power);
		virtual int YvTickSpacing(int &multiplier,int &power);
		
		virtual int s2x(int s);
		virtual int v2y(int v);
		
		virtual void vpp2vpp_m();
		virtual void vrange2vpp(int vmin,int vmax,int &vpp,int &vpp_s);
		virtual int vpp2vmax(int vpp,int vpp_s);
		virtual void HZoom(int z);
		virtual void HZoomReset();
		virtual void HZoomin();
		virtual void HZoomout();
		virtual void HZoomPrint();
		virtual void VZoomPrint();
		virtual void SetVAuto();
		virtual void SetVRange(int min,int max);
		virtual void VZoomin();
		virtual void VZoomout();
        virtual void Plot(std::vector<int> &v,unsigned color=0xffffff);
        virtual void Plot(const std::vector<std::vector<int> *> &v,const std::vector<unsigned> &color);
		
		virtual void Highlight();
		virtual void NoHighlight();
		
        virtual void SetTitle(const std::string &t);
		
		
};



#endif


