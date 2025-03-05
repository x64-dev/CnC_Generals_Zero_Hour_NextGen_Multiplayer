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

// WBFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "worldbuilder.h"
#include "MainFrm.h"
#include "WBFrameWnd.h"
#include "WorldBuilderDoc.h"
#include "WHeightMapEdit.h"
#include "WbView3d.h"

/////////////////////////////////////////////////////////////////////////////
// CWBFrameWnd

IMPLEMENT_DYNCREATE(CWBFrameWnd, CFrameWnd)

CWBFrameWnd::CWBFrameWnd()
{
}

CWBFrameWnd::~CWBFrameWnd()
{
}

BOOL CWBFrameWnd::LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle,
				CWnd* pParentWnd,
				CCreateContext* pContext) {
	//dwDefaultStyle &= ~(WS_SIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU);

	BOOL ret = CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, CMainFrame::GetMainFrame(), pContext);
	if (ret) {
		Int top = ::AfxGetApp()->GetProfileInt(TWO_D_WINDOW_SECTION, "Top", 10);
		Int left =::AfxGetApp()->GetProfileInt(TWO_D_WINDOW_SECTION, "Left", 10);
		this->SetWindowPos(NULL, left,
			top, 0, 0,
			SWP_NOZORDER|SWP_NOSIZE);
		if (!m_cellSizeToolBar.Create(this, IDD_CELL_SLIDER, CBRS_LEFT, IDD_CELL_SLIDER))
		{
			DEBUG_CRASH(("Failed to create toolbar\n"));
		}
		EnableDocking(CBRS_ALIGN_ANY);
		m_cellSizeToolBar.SetupSlider();
		m_cellSizeToolBar.EnableDocking(CBRS_ALIGN_ANY);
		DockControlBar(&m_cellSizeToolBar);
	}
	return(ret);
}

void CWBFrameWnd::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(TWO_D_WINDOW_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(TWO_D_WINDOW_SECTION, "Left", frameRect.left);
	}
}


BEGIN_MESSAGE_MAP(CWBFrameWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CWBFrameWnd)
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWBFrameWnd message handlers


/////////////////////////////////////////////////////////////////////////////
// CWB3dFrameWnd

IMPLEMENT_DYNCREATE(CWB3dFrameWnd, CMainFrame)

CWB3dFrameWnd::CWB3dFrameWnd()
{
}

CWB3dFrameWnd::~CWB3dFrameWnd()
{
}


BEGIN_MESSAGE_MAP(CWB3dFrameWnd, CMainFrame)
	//{{AFX_MSG_MAP(CWB3dFrameWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_WM_MOVE()
	ON_COMMAND(ID_WINDOW_PREVIEW2954x1662, OnWindowPreview2954x1662)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PREVIEW2954x1662, OnUpdateWindowPreview2954x1662)
	ON_COMMAND(ID_WINDOW_PREVIEW1280x720, OnWindowPreview1280x720)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PREVIEW1280x720, OnUpdateWindowPreview1280x720)
	ON_COMMAND(ID_WINDOW_PREVIEW1920x1080, OnWindowPreview1920x1080)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PREVIEW1920x1080, OnUpdateWindowPreview1920x1080)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWB3dFrameWnd message handlers
BOOL CWB3dFrameWnd::LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle,
				CWnd* pParentWnd,
				CCreateContext* pContext) {
	dwDefaultStyle &= ~(WS_SIZEBOX);

	BOOL ret = CMainFrame::LoadFrame(nIDResource, dwDefaultStyle, CMainFrame::GetMainFrame(), pContext);
	return(ret);
}


void CWB3dFrameWnd::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Left", frameRect.left);
	}
}

void CWB3dFrameWnd::OnWindowPreview2954x1662()
{
	if (m_3dViewWidth == 2954) return;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", 2954);
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", 1662);
	adjustWindowSize();
}

void CWB3dFrameWnd::OnUpdateWindowPreview2954x1662(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_3dViewWidth== 2954?1:0);
}

void CWB3dFrameWnd::OnWindowPreview1280x720() 
{
	if (m_3dViewWidth == 640) return;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", 1280);
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", 720);
	adjustWindowSize();
}

void CWB3dFrameWnd::OnUpdateWindowPreview1280x720(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_3dViewWidth== 1280 ?1:0);
}

void CWB3dFrameWnd::OnWindowPreview1920x1080() 
{
	if (m_3dViewWidth == 800) return;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", 1920);
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", 1080);
	adjustWindowSize();
}

void CWB3dFrameWnd::OnUpdateWindowPreview1920x1080(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_3dViewWidth==1920?1:0);
}
