#ifndef CROWD_CHARACTER_REF_H
#define CROWD_CHARACTER_REF_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>

#include <Core/Containers/String.h>

namespace AnimatedCrowd
{

//! C_CrowdCharacterRef

class C_CrowdCharacterRef
{
public:

	C_CrowdCharacterRef()
		: m_BehaviourIndex(0)
	{
#ifdef CFG_CONVERTER
		m_StoreSHNode = false;
#endif
	}

	~C_CrowdCharacterRef()
	{
	}

	const U16 GetBehaviourIndex() const
	{
		return m_BehaviourIndex;
	}

#ifdef CFG_CONVERTER
	void SetBehaviourIndex(const U16 behaviourIndex)
	{
		m_BehaviourIndex = behaviourIndex;
	}

	const Maths::C_AxisBox&	GetAxisBox() const
	{
		return m_AxisBox;
	}

	const Maths::C_AxisBox GetAxisBox(const Maths::HVec3& position) const
	{
		Maths::C_AxisBox axisBox;

		axisBox.Min(m_AxisBox.Min() + position);
		axisBox.Max(m_AxisBox.Max() + position);

		return axisBox;
	}

	void SetAxisBox(const Maths::C_AxisBox& axisBox)
	{
		m_AxisBox = axisBox;
	}

	void StoreSHNode(const bool storeSH)
	{
		m_StoreSHNode = storeSH;
	}

	const bool StoreSHNode() const
	{
		return m_StoreSHNode;
	}

	template< typename Archive >
	void Serialise( Archive& ar )
	{
		ar << m_BehaviourIndex;
	}
#endif
	
#ifndef CFG_CONVERTER
	template< typename Archive > 
	void Deserialise( Archive& ar )
	{
		if (g_CrowdFileVersion >= AnimatedCrowd::E_CrowdFileVersion_BehaviourHashes)
		{
			ar >> m_BehaviourIndex;
		}
		else
		{
			if (g_CrowdFileVersion >= E_CrowdFileVersion_BehaviourRefactor)
			{
				S32 behaviourType;

				ar >> behaviourType;

				m_BehaviourIndex = static_cast<U16>(behaviourType);
			}
			else
			{
				S32 memberType;

				ar >> memberType;

				m_BehaviourIndex = static_cast<U16>(MapMemberType(static_cast<CrowdMemberType>(memberType)));
			}
		}
	}
#endif

private:

	U16					m_BehaviourIndex;

#ifdef CFG_CONVERTER
	Maths::C_AxisBox	m_AxisBox;
	bool				m_StoreSHNode;
#endif
};

//! C_CrowdSHNodeData

class C_CrowdSHNodeData
{
public:

	C_CrowdSHNodeData()
		: m_InstanceIndex(-1)
	{
		m_SHData[0]  = 1.f; m_SHData[1]  = 0.f; m_SHData[2]  = 0.f; m_SHData[3]  = 0.f;
		m_SHData[4]  = 0.f; m_SHData[5]  = 1.f; m_SHData[6]  = 0.f; m_SHData[7]  = 0.f;
		m_SHData[8]  = 0.f; m_SHData[9]  = 0.f; m_SHData[10] = 1.f; m_SHData[11] = 0.f;
		m_SHData[12] = 0.f; m_SHData[13] = 1.f; m_SHData[14] = 0.f; m_SHData[15] = 1.f;
	}

	~C_CrowdSHNodeData()
	{
	}

	const U16 GetInstanceIndex()
	{
		return m_InstanceIndex;
	}

#ifdef CFG_CONVERTER
	void SetInstanceIndex(const U16 instanceIndex)
	{
		m_InstanceIndex = instanceIndex;
	}

	F32* GetFloatArrayPtr()
	{
		return m_SHData;
	}

	template< typename Archive >
	void Serialise( Archive& ar )
	{
		ar << m_InstanceIndex;

		for (U32 i = 0; i < 16; ++i)
			ar << m_SHData[i];
	}
#endif

#ifndef CFG_CONVERTER
	const F32* GetSHData() const
	{
		return m_SHData;
	}
#endif
	
#ifndef CFG_CONVERTER
	template< typename Archive > 
	void Deserialise( Archive& ar )
	{
		ar >> m_InstanceIndex;

		F32 fTemp = 0.f;

		for (U32 i = 0; i < 16; ++i)
		{
			ar >> fTemp;
			m_SHData[i] = fTemp;
		}
	}
#endif

private:

	F32				m_SHData[16];
	U16				m_InstanceIndex;
};

}; // namespace AnimatedCrowd

#endif // CROWD_CHARACTER_REF_H
