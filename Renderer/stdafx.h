#pragma once

#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable:4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


#include "d3dx12.h"

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <queue>
#include <memory>
#include <string>
#include <exception>
#include <array>
#include <wrl.h>
#include <ppltasks.h>
#include <cassert>
#include "../Framework/Utility.h"
#include "../Framework/INoCopy.h"




