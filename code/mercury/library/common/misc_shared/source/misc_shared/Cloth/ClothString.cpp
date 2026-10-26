#include "MiscSharedPch.h"
#include "misc_shared/Cloth/Cloth.h"

namespace Cloth {
//-------------------------------------------------------------------------------


void C_String::Init(const InitParams & initParams)
{
	/* Note - The actual particle separation will differ slightly from the value specified in initParams. This must
	happen in order to ensure that an integer number of particles fits within the requested length.
	*/


	Base::Init(initParams);

	hv startToEnd = initParams.endPos - initParams.startPos;
	hf length = startToEnd.Length();
	hv direction = startToEnd / length;


	hf particleSeparation = initParams.particleSeparation;
	if (particleSeparation <= 0.0f)
	{
		particleSeparation = 0.1f;
		LOGWARN( g_Log, "Particle separation was zero or negative when initialising a string of "
			"cloth - defaulting to 0.1f." );
	}

	m_size.x = S32( RoundToIntNearest(length * initParams.rumpleFactor / particleSeparation) ) + 1;
	if (m_size.x <= 1)
		m_size.x = 2;

	particleSeparation = length * initParams.rumpleFactor / (m_size.x - 1);
	m_stretchNaturalLength = particleSeparation;
	m_bendNaturalLength = particleSeparation * 2.0f;


	m_particleArray.Resize(m_size.x);
	m_orientationArray.Resize(m_size.x);


	CLOTH_FORSTRING
	{
		hv position = initParams.startPos +
			(direction * F32(loc.x) / initParams.rumpleFactor) * particleSeparation;

		bool isPinned;
		if (initParams.pinSeparation == 0)
		{
			isPinned = (loc.x == 0);
		}
		else
		{
			isPinned = (loc.x % initParams.pinSeparation == 0 || loc.x == m_size.x - 1);
		}

		Base::Init( Particle(loc), position, isPinned );
	}
}

	
void C_String::DoMinorUpdate( const hf & dt )
{
	CLOTH_FORSTRING
		ResetForce( Particle(loc) );


	// APPLY FORCES //

	CLOTH_FORSTRINGCROP(1)
		ApplyStretchForce( Particle(loc), Particle(loc.PlusX(1)) );

	CLOTH_FORSTRINGCROP(2)
		ApplyBendForce( Particle(loc), Particle(loc.PlusX(2)) );

	CLOTH_FORSTRING
	{
		ApplyGravity( Particle(loc) );
		ApplyWindResistance(loc);
	}

	// END APPLY FORCES //


	CLOTH_FORSTRING
	{
		LimitForce( Particle(loc) );
		Step( Particle(loc), dt );
	}

	if (m_collisionPlaneArray.Size() != 0)
	{
		CLOTH_FORSTRING
			DoCollisions( Particle(loc) );
	}
}


void C_String::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr ) const
{
	CLOTH_FORSTRINGCROP(1)
		DebugRenderLink( debugRenderPtr, Particle(loc), Particle(loc.PlusX(1)) );

	CLOTH_FORSTRING
	{
		DebugRenderTangent( debugRenderPtr, loc );
		DebugRenderOrientation( debugRenderPtr, loc );
	}
}


void C_String::DebugRenderOrientation( DbUtil::I_DebugRenderPtr debugRenderPtr, const iv & loc ) const
{
	Maths::HMat43 matrix( Orientation(loc), Particle(loc).position );
	debugRenderPtr->Draw3dMatrix( matrix, F32(g_DebugDebugRenderOrientationMatrixSize) );
}
		

//-------------------------------------------------------------------------------
};
