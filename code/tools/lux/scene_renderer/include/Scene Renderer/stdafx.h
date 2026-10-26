// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// TODO: reference additional headers your program requires here

#include <core/core.h>

const float PI = 3.1415926535897932384626433832795f;

#include <stdio.h>
#include <assert.h>

#include <string>
#include <vector>
#include <io.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>

#include <windows.h>
#include <mmsystem.h>

#include <D3DX9.h>
#include <dxerr.h>

#include <core/maths/Constants.h>
#include <core/maths/Storage.h>
#include <old_nightmare_maths/maths.h>
#include <core/maths/MathsUtil.h>
#include <core/UtMaths.h>
#include <core/containers/vector.h>
#include <core/containers/bitvector.h>
#include <Lpvs Common/LpvsCommonHeaders.h>

#include <Scene Renderer/Typedefs.h>

#include <Aftk/Aftk.h>

#include <spatial_hierarchy/spatialDividerHeaders.h>

#include "Common.h"
#include "SinCos.h"
#include <scene renderer/D3dWindow.h>
#include "Camera.h"
