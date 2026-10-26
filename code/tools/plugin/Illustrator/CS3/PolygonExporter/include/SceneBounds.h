
#ifndef IPF_SCENE_BOUNDS_H
#define IPF_SCENE_BOUNDS_H

#include "Defines.h"

namespace Ipf
{

/*---------------------------------------------------------------------------------------
	Class : C_SceneBounds

	Desc : Helper class for calculating document bounds.
---------------------------------------------------------------------------------------*/
class C_SceneBounds
{
public: 
								/// Reset the bounding box.
	void						Clear();

								/// Calculate the scene bounds for all the art objects in this layer.
	void						AddLayerBounds( AIArtHandle pLayerHandle );

								/// Calculate the scene bounds for all the art objects in this group.
	void						AddGroupBounds( AIArtHandle pGroupHandle );

								/// Calculate the bounds for the given art object.
	void						AddArtBounds( AIArtHandle pArtHandle );

								/// Return the bounding box.
	const Maths::C_AxisBox&		BoundingBox()
								{
									return m_BoundingBox;
								}

	const F32					Width() const
								{
									return m_BoundingBox.Max().X() - m_BoundingBox.Min().X();
								}

	const F32					Height() const
								{
									return m_BoundingBox.Max().Y() - m_BoundingBox.Min().Y();
								}

private:
	Maths::C_AxisBox			m_BoundingBox;
};

}; // namespace Ipf

#endif // IPF_SCENE_BOUNDS_H