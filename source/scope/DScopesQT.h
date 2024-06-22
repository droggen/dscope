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

#ifndef __DSCOPESQT_H
#define __DSCOPESQT_H

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
#include "Scopes.h"






class DScopesQT : public Scopes
{
	protected:
		QImage *surface;
        std::map<unsigned,QImage*> surfaces;
		bool alpha;

		QPainter painter;
		
		bool dirty;

		virtual void Clear();
		virtual void DeleteScope(unsigned handle);
		virtual void CreateResizeScope(unsigned handle,unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool direct);
		virtual void DeleteScopes();
		
	public:
		DScopesQT(QImage *s,unsigned _w,unsigned _h,bool _alpha=false);
		virtual ~DScopesQT();
		
		virtual void Render();
		virtual void Resize(QImage *s,unsigned nw,unsigned nh,bool _alpha=false);
		virtual void SetAlpha(bool _alpha);
};




#endif
