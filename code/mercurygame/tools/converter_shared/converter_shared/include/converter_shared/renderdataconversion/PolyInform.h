#ifndef __RENDERDATACONVERSION_POLYINFORM_H__
#define __RENDERDATACONVERSION_POLYINFORM_H__

namespace RenderDataConversion
{

// virtual base class that can query information about a polygon
class I_PolyInform
{
public:
	/// return false if the polygon is not supposed to be rendered
	virtual bool IsVisible(const Aftk::C_Polygon *pPoly) const = 0;
	/// Return true if the polygon should be classified and culled using the portal system.
	virtual bool IsPortalCulled(const Aftk::C_Polygon *pPoly) const = 0;
	/// Return true if the polygon should be rendered with cullmode none.
	virtual bool DoNotBackfaceCull(const Aftk::C_Polygon *pPoly) const = 0;
	/// Return custom primitive flags
	virtual int GetCustomFlags(const Aftk::C_Polygon *pPoly) const = 0;
};

}

#endif
