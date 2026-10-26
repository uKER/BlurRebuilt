#ifdef _MSC_VER_
#pragma once
#endif

#ifndef ATTK_HEADERS_H
#define ATTK_HEADERS_H

//External includes
#include <core/core.h>
#include <core/maths/utmaths/maths.h>
#include <core/containers/vector.h>

//#include <assert.h>

#ifndef ASSERT
	#ifdef _DEBUG
	#define ASSERT(x) if (!(x)) _asm int 3
	#else
	#define ASSERT(x) assert(x)
	#endif
#endif

//STL
#include <string>

//Attk includes
#include "Enum.h"
#include "Global.h"
#include "Status.h"
#include "OptionAttribute.h"
#include "Options.h"
#include "BaseImage.h"
#include "PixelFormats.h"
#include "Image.h"
#include "CubeMap.h"
#include "MipmapImageChain.h"
#include "MipmapPixelDataChain.h"
#include "Utility.h"

//Attkut
#include "./attkut/GenerateNormalMap.h"
#include "./attkut/GenerateMipmaps.h"
#include "./attkut/ImageFilter.h"
#include "./attkut/ImageAdjustmentCurve.h"
#include "./attkut/ImageAdjustment.h"
#include "./attkut/ImageResize.h"
#include "./attkut/ImageRotate.h"
#include "./attkut/FloodFill.h"
#endif
