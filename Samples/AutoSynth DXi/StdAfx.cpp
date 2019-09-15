// stdafx.cpp : source file that includes just the standard includes
//	DShowMFC.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// Make sure they have the proper version of the DirectX SDK installed
#include <ActiveX.ver>

#if (VERSION_RES_MAJOR_VER != 8)
#error DirectX 8 SDK is required to build this project.
#endif
