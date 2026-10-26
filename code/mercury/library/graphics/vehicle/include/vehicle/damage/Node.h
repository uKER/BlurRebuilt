#ifndef VEHICLEDAMAGE_NODE_H
#define VEHICLEDAMAGE_NODE_H
// Refactored from DamageLayer header.


namespace Vehicle
{

//! A single damage node.
// NOTE: Please be careful modifying this, you will need to change the SPUDamageJob to match.
class C_Node
{
public:
	C_Node()
	{
		Reset(false);
	}

	//! Reset the node to defaults
	void Reset(bool p_QuickReset)
	{
		if (p_QuickReset)
		{
			m_Position				= m_OriginalPosition;
		}
		else
		{
			m_OriginalPosition		= Maths::HVec3(0.0f, 0.0f, 0.0f);
			m_Position				= Maths::HVec3(0.0f, 0.0f, 0.0f);
			m_MaxDisplacement		= Maths::HVec3(0.0f, 0.0f, 0.0f);
			m_TargetPosition		= Maths::HVec3(0.0f, 0.0f, 0.0f);
			m_CachedPosition		= Maths::HVec3(0.0f, 0.0f, 0.0f);
			m_OffsetVector			= Maths::HVec3(0.0f, 0.0f, 0.0f);
			m_OriginalScratchLevel	= 0.0f;
			m_OriginalDeformLevel	= 0.0f;
			m_OffsetLength			= 0.0f;

			m_DeformRange.m_MinX	= 0.0f;
			m_DeformRange.m_MaxX	= 1.0f;
			m_DeformRange.m_MinY	= 1.0f;
			m_DeformRange.m_MaxY	= 0.0f;
			m_DeformRange.m_MinZ	= 0.0f;
			m_DeformRange.m_MaxZ	= 0.0f;
		}

		m_DebugColour			= Maths::RGBA(0, 0, 0, 255);

		m_DeformLevel			= m_OriginalDeformLevel;
		m_ScratchLevel			= m_OriginalScratchLevel;
		m_TargetDeformLevel		= 0.0f;
		m_CachedDeformLevel		= 0.0f;

		m_Deformable			= true;	
		m_Active				= false;
	}

public:
	// Jiggled this about a bit for 16-byte aligned loads.
	struct DeformRange
	{
		float m_MinX;
		float m_MinY;
		float m_MinZ;
		float _pad0;
		float m_MaxX;
		float m_MaxY;
		float m_MaxZ;
		float _pad1;
	};
	DeformRange m_DeformRange;
	DeformRange m_DriveableDeformRange;

	Maths::HVec3	m_OriginalPosition;							//!< Original, unmodified position of the node in local space
	Maths::HVec3	m_Position;									//!< Current node position in local space
	Maths::HVec3	m_MaxDisplacement;							//!< Maximum amount the node is allowed to move
	Maths::HVec3	m_TargetPosition;
	Maths::HVec3	m_CachedPosition;
	Maths::HVec3	m_OffsetVector;

	Maths::RGBA		m_DebugColour;

	float m_OriginalDeformLevel;
	float m_OriginalScratchLevel;
	float m_DeformLevel;										//!< The amount of the damaged morph target to reveal
	float m_ScratchLevel;										//!< The scratch level at this node
	float m_TargetDeformLevel;
	float m_CachedDeformLevel;
	float m_TargetScratchLevel;
	float m_CachedScratchLevel;
	float m_OffsetLength; 

	bool			m_Deformable;								//!< Can this node be deformed
	bool			m_Active;									//!< Debug, is this node being manipulated	
};

}

#endif // #ifndef VEHICLEDAMAGE_NODE_H
