// GameRenderer.h
//

#pragma once

#define D3DXFX_LARGEADDRESS_HANDLE

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