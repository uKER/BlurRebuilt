#include "MiscSharedPch.h"
#include "misc_shared/Cloth/Cloth.h"

namespace Cloth {
//-------------------------------------------------------------------------------


void C_Sheet::Init(const InitParams & initParams)
{
	/* Note - The actual particle separations in the x and y directions will differ slightly from each other and
	from the particle separation value specified in initParams. This must happen in order to ensure that an integer
	number of particles fits within the requested length and width.
	*/


	Base::Init(initParams);

	m_shearSpringConst = initParams.shearSpringConst;
	m_shearDampCoeff = initParams.shearDampCoeff;

	bool pinOnSide = initParams.pinOnSide;
	hf rumpleFactorX = pinOnSide ? 1.0f : initParams.rumpleFactor;
	hf rumpleFactorY = pinOnSide ? initParams.rumpleFactor : 1.0f;


	hf desiredParticleSeparation = initParams.particleSeparation;
	if (desiredParticleSeparation <= 0.0f)
	{
		desiredParticleSeparation = 0.1f;
		LOGWARN( g_Log, "Particle separation was zero or negative when initialising a sheet of "
			"cloth - defaulting to 0.1f." );
	}

	m_size.x = S32( RoundToIntNearest(initParams.length * rumpleFactorX / desiredParticleSeparation) ) + 1;
	if (m_size.x <= 1)
		m_size.x = 2;

	m_size.y = S32( RoundToIntNearest(initParams.width * rumpleFactorY / desiredParticleSeparation) ) + 1;
	if (m_size.y <= 1)
		m_size.y = 2;

	hf particleSeparationX = initParams.length * rumpleFactorX / (m_size.x - 1);
	hf particleSeparationY = initParams.width * rumpleFactorY / (m_size.y - 1);


	m_stretchNaturalLength = particleSeparationX;
	m_bendNaturalLength = particleSeparationX * 2.0f;
	m_shearNaturalLength = (particleSeparationX * particleSeparationX + particleSeparationY * particleSeparationY).
		Sqrt();
	m_stretchNaturalLengthY = particleSeparationY;
	m_bendNaturalLengthY = particleSeparationY * 2.0f;


	m_particleArray.Resize(m_size.x * m_size.y);


	const Maths::HMat43 & worldMatrix = initParams.worldMatrix;

	CLOTH_FORSHEET
	{
		hv position = worldMatrix.AW() +
			worldMatrix.AX() * F32(loc.x) / rumpleFactorX * particleSeparationX -
			worldMatrix.AY() * F32(loc.y) / rumpleFactorY * particleSeparationY;

		bool isPinned;
		if (initParams.pinSeparation == 0)
		{
			isPinned = (loc.x == 0 && loc.y == 0);
		}
		else
		{
			if (pinOnSide)
				isPinned = ( loc.x == 0 && (loc.y % initParams.pinSeparation == 0 || loc.y == m_size.y - 1) );
			else
				isPinned = ( loc.y == 0 && (loc.x % initParams.pinSeparation == 0 || loc.x == m_size.x - 1) );
		}

		Base::Init( Particle(loc), position, isPinned );
	}
}

	
void C_Sheet::DoMinorUpdate( const hf & dt )
{
	CLOTH_FORSHEET
		ResetForce( Particle(loc) );


	// APPLY FORCES //

	CLOTH_FORSHEETCROP(1, 0)
		ApplyStretchForce( Particle(loc), Particle(loc.PlusX(1)) );

	CLOTH_FORSHEETCROP(0, 1)
		ApplyStretchForceY( Particle(loc), Particle(loc.PlusY(1)) );

	CLOTH_FORSHEETCROP(1, 1)
	{
		ApplyShearForce( Particle(loc), Particle(loc.Plus(1, 1)) );
		ApplyShearForce( Particle(loc.PlusX(1)), Particle(loc.PlusY(1)) );
	}

	CLOTH_FORSHEETCROP(2, 0)
		ApplyBendForce( Particle(loc), Particle(loc.PlusX(2)) );

	CLOTH_FORSHEETCROP(0, 2)
		ApplyBendForceY( Particle(loc), Particle(loc.PlusY(2)) );

	CLOTH_FORSHEET
	{
		ApplyGravity( Particle(loc) );
		ApplyWindResistance(loc);
	}

	// END APPLY FORCES //


	CLOTH_FORSHEET
	{
		LimitForce( Particle(loc) );
		Step( Particle(loc), dt );
	}

	if (m_collisionPlaneArray.Size() != 0)
	{
		CLOTH_FORSHEET
			DoCollisions( Particle(loc) );
	}
}


void C_Sheet::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr ) const
{
	CLOTH_FORSHEETCROP(1, 0)
		DebugRenderLink( debugRenderPtr, Particle(loc), Particle(loc.PlusX(1)) );

	CLOTH_FORSHEETCROP(0, 1)
		DebugRenderLink( debugRenderPtr, Particle(loc), Particle(loc.PlusY(1)) );

	CLOTH_FORSHEET
		DebugRenderNormal( debugRenderPtr, loc );

	CLOTH_FORSHEET
		DebugRenderTangent( debugRenderPtr, loc );
}


//-------------------------------------------------------------------------------
};
