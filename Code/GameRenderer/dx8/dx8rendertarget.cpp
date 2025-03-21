#include "../GameRenderer/GameRenderer.h"

bool wwRenderTarget::Initialize(
	UINT                width,
	UINT                height,
	D3DFORMAT           colorFormat,
	D3DFORMAT           depthFormat,
	D3DMULTISAMPLE_TYPE msaaType,
	DWORD               msaaQuality)
{

	Release(); // Clean up if re-initializing

	m_pDevice = DX8Wrapper::D3DDevice;
	m_pDevice->AddRef();

	m_Width = width;
	m_Height = height;
	m_bUseMSAA = (msaaType != D3DMULTISAMPLE_NONE);

	HRESULT hr = S_OK;

	hr = m_pDevice->CreateRenderTarget(
		m_Width,
		m_Height,
		colorFormat,
		D3DMULTISAMPLE_NONE,
		0,
		FALSE,
		&m_pRT_Resolved,
		nullptr
	);
	if (FAILED(hr))
	{
		Release();
		return false;
	}

	if (depthFormat != D3DFMT_UNKNOWN)
	{
		hr = m_pDevice->CreateDepthStencilSurface(
			m_Width,
			m_Height,
			depthFormat,
			D3DMULTISAMPLE_NONE,
			0,
			TRUE,
			&m_pDS_Resolved,
			nullptr
		);
		if (FAILED(hr))
		{
			Release();
			return false;
		}
	}

	if (m_bUseMSAA)
	{
		hr = m_pDevice->CreateRenderTarget(
			m_Width,
			m_Height,
			colorFormat,
			msaaType,
			msaaQuality,
			FALSE,
			&m_pRT_MSAA,
			nullptr
		);
		if (FAILED(hr))
		{
			Release();
			return false;
		}

		if (depthFormat != D3DFMT_UNKNOWN)
		{
			hr = m_pDevice->CreateDepthStencilSurface(
				m_Width,
				m_Height,
				depthFormat,
				msaaType,
				msaaQuality,
				TRUE,
				&m_pDS_MSAA,
				nullptr
			);
			if (FAILED(hr))
			{
				Release();
				return false;
			}
		}
	}

	auto UnwrapSurface = [&](IDirect3DSurface9* pSurf, ID3D12Resource** ppOut) -> void
		{
			if (!pSurf || !ppOut) return;

			DX8Wrapper::device9On12->UnwrapUnderlyingResource(
				pSurf,
				DX8Wrapper::D3D12Renderer->graphics_queue->dx_queue,
				__uuidof(ID3D12Resource),
				reinterpret_cast<void**>(ppOut)
			);
		};

	// Unwrap each surface if valid
	UnwrapSurface(m_pRT_Resolved, &m_pRT_Resolved_12);
	UnwrapSurface(m_pDS_Resolved, &m_pDS_Resolved_12);
	UnwrapSurface(m_pRT_MSAA, &m_pRT_MSAA_12);
	UnwrapSurface(m_pDS_MSAA, &m_pDS_MSAA_12);

	return true;
}

void wwRenderTarget::Release()
{
	// Release D3D12 resources
	SAFE_RELEASE(m_pRT_MSAA_12);
	SAFE_RELEASE(m_pDS_MSAA_12);
	SAFE_RELEASE(m_pRT_Resolved_12);
	SAFE_RELEASE(m_pDS_Resolved_12);

	// Release D3D9 surfaces
	SAFE_RELEASE(m_pRT_MSAA);
	SAFE_RELEASE(m_pDS_MSAA);
	SAFE_RELEASE(m_pRT_Resolved);
	SAFE_RELEASE(m_pDS_Resolved);

	// Finally, release D3D9 device
	SAFE_RELEASE(m_pDevice);

	m_Width = 0;
	m_Height = 0;
	m_bUseMSAA = false;
}

bool wwRenderTarget::BeginRender()
{
	if (!m_pDevice)
		return false;

	HRESULT hr = S_OK;

	// If we have MSAA surfaces, set them. Otherwise, set single-sample surfaces.
	if (m_bUseMSAA && m_pRT_MSAA)
	{
		hr = m_pDevice->SetRenderTarget(0, m_pRT_MSAA);
		if (FAILED(hr)) return false;

		// Depth
		hr = m_pDevice->SetDepthStencilSurface(m_pDS_MSAA);
		if (FAILED(hr)) return false;
	}
	else
	{
		// Single-sample
		if (!m_pRT_Resolved)
			return false;

		hr = m_pDevice->SetRenderTarget(0, m_pRT_Resolved);
		if (FAILED(hr)) return false;

		hr = m_pDevice->SetDepthStencilSurface(m_pDS_Resolved);
		if (FAILED(hr)) return false;
	}

	return true;
}

bool wwRenderTarget::EndRender()
{
	if (!m_pDevice)
		return false;

	if (m_bUseMSAA && m_pRT_MSAA && m_pRT_Resolved)
	{
		HRESULT hr = m_pDevice->StretchRect(
			m_pRT_MSAA,
			nullptr,
			m_pRT_Resolved,
			nullptr,
			D3DTEXF_NONE
		);
		if (FAILED(hr)) return false;
	}

	if (m_pRT_Resolved)
	{
		IDirect3DSurface9* pBackBuffer = nullptr;
		HRESULT hr = m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
		if (SUCCEEDED(hr) && pBackBuffer)
		{
			hr = m_pDevice->StretchRect(
				m_pRT_Resolved,
				nullptr,
				pBackBuffer,
				nullptr,
				D3DTEXF_NONE
			);
			pBackBuffer->Release();

			if (FAILED(hr)) return false;
		}
	}

	m_pDevice->SetRenderTarget(0, nullptr);

	return true;
}
