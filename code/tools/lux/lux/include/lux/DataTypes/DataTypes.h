#ifndef __LUX_DATATYPES_DATATYPES_H__
#define __LUX_DATATYPES_DATATYPES_H__

namespace LUX
{

namespace DataTypes
{

enum DataTypes
{
	E_DataType_None,			
	E_DataType_Scalar,						// a single floating point scalar
	E_DataType_Colour,						// 3 floats (RGB)
	E_DataType_ColourAlphaScale,			// 3 floats (RGB) + alpha scaling
	E_DataType_SHVector,					// SH vector of arbitrary order
	E_DataType_SHVectorRGB,					// 3 SH vectors (RGB)
	E_DataType_SamplePoint,					// samplepoint containing RGBSH + sun, sky scalars
	E_DataType_TransferMatrix,				// light transfer matrix
	E_DataType_Basis3,						// basis3 texture format for the club
	E_DataType_ColourAndSky,				// RGB + sky scalar
	E_DataType_SHVector01,					// SH vector with all components scaled to be within the [0,1] range
	E_DataType_Basis3TheClub,				// Texture format for The Club
	E_DataType_GothamNight,					// RGB + Alpha Scaling, 2 channels for animation
	E_DataType_GothamSamplePoint,			// samplepoint with 2 channel animation info
	E_DataType_RGBDirection,				// RGB + direction + sun/sky
};

}; // namespace DataTypes

}; // namespace LUX

#endif
