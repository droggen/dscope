/*
   Scopes - Multiple Scope Holder and UI

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

#include <assert.h>
#include <math.h>
#include <algorithm>
#include "Scopes.h"



/******************************************************************************
*******************************************************************************
Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes
*******************************************************************************
*******************************************************************************
PRIVATE   PRIVATE   PRIVATE   PRIVATE   PRIVATE   PRIVATE   PRIVATE   PRIVATE
******************************************************************************/
// Reshape: iterate through all the scopes and sets their size/position.
void Scopes::Reshape()
{
	unsigned scopew,scopeh;
	
	Clear();
	
	if(handles.size()==0)
		return;
	nx=ny=static_cast<unsigned>(ceil(sqrt((double)handles.size())));

	
	scopew=w/nx;
	scopeh=h/ny;
	
	for(unsigned i=0;i<handles.size();i++)
	{
		InitScope(i,hscope==i && magnify);
		scopes[handles[i]].scope->NoHighlight();
	}
	if(!magnify)
		scopes[handles[hscope]].scope->Highlight();
}
unsigned Scopes::GenerateHandle()
{
	unsigned handle=0;
	while(find(handles.begin(),handles.end(),handle)!=handles.end())
		handle++;
	return handle;
}
unsigned Scopes::_AddScope()
{
	unsigned h = GenerateHandle();
	handles.push_back(h);
	return h;
}
unsigned Scopes::GetID(unsigned handle)
{
	vector<unsigned>::iterator it = find(handles.begin(),handles.end(),handle);
	return it-handles.begin();
}


/******************************************************************************
*******************************************************************************
Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes
*******************************************************************************
*******************************************************************************
PROTECTED   PROTECTED   PROTECTED   PROTECTED   PROTECTED   PROTECTED
******************************************************************************/


void Scopes::DeleteScopes()
{
	while(!scopes.empty())
		DeleteScope(scopes.begin()->first);
}
void Scopes::DeleteScope(unsigned handle)
{
	if(scopes[handle].scope)
	{
		delete scopes[handle].scope;
		scopes[handle].scope=0;
	}
	scopes.erase(handle);
	vector<unsigned>::iterator it;
	for(it = handles.begin();it!=handles.end();it++)
	{
		if(*it==handle)
		{
			handles.erase(it);
			break;
		}
	}
}
void Scopes::InitScope(unsigned n,bool magnify)
{
	unsigned scopew,scopeh;
	scopew=w/nx;
	scopeh=h/ny;
	
	if(magnify==false)
	{
		unsigned y = n/nx;
		unsigned x = n%nx;
			
		CreateResizeScope(handles[n],x*scopew,y*scopeh,scopew,scopeh,true);
	}
	else
	{
		CreateResizeScope(handles[n],w*1/10,h*1/10,w*8/10,h*8/10,false);
	}
}
/*
	ScopeUpdateMagnify takes IDs as input: ID of last highlighted scope, ID of newly highlighted scope (possibly identical to last)
*/
void Scopes::ScopeUpdateMagnify(unsigned prevpos,unsigned nowpos)
{
	if(prevpos==nowpos)
	{
		InitScope(prevpos,magnify);
		if(magnify)
		{
			scopes[handles[nowpos]].scope->NoHighlight();
			Clear();
		}
		else
		{
			scopes[handles[nowpos]].scope->Highlight();
			Clear();
		}
	}
	else
	{
		if(magnify)
		{
			InitScope(prevpos,false);
			InitScope(nowpos,magnify);
			Clear();
		}
		else
		{
			scopes[handles[prevpos]].scope->NoHighlight();
			scopes[handles[nowpos]].scope->Highlight();
			Clear();
		}
	}
}


/******************************************************************************
*******************************************************************************
Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes   Scopes
*******************************************************************************
*******************************************************************************
PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC
******************************************************************************/


Scopes::Scopes(unsigned _w,unsigned _h)
	: w(_w)
	, h(_h)
	, nx(0)
	, ny(0)
	, hscope(0)
	, magnify(false)
	, transparentmagnify(false)
{
}

Scopes::~Scopes()
{
	DeleteScopes();	
}


unsigned Scopes::AddScope()
{
	unsigned h = _AddScope();
	Reshape();
	return h;
}

vector<unsigned> Scopes::AddScopes(unsigned n)
{
	vector<unsigned> hs;
	
	for(unsigned i=0;i<n;i++)
		hs.push_back(_AddScope());
	Reshape();
	return hs;
}
void Scopes::RemoveScope(unsigned handle)
{
	if(GetID(handle)<hscope)
		hscope--;
	else
	{
		if(handles[hscope]==handle)
		{
			if(hscope>0)
				hscope--;
			magnify=false;
			transparentmagnify=false;
		}
	}
	DeleteScope(handle);
	Reshape();
}
	
void Scopes::IPoint(unsigned x,unsigned y)
{
	if(handles.size()==0) return;
	unsigned scopew,scopeh;
	unsigned px,py;
	
	scopew=w/nx;
	scopeh=h/ny;
	
	unsigned pre=hscope;
	
	px = x/scopew;
	py = y/scopeh;
	hscope = py*nx+px;
	if(hscope>handles.size()-1)
		hscope=handles.size()-1;
			
	ScopeUpdateMagnify(pre,hscope);
}

void Scopes::ILeft()
{
	if(handles.size()==0) return;
	if(hscope>0)
	{
		hscope--;
		ScopeUpdateMagnify(hscope+1,hscope);
	}
}

void Scopes::IRight()
{
	if(handles.size()==0) return;
	if(hscope<handles.size()-1)
	{
		hscope++;
		ScopeUpdateMagnify(hscope-1,hscope);
	}
}

void Scopes::ITop()
{
	if(handles.size()==0) return;
	unsigned pre=hscope;
	if(hscope>nx)
		hscope-=nx;
	else
		hscope=0;
	ScopeUpdateMagnify(pre,hscope);
}

void Scopes::IBottom()
{
	if(handles.size()==0) return;
	unsigned pre=hscope;
	if(hscope<handles.size()-nx)
		hscope+=nx;
	else
		hscope=handles.size()-1;
	ScopeUpdateMagnify(pre,hscope);
}

void Scopes::IHZoomIn()
{
	if(handles.size()==0) return;
	scopes[handles[hscope]].scope->HZoomin();
}
void Scopes::IHZoomOut()
{
	if(handles.size()==0) return;
	scopes[handles[hscope]].scope->HZoomout();
}

void Scopes::IHZoomReset()
{
	if(handles.size()==0) return;
	scopes[handles[hscope]].scope->HZoomReset();
}

void Scopes::IVZoomIn()
{
	if(handles.size()==0) return;
	scopes[handles[hscope]].scope->VZoomin();
}

void Scopes::IVZoomOut()
{
	if(handles.size()==0) return;
	scopes[handles[hscope]].scope->VZoomout();
}

void Scopes::IVZoomAuto()
{
	if(handles.size()==0) return;
	scopes[handles[hscope]].scope->SetVAuto();
}
void Scopes::IMagnify()
{
	if(handles.size()==0) return;
	if(magnify==false)
	{
		magnify=true;
		transparentmagnify=false;
	}
	else
	{
		if(transparentmagnify==false)
			transparentmagnify=true;
		else
		{
			magnify=false;
			transparentmagnify=false;
		}
	}
	ScopeUpdateMagnify(hscope,hscope);
}

/*
	operator[] takes a handle and returns a pointer to the scope
*/
Scope* Scopes::operator[](unsigned handle)
{
	assert(scopes.find(handle)!=scopes.end());
	return scopes[handle].scope;
}

/*
	// Resize the stuff
*/
void Scopes::Resize(unsigned nw,unsigned nh)
{
	w = nw;
	h = nh;
	Reshape(); 
}




