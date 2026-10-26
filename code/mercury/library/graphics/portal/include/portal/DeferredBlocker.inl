
// Default c'tor.
C_DeferredBlocker::C_DeferredBlocker()
{
	// Work done in init. list.
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_DeferredBlocker::~C_DeferredBlocker()
{
	return;
}

// --------------------------------------------------------------

// Check for intersection between portal and blocker.
__forceinline NOALIAS const bool C_DeferredBlocker::Intersection(const C_Blocker* __restrict pBlocker, const C_Portal* __restrict pPortal) const
{
	const Maths::HMat43 transform		= pBlocker->GetWorldMatrix().InverseRotTrans();
	const Maths::HMat33 rot				= transform.Rotation();
	const Maths::HVec3 planeNormal		= pPortal->GetPlane().Normal() * rot;
	const Maths::C_Vertex planeVertex	= pPortal->GetVertexAtIndex(0) * transform;
	const Maths::C_AxisBox blockerAABB(-pBlocker->GetHalfExtents(), pBlocker->GetHalfExtents());
	const Maths::C_AxisBox polygonAABB(pPortal->GetVertexPtr(), pPortal->GetVertexCount(), transform);	// Loss of accuracy a little here, but should be good enough. ;)
	const bool bIntersected				= blockerAABB.Intersects(polygonAABB);
	return bIntersected;
}

