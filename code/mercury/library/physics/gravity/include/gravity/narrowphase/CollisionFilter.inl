#ifndef COLLISION_FILTER_INL
#define COLLISION_FILTER_INL

#include "gravity/narrowphase/CollisionFilter.h"

namespace Collision
{
	FORCE_INLINE bool C_CollisionFilter::CanCollide(const C_CollisionFilter* pOther) const
	{
		//return true;
		BIZ_ASSERT(pOther);
		/* 
		 * Default group is group 0. 
		 * Behaviour - if groups are different, use the results of testPolyFlags. If groups are the same, use results of bInterGroupTest
		 */

		//const bool bTestPolyFlags( (( m_CollisionFlags & pOther->m_CollisionMask ) && ( m_CollisionMask & pOther->m_CollisionFlags )));
		//const bool bNotSameGroup(( m_GroupId != pOther->m_GroupId ) || ( m_GroupId == 0 ));
		//const bool bInterGroupTest((( m_GroupMask & pOther->m_GroupFlags ) && ( m_GroupFlags & pOther->m_GroupMask )));
		//const bool bBodyTest((m_BodyFlags & pOther->m_BodyMask) && (m_BodyMask & pOther->m_BodyFlags));
		////return (bTestPolyFlags && bNotSameGroup) ||((!bNotSameGroup) && bInterGroupTest);
		//return (m_NegateFilter || pOther->m_NegateFilter) ^ (bTestPolyFlags && bBodyTest && (bNotSameGroup || bInterGroupTest));

		const bool bTestPolyFlags1( (m_CollisionFlags & pOther->m_CollisionMask) != 0 );
		const bool bTestPolyFlags2( (m_CollisionMask & pOther->m_CollisionFlags) != 0 );
		const bool bBodyTest1( (m_BodyFlags & pOther->m_BodyMask) != 0);
		const bool bBodyTest2( (m_BodyMask & pOther->m_BodyFlags) != 0);
		const bool bInterGroupTest1( (m_GroupMask & pOther->m_GroupFlags) != 0 );
		const bool bInterGroupTest2( (m_GroupFlags & pOther->m_GroupMask) != 0 );
		const bool bTestPolyFlags(bTestPolyFlags1 & bTestPolyFlags2);
		const bool bNotSameGroup((( m_GroupId != pOther->m_GroupId ) | ( m_GroupId == 0 )));
		const bool bInterGroupTest((!bNotSameGroup)& bInterGroupTest1 & bInterGroupTest2);
		const bool bBodyTest((bBodyTest1 & bBodyTest2));
		const bool bNegate(m_NegateFilter | pOther->m_NegateFilter);
		//return (bTestPolyFlags && bNotSameGroup) ||((!bNotSameGroup) && bInterGroupTest);
		return (bNegate ^ ((bTestPolyFlags & bBodyTest) & (bNotSameGroup | bInterGroupTest)));
	}

	inline bool C_CollisionFilter::CanCollide(const U32 collisionFlags) const
	{
		//return true;
		return m_NegateFilter ^ ((m_CollisionMask & collisionFlags) != 0);
	}

}


#endif
