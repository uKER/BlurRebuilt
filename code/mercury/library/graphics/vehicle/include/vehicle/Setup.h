/////////////////////////////////////////////////////////////
// Setup
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_SETUP_H__
#define __VEHICLE_SETUP_H__

namespace WorldRender
{
	class C_LightSamplePointScene;
};

namespace Vehicle
{

struct XtVehicle;

/* 
===============================================================
					class C_VehicleModelSetup

NOTES: 
===============================================================
*/
class C_VehicleModelSetup
{
public:

	C_VehicleModelSetup();
	~C_VehicleModelSetup();

	void Create();

public:

	struct InitialState
	{
		bool	m_NightLights;

		float	m_ScratchLevel;
		float	m_DamageLevel;
	};

	Maths::HMat43				m_Matrix;

	Util::C_String				m_FilePath;							//!< Path for model
	Util::C_String				m_FileName;							//!< Name of model

	S32							m_UniqueID;
	S32							m_Index;

	XtVehicle*					m_pXtVehicle;

	bool						m_bCreateRenderLayer;				//!< Should the vehicle have a render layer
	bool						m_bCreateGameLayer;					//!< Should the vehicle have a game layer
	bool						m_bCreateDamageLayer;				//!< Should the vehicle have a damage layer

	InitialState				m_InitialState;

	WorldRender::C_LightSamplePointScene* m_pLightSamplePointScene;
};

} //namespace Vehicle

#endif

