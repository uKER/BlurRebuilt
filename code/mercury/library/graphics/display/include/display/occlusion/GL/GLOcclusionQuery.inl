INLINE const bool C_OcclusionQuery::IsCreated() const
{
	return false;
}

INLINE const E_OcclusionQueryResult C_OcclusionQuery::Visible() const
{
	return E_OcclusionQueryResult_Failed;
}

INLINE const E_OcclusionQueryResult C_OcclusionQuery::Visible(U32& visible) const
{
	return E_OcclusionQueryResult_Failed;
}

INLINE const E_OcclusionQueryResult C_OcclusionQuery::Visible(F32& visible) const
{
	return E_OcclusionQueryResult_Failed;
}
