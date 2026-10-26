#ifndef __COLLISIONCACHE_INL__
#define __COLLISIONCACHE_INL__

namespace RacingPhysics
{

template <typename T, typename BoundingBox, bool FIRST_ONLY>
FORCE_INLINE void CollisionCache::CollideTriangleEdges(const T& collisionObject, const BoundingBox& box, CollisionCache::Contact& contact) const
{
	RP_ASSERT(m_state == STATE_READY);

	int nte = GetNumTriangleEdges();

	if (nte > 0)
	{
		const CollisionCache::TriangleEdge* __restrict pTriangleEdge = &GetTriangleEdge(0);

		for (int i = 0; i < nte; i++, pTriangleEdge++)
		{
			DCBT(0x080, pTriangleEdge);

			if (box.TestBounds(pTriangleEdge->vertex0, pTriangleEdge->vertex1))
			{
				if (collisionObject.CollideFeature(*pTriangleEdge, contact) && FIRST_ONLY)
					return;
			}
		}
	}
}

template <typename T, typename BoundingBox, bool FIRST_ONLY>
FORCE_INLINE void CollisionCache::CollideTriangleVertices(const T& collisionObject, const BoundingBox& box, CollisionCache::Contact& contact) const
{
	int ntv = GetNumTriangleVertices();

	if (ntv > 0)
	{
		const CollisionCache::TriangleVertex* __restrict pTriangleVertex = &GetTriangleVertex(0);

		for (int i = 0; i < ntv; i++, pTriangleVertex++)
		{
			DCBT(0x080, pTriangleVertex);

			if (collisionObject.CollideFeature(*pTriangleVertex, contact) && FIRST_ONLY)
				return;
		}
	}
}

template <typename T, typename BoundingBox, bool FIRST_ONLY>
FORCE_INLINE void CollisionCache::CollideTriangleSurfaces(const T& collisionObject, const BoundingBox& box, CollisionCache::Contact& contact) const
{
	int nt = GetNumTriangles();

	if (nt > 0)
	{
		const CollisionCache::Triangle* __restrict pTriangle = &GetTriangle(0);

		for (int i = 0; i < nt; i++, pTriangle++)
		{
			DCBT(0x080, pTriangle);

			if (box.TestBounds(pTriangle->vertex0, 
							pTriangle->vertex1, 
							pTriangle->vertex2))
			{
				contact.boundingBoxesHit++;

				if (collisionObject.CollideFeature(*pTriangle, contact) && FIRST_ONLY)
					return;
			}
		}
	}
}

template <typename T, typename BoundingBox, bool FIRST_ONLY>
FORCE_INLINE void CollisionCache::CollideVerticalSurfaces(const T& collisionObject, const BoundingBox& box, CollisionCache::Contact& contact) const
{
	int nvs = GetNumVerticalSurfaces();

	if (nvs > 0)
	{
		const CollisionCache::VerticalSurface* __restrict pVerticalSurface = &GetVerticalSurface(0);

		for (int i = 0; i < nvs; i++, pVerticalSurface++)
		{
			DCBT(0x080, pVerticalSurface);

			if (box.TestBoundsXZ(pVerticalSurface->vertex0, pVerticalSurface->vertex1))
			{
				contact.boundingBoxesHit++;

				if (collisionObject.CollideFeature(*pVerticalSurface, contact) && FIRST_ONLY)
					return;
			}
		}
	}
}

template <typename T, typename BoundingBox, bool FIRST_ONLY>
FORCE_INLINE CollisionCache::Contact CollisionCache::CollideVolume(const T& collisionObject, const BoundingBox& box, U32 collisionFlags) const
{
	RP_ASSERT(m_state == STATE_READY);
	RP_ASSERT(m_type == CACHE_TYPE_FULL);

	Contact contact;
	contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
	contact.intersectionDistance = Maths::Constants<ScalarType>::Zero();
	contact.boundingBoxesHit = 0;

	RP_ASSERT(BoundingBoxContains(box.GetMin(), box.GetMax(), 1.5f) != 0);

	if (!(collisionFlags & COLLISION_FLAG_IGNORE_TRIANGLES))
	{
		CollideTriangleSurfaces<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

		if (FIRST_ONLY && contact.intersectionDistance > Maths::Constants<ScalarType>::Zero())
			return contact;

		// If didn't hit any triangle bounding boxes, it won't hit edges or vertices either.
		if (contact.boundingBoxesHit > 0)
		{
			CollideTriangleEdges<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

			if (FIRST_ONLY && contact.intersectionDistance > Maths::Constants<ScalarType>::Zero())
				return contact;

			CollideTriangleVertices<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

			if (FIRST_ONLY && contact.intersectionDistance > Maths::Constants<ScalarType>::Zero())
				return contact;
		}
	}

	if (!(collisionFlags & COLLISION_FLAG_IGNORE_VERTICAL_SURFACES))
	{
		CollideVerticalSurfaces<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

		if (FIRST_ONLY && contact.intersectionDistance > Maths::Constants<ScalarType>::Zero())
			return contact;
	}

	return contact;
}

// Ray collision only hits surfaces.
template <typename T, typename BoundingBox, bool FIRST_ONLY>
FORCE_INLINE CollisionCache::Contact CollisionCache::CollideRay(const T& collisionObject, const BoundingBox& box, const ScalarType& maxDistance, U32 collisionFlags) const
{
	RP_ASSERT(m_state == STATE_READY);

	Contact contact;
	contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
	contact.intersectionDistance = maxDistance;
	contact.boundingBoxesHit = 0;

	RP_ASSERT(BoundingBoxContains(box.GetMin(), box.GetMax(), 1.5f) != 0);

	if (!(collisionFlags & COLLISION_FLAG_IGNORE_TRIANGLES))
	{
		CollideTriangleSurfaces<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

		if (FIRST_ONLY && contact.intersectionDistance < maxDistance)
			return contact;
	}

	if (!(collisionFlags & COLLISION_FLAG_IGNORE_VERTICAL_SURFACES))
	{
		CollideVerticalSurfaces<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

		if (FIRST_ONLY && contact.intersectionDistance < maxDistance)
			return contact;
	}

	return contact;
}

// Ray collision only hits surfaces.
template <typename T, typename BoundingBox>
FORCE_INLINE int CollisionCache::CountRayCollisions(const T& collisionObject, const BoundingBox& box, const ScalarType& maxDistance, U32 collisionFlags) const
{
	RP_ASSERT(m_state == STATE_READY);

	Contact contact;
	contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
	contact.intersectionDistance = maxDistance;
	contact.boundingBoxesHit = 0;

	RP_ASSERT(BoundingBoxContains(box.GetMin(), box.GetMax(), 1.5f) != 0);

	int count = 0;

	if (!(collisionFlags & COLLISION_FLAG_IGNORE_TRIANGLES))
	{
		CollideTriangleSurfaces<T, BoundingBox, false>(collisionObject, box, contact);

		if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
			contact.intersectionDistance = maxDistance;
			count++;
		}
	}

	if (!(collisionFlags & COLLISION_FLAG_IGNORE_VERTICAL_SURFACES))
	{
		CollideVerticalSurfaces<T, BoundingBox, false>(collisionObject, box, contact);

		if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
			contact.intersectionDistance = maxDistance;
			count++;
		}
	}

	return count;
}

template <typename T, typename BoundingBox, bool FIRST_ONLY>
FORCE_INLINE CollisionCache::Contact CollisionCache::CollideSweptVolume(const T& collisionObject, const BoundingBox& box, const ScalarType& maxDistance, U32 collisionFlags) const
{
	RP_ASSERT(m_state == STATE_READY);
	RP_ASSERT(m_type == CACHE_TYPE_FULL);

	Contact contact;
	contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
	contact.intersectionDistance = maxDistance;
	contact.boundingBoxesHit = 0;

	RP_ASSERT(BoundingBoxContains(box.GetMin(), box.GetMax(), 1.5f) != 0);

	if (!(collisionFlags & COLLISION_FLAG_IGNORE_TRIANGLES))
	{
		CollideTriangleSurfaces<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

		if (FIRST_ONLY && contact.intersectionDistance < maxDistance)
			return contact;

		// If didn't hit any triangle bounding boxes, it won't hit edges or vertices either.
		if (contact.boundingBoxesHit > 0)
		{
			CollideTriangleEdges<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

			if (FIRST_ONLY && contact.intersectionDistance < maxDistance)
				return contact;

			CollideTriangleVertices<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

			if (FIRST_ONLY && contact.intersectionDistance < maxDistance)
				return contact;
		}
	}

	if (!(collisionFlags & COLLISION_FLAG_IGNORE_VERTICAL_SURFACES))
	{
		CollideVerticalSurfaces<T, BoundingBox, FIRST_ONLY>(collisionObject, box, contact);

		if (FIRST_ONLY && contact.intersectionDistance < maxDistance)
			return contact;
	}

	return contact;
}

}


#endif
