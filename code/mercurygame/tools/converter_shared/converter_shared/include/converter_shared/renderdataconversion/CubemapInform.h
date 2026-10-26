#ifndef __RENDERDATACONVERSION_CUBEMAPINFORM_H__
#define __RENDERDATACONVERSION_CUBEMAPINFORM_H__

namespace RenderDataConversion
{

// virtual base class that can be used to query what cubemap texture filename is associated with a particular polygon
class I_CubemapInform
{
public:
	// returns the filename for the cubemap for the specified bounding box
	virtual Util::C_String GetCubemapFilename(const Maths::C_AxisBox &bbox) const = 0;
};

}

#endif
