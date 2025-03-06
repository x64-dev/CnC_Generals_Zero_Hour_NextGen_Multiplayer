/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

////// W3DWebBrowser.cpp ///////////////
// July 2002 Bryan Cleveland

#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "WW3D2/Texture.h"
#include "WW3D2/TextureLoader.h"
#include "WW3D2/SurfaceClass.h"
#include "GameClient/Image.h"
#include "GameClient/GameWindow.h"
#include "vector2i.h"
#include <d3dx9.h>
#include "GameRenderer.h"

W3DWebBrowser::W3DWebBrowser() : WebBrowser() {
}

Bool W3DWebBrowser::createBrowserWindow(char *tag, GameWindow *win) 
{
	return FALSE;
}

void W3DWebBrowser::closeBrowserWindow(GameWindow *win) 
{
	DX8WebBrowser::DestroyBrowser(win->winGetInstanceData()->m_decoratedNameString.str());
}
