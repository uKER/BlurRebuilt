#ifndef DEFAULT_COLLISION_FILTER_H
#define DEFAULT_COLLISION_FILTER_H

namespace Collision
{

	/*
	 * A brief description of what these variables mean...
	 * m_CollisionFlags = what I am
	 * m_CollisionMask = what I will accept to collide with
	 * m_GroupId = what group am I in
	 * m_GroupFlags = what I am in the group
	 * m_GroupMask = what I will collide with in my group
	 */

	class C_Shape;

	struct C_CollisionParams
	{
		U32 m_CollisionFlags;
		U32 m_CollisionMask;
	};

	struct C_GroupParams
	{
		U32 m_GroupId;
		U32 m_GroupFlags;
		U32 m_GroupMask;
	};

	struct C_BodyParams
	{
		U32 m_BodyFlags;
		U32 m_BodyMask;
	};

	class ALIGN16 C_CollisionFilter
	{
	public:

		friend class C_Shape;

		C_CollisionFilter(const U32 collisionFlag = 0xffffffff, const U32 collisionMask = 0xffffffff, 
			const U32 groupId = 0, const U32 groupFlags = 0x0, const U32 groupMask = 0x0,
			const U32 bodyFlags = 0xffffffff, const U32 bodyMask = 0xffffffff, bool bNegateFilter = false )
			: m_CollisionFlags(collisionFlag), m_CollisionMask(collisionMask), m_GroupId(groupId), m_GroupFlags(groupFlags),
			m_GroupMask(groupMask), m_BodyFlags(bodyFlags), m_BodyMask(bodyMask),
			m_NegateFilter(bNegateFilter)
		{
		}

		C_CollisionFilter(const C_CollisionParams& cp) : m_CollisionFlags(cp.m_CollisionFlags), m_CollisionMask(cp.m_CollisionMask), m_GroupId(0), m_GroupFlags(0x0),
			m_GroupMask(0x0), m_BodyFlags(0xffffffff), m_BodyMask(0xffffffff)
		{
		}

		C_CollisionFilter(const C_CollisionParams& cp, const C_GroupParams& gp) : m_CollisionFlags(cp.m_CollisionFlags), m_CollisionMask(cp.m_CollisionMask), 
			m_GroupId(gp.m_GroupId), m_GroupFlags(gp.m_GroupFlags),
			m_GroupMask(gp.m_GroupMask), m_BodyFlags(0xffffffff), m_BodyMask(0xffffffff)
		{
		}

		C_CollisionFilter(const C_CollisionParams& cp, const C_BodyParams& bp) : m_CollisionFlags(cp.m_CollisionFlags), m_CollisionMask(cp.m_CollisionMask), m_GroupId(0), m_GroupFlags(0x0),
			m_GroupMask(0x0), m_BodyFlags(bp.m_BodyFlags), m_BodyMask(bp.m_BodyMask)
		{
		}

		C_CollisionFilter(const C_CollisionParams& cp, const C_GroupParams& gp, const C_BodyParams& bp) : m_CollisionFlags(cp.m_CollisionFlags), 
			m_CollisionMask(cp.m_CollisionMask), m_GroupId(gp.m_GroupId), m_GroupFlags(gp.m_GroupFlags),
			m_GroupMask(gp.m_GroupMask), m_BodyFlags(bp.m_BodyFlags), m_BodyMask(bp.m_BodyMask)
		{
		}

		C_CollisionFilter(const C_CollisionParams& cp, const C_GroupParams& gp, const C_BodyParams& bp,
			bool bNegateFilter) : m_CollisionFlags(cp.m_CollisionFlags), 
			m_CollisionMask(cp.m_CollisionMask), m_GroupId(gp.m_GroupId), m_GroupFlags(gp.m_GroupFlags),
			m_GroupMask(gp.m_GroupMask), m_BodyFlags(bp.m_BodyFlags), m_BodyMask(bp.m_BodyMask),
			m_NegateFilter(bNegateFilter)
		{
		}

		~C_CollisionFilter()
		{
		}

		template <typename Serial>
		void Serialise(Serial& s)
		{
			s << m_CollisionFlags << m_CollisionMask << m_GroupId << m_GroupFlags << m_GroupMask << m_BodyFlags << m_BodyMask << m_NegateFilter;
		}

		template <typename Serial>
		void Deserialise(Serial& s)
		{
			s >> m_CollisionFlags >> m_CollisionMask >> m_GroupId >> m_GroupFlags >> m_GroupMask >> m_BodyFlags >> m_BodyMask >> m_NegateFilter;
		}

		bool CanCollide(const C_CollisionFilter* pOther) const;

		bool CanCollide(const U32 collisionFlags) const;
		
		void MergeFlags(const U32 collisionFlags)
		{
			m_CollisionFlags |= collisionFlags;
		}

		void MergeMask(const U32 collisionMask)
		{
			m_CollisionMask |= collisionMask;
		}

		void MergeFlagsAndMask(const C_CollisionFilter& filter)
		{
			m_CollisionFlags |= filter.m_CollisionFlags;
			m_CollisionMask |= filter.m_CollisionMask;
		}

		void SetCollisionFlags(const U32 collisionFlags)
		{
			m_CollisionFlags = collisionFlags;
		}

		void SetCollisionMask(const U32 collisionMask)
		{
			m_CollisionMask = collisionMask;
		}

		void SetGroupId(const U32 groupId)
		{
			m_GroupId = groupId;
		}

		void SetGroupFlags(const U32 groupFlags)
		{
			m_GroupFlags = groupFlags;
		}

		void SetGroupMask(const U32 groupMask)
		{
			m_GroupMask = groupMask;
		}

		void SetBodyFlags(const U32 bodyFlags)
		{
			m_BodyFlags = bodyFlags;
		}

		void SetBodyMask(const U32 bodyMask)
		{
			m_BodyMask = bodyMask;
		}

		void SetNegateFilter(const bool negateFilter)
		{
			m_NegateFilter = negateFilter;
		}

		U32 GetCollisionFlags()const
		{
			return m_CollisionFlags;
		}

		U32 GetGroupId()const
		{
			return m_GroupId;
		}

		U32 GetCollisionMask()const
		{
			return m_CollisionMask;
		}
		U32 GetGroupFlags()const
		{
			return m_GroupFlags;
		}

		U32 GetGroupMask()const 
		{
			return m_GroupMask;
		}

		U32 GetBodyFlags()const 
		{
			return m_BodyFlags;
		}

		U32 GetBodyMask()const
		{
			return m_BodyMask;
		}

		bool GetNegateFilter()const 
		{
			return m_NegateFilter;
		}


	private:
		U32 m_CollisionFlags;
		U32 m_CollisionMask;
		U32 m_GroupId;
		U32 m_GroupFlags;
		U32 m_GroupMask;
		U32 m_BodyFlags;
		U32 m_BodyMask;
		bool m_NegateFilter; 
	};
}

#include "gravity/narrowphase/CollisionFilter.inl"

#endif
