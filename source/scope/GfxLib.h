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

#ifndef __GFXLIB_H
#define __GFXLIB_H

#include <QImage>

#define BLACK 0x000000l
#define WHITE 0xffffffl
#define RED 0xff0000l
#define LRED 0x800000l
#define BLUE 0x0000ffl
#define LBLUE 0x000080l
#define GREY 0x808080l

extern QImage GfxImage;
extern QPaintDevice *GfxSurface;


void GfxInit();
void GfxGetSize(int *sx,int *sy);
void GfxPutPixel(int x,int y,int color);
int GfxGetPixel(int x,int y);
void GfxFillRectangle(int x1,int y1,int sx, int sy,int color);
void GfxLine(int x1,int y1,int x2,int y2,int color);
void GfxUpdate();

#endif


