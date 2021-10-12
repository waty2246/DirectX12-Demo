#pragma once

// Standard headers
#include <stdio.h>
#include <memory>
#include <vector>
#include <functional>
#include <sstream>
#include <fstream>
#include <string>
#include <math.h>

// System headers
#include <windows.h>
#include <mmsystem.h>
#include <windowsx.h>

// Direct3D 12 headers
#include <d3d12.h>
#include <d3dcompiler.h>

//DXGI headers
#include <dxgi1_6.h>

//Direct3D 12 helper headers
#include <d3dx12.h>
#include <wrl/client.h>
#include <pix3.h>
#include <DirectXMath.h>
#include <DDSTextureLoader.h>

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

#define ThrowIfFailed(expr) if(FAILED(expr)) throw exception(#expr)
#define ThrowIfTrue(expr) if(expr) throw exception(#expr)

#define GetShaderFilePath(shaderFile) TEXT("../shaders/" ## shaderFile)
#define GetDataFilePathA(dataFile) ("../assets/" ## dataFile)
#define GetDataFilePath(dataFile) TEXT("../assets/" ## dataFile)
#define GetTextureFilePath(textureFile) TEXT("../assets/textures/" ## textureFile)
#define GetTextureFilePathA(textureFile) ("../assets/textures/" ## textureFile)

#if !defined(NDEBUG)
#define LOG_DEBUG(msg) OutputDebugString(TEXT(msg ## "\n"));
#endif
