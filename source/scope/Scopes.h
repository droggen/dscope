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
/*
	This library defines the Abstract Base Class 'Scopes'. This class is a scope holder for multiple scopes. It featues:
		- Rendering and layouting of multpiple scopes
		- Rendering of zoomed scope, both opaque and transparent
		- Simple user interface to select a scope and sent scope control commands (e.g. zoom)
		- Dynamic addition/removal of scopes
	
	Instanciable classes derive from the Scopes ABC - see e.g. SDLScopes for SDL rendering.
	
	From the user side, all scopes are accessed through handles.
	
	Usage patterns:
	
		1. Instanciate the Scopes
		2. Add scopes with Scopes::AddScope (single scope) or Scopes::AddScopes (multiple scopes). These function return handles for accessing the scopes
		3. Plot on the scopes:
		3.1. Access the Scope object with Scopes::operator[]
		3.2. Plot on the scope by calling the Plot method of the Scope
		3.3. Call Scope::Render() to ensure correct rendering of the scopes; this handles in particular the correct rendering when a plot is magnified
		4. Send UI commands to the scopes with the Scopes::Ixxx methods; e.g. Scopes::IPoint, Scopes:ITop, etc.
		5. Possibly remove scopes with Scopes::RemoveScope
		6. Delete the Scopes object.
		
	See derived classes (e.g. SDLScopes) to see how a drawing surface and other rendering options are specified.
		
*/
/*
	Scopes are referred to by two means:
		handle: unique number that identifies the scopes. This number is tied to the map used to hold the scopeholder, i.e. get the scopeholder from the handle
		ID: number that identifies the position of the scope on screen, from 0 up to numscopes-1. Vector handles gets the handle of scope ID: h=handles[ID]
	
	The current highlighted (or selected, magnified, etc) scope ID is hscope (carefule: hscope is an ID!)
	
*/
/*
History:
	1.0	2008-02-06	First release.	
*/


#ifndef __SCOPES_H
#define __SCOPES_H


#include <vector>
#include <map>

#include "Scope.h"




class Scopes {
	
	private:
		virtual unsigned GenerateHandle();
		// Reshape: iterate through all the scopes and sets their size/position.
		virtual void Reshape();
		virtual unsigned _AddScope();
		virtual unsigned GetID(unsigned handle);
		
	protected:

		typedef struct {unsigned x,y,w,h; Scope* scope;} scopeholder;
		
        std::map<unsigned,scopeholder> scopes;	// Get the scopeholder from the handle: scopeholder = scopes[handle]
        std::vector<unsigned> handles;			// Table to map ID of scope to handle of scope: handle = handles[ID]
		
		unsigned w,h;
		unsigned nx,ny;
		unsigned hscope;		// ID of highlighted scope
		bool magnify;
		bool transparentmagnify;
		
		virtual void InitScope(unsigned n,bool magnify=false);
		virtual void DeleteScopes();
		virtual void DeleteScope(unsigned handle);
		virtual void CreateResizeScope(unsigned handle,unsigned _x,unsigned _y,unsigned _w,unsigned _h,bool direct)=0;
		virtual void Clear()=0;
		virtual void ScopeUpdateMagnify(unsigned prevpos,unsigned nowpos);

	public:
		Scopes(unsigned _w,unsigned _h);
		virtual ~Scopes();
		
		virtual void IPoint(unsigned x,unsigned y);
		virtual void ILeft();
		virtual void IRight();
		virtual void ITop();
		virtual void IBottom();
		virtual void IHZoomIn();
		virtual void IHZoomOut();
		virtual void IHZoomReset();
		virtual void IVZoomIn();
		virtual void IVZoomOut();
		virtual void IVZoomAuto();
		virtual void IMagnify();
		virtual void Render()=0;
		
		virtual unsigned AddScope();
        virtual std::vector<unsigned> AddScopes(unsigned n);
		virtual void RemoveScope(unsigned handle);
		
		virtual Scope* operator[](unsigned handle);
		virtual void Resize(unsigned nw,unsigned nh);
		
		
};




#endif


