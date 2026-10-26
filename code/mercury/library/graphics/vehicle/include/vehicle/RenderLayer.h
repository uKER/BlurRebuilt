/////////////////////////////////////////////////////////////
// RenderLayer
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_RENDER_LAYER_H__
#define __VEHICLE_RENDER_LAYER_H__

#include "Vehicle/Layer.h"

namespace RenderingData { namespace Target { class Scene; } }

namespace Vehicle
{

/* 
===============================================================
					class C_VehicleModel

NOTES: Completely self contained render layer, doesn't know anything else exists, and doesn't give a flying fudge. 
===============================================================
*/
class C_RenderLayer : public C_Layer
{
public:

	C_RenderLayer();
	
	~C_RenderLayer();
	
	//! Creates layer
	bool Create();

	//! Destroys layer
	bool Destroy();

	//! Updates layer
	void Update();

	//! Return pointer to the scene data
	RenderingData::Target::Scene* GetScene() { return m_pScene; }

	//! Sets visibility of a part
	void SetPartVisible(U8 p_ePartCode, U8 p_ePositionCode, bool p_bVisible);

	//! Display using the vehicle renderer
	void Render();

	//! Return true if renderer has already been created
	bool Created() { return m_bCreated; }

private:

	RenderingData::Target::Scene*					m_pScene;							//!< Scene data

	bool											m_bCreated;
};

} //namespace Vehicle

#endif
