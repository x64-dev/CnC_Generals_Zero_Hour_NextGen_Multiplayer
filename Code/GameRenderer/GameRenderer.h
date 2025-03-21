// GameRenderer.h
//

#pragma once

#define D3DXFX_LARGEADDRESS_HANDLE

#include <d3d9.h>

class wwDeviceTexture
{
public:
	wwDeviceTexture(IDirect3DTexture9* pTexture)
		: m_pTexture(pTexture)
	{
		if (m_pTexture)
		{
	//		m_pTexture->AddRef(); // hold onto the texture
		}
	}

	ULONG AddRef()
	{
		return m_pTexture ? m_pTexture->AddRef() : 0;
	}

	ULONG Release()
	{
		// If we don't have a valid texture, there's nothing to do
		if (!m_pTexture)
			return 0;

		// Decrement COM's reference count
		ULONG refCount = m_pTexture->Release();

		// If COM has fully released it (refCount == 0), delete ourselves
		if (refCount == 0)
		{
			// Setting m_pTexture to nullptr here is optional safety
			m_pTexture = nullptr;

			// **Self-delete** – after this, the object is gone
			delete this;

			// Once we call `delete this;`, the current object is destroyed.
			// Typically you don't want to access any member variables or
			// methods after this line. Returning 0 here is usually fine.
			return 0;
		}

		// Otherwise, return the new ref count
		return refCount;
	}

	// Optionally expose the wrapped pointer if needed externally
	IDirect3DTexture9* GetWrappedTexture() const
	{
		return m_pTexture;
	}

	//--------------------------------------------------------------------------
	// IUnknown-like methods
	//--------------------------------------------------------------------------

	HRESULT QueryInterface(REFIID riid, void** ppvObj)
	{
		return m_pTexture ? m_pTexture->QueryInterface(riid, ppvObj)
			: E_POINTER;
	}

	//--------------------------------------------------------------------------
	// IDirect3DBaseTexture9-like methods
	//--------------------------------------------------------------------------

	HRESULT GetDevice(IDirect3DDevice9** ppDevice)
	{
		return m_pTexture ? m_pTexture->GetDevice(ppDevice)
			: E_POINTER;
	}

	HRESULT SetPrivateData(REFGUID refguid, const void* pData,
		DWORD SizeOfData, DWORD Flags)
	{
		return m_pTexture ? m_pTexture->SetPrivateData(refguid, pData, SizeOfData, Flags)
			: E_POINTER;
	}

	HRESULT GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
	{
		return m_pTexture ? m_pTexture->GetPrivateData(refguid, pData, pSizeOfData)
			: E_POINTER;
	}

	HRESULT FreePrivateData(REFGUID refguid)
	{
		return m_pTexture ? m_pTexture->FreePrivateData(refguid)
			: E_POINTER;
	}

	DWORD SetPriority(DWORD PriorityNew)
	{
		return m_pTexture ? m_pTexture->SetPriority(PriorityNew)
			: 0;
	}

	DWORD GetPriority()
	{
		return m_pTexture ? m_pTexture->GetPriority()
			: 0;
	}

	void PreLoad()
	{
		if (m_pTexture)
			m_pTexture->PreLoad();
	}

	D3DRESOURCETYPE GetType()
	{
		return m_pTexture ? m_pTexture->GetType()
			: D3DRTYPE_SURFACE; // or some default
	}

	DWORD SetLOD(DWORD LODNew)
	{
		return m_pTexture ? m_pTexture->SetLOD(LODNew)
			: 0;
	}

	DWORD GetLOD()
	{
		return m_pTexture ? m_pTexture->GetLOD()
			: 0;
	}

	DWORD GetLevelCount()
	{
		return m_pTexture ? m_pTexture->GetLevelCount()
			: 0;
	}

	HRESULT SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
	{
		return m_pTexture ? m_pTexture->SetAutoGenFilterType(FilterType)
			: E_POINTER;
	}

	D3DTEXTUREFILTERTYPE GetAutoGenFilterType()
	{
		return m_pTexture ? m_pTexture->GetAutoGenFilterType()
			: D3DTEXF_NONE; // or some default
	}

	void GenerateMipSubLevels()
	{
		if (m_pTexture)
			m_pTexture->GenerateMipSubLevels();
	}

	//--------------------------------------------------------------------------
	// IDirect3DTexture9-like methods
	//--------------------------------------------------------------------------

	HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc)
	{
		return m_pTexture ? m_pTexture->GetLevelDesc(Level, pDesc)
			: E_POINTER;
	}

	HRESULT GetSurfaceLevel(UINT Level, IDirect3DSurface9** ppSurfaceLevel)
	{
		return m_pTexture ? m_pTexture->GetSurfaceLevel(Level, ppSurfaceLevel)
			: E_POINTER;
	}

	HRESULT LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect,
		const RECT* pRect, DWORD Flags)
	{
		return m_pTexture ? m_pTexture->LockRect(Level, pLockedRect, pRect, Flags)
			: E_POINTER;
	}

	HRESULT UnlockRect(UINT Level)
	{
		return m_pTexture ? m_pTexture->UnlockRect(Level)
			: E_POINTER;
	}

	HRESULT AddDirtyRect(const RECT* pDirtyRect)
	{
		return m_pTexture ? m_pTexture->AddDirtyRect(pDirtyRect)
			: E_POINTER;
	}

private:
	// Destructor
	~wwDeviceTexture()
	{

	}

	// Underlying real texture pointer
	IDirect3DTexture9* m_pTexture;
};

#include "dx8/dx8wrapper.h"
#include "dx8/dx8list.h"
#include "dx8/dx8renderer.h"
#include "dx8/dx8fvf.h"
#include "dx8/dx8vertexbuffer.h"
#include "dx8/dx8indexbuffer.h"
#ifndef EDITOR
#include "dx8/dx8caps.h"
#endif
#include "dx8/dx8polygonrenderer.h"
#include "dx8/dx8renderer.h"
#include "dx8/dx8texman.h"
#ifndef EDITOR
#include "dx8/dx8WebBrowser.h"
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx9.h"

extern ImFont* g_BigConsoleFont;