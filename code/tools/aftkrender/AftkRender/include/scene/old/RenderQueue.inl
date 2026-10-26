

	bool C_RenderTaskFrontToBackSort::operator()(const C_RenderTask& lhs, const C_RenderTask& rhs)
	{
		const C_SceneRenderer *pRenderer = lhs.m_pRenderMeshNode->GetRenderer();
	
		const C_RenderMeshCluster *plhsCluster = lhs.m_pRenderMeshNode->GetMeshClusterPtr(lhs.m_LodLevel, lhs.m_RenderMeshClusterIndex);
		const C_RenderMeshCluster *prhsCluster = rhs.m_pRenderMeshNode->GetMeshClusterPtr(rhs.m_LodLevel, rhs.m_RenderMeshClusterIndex);

		S32 lhsMatIdx = plhsCluster->GetAftkMaterialIndex();
		S32 rhsMatIdx = prhsCluster->GetAftkMaterialIndex();

		// material has the higher sorting priority
		if (lhsMatIdx<rhsMatIdx) return true;
		if (rhsMatIdx<lhsMatIdx) return false;

		const Maths::HVec3 camPos = Display::Device()->GetView()->GetViewSettings3D().GetCameraPosition();
		const Maths::HVec3 lhsPosition = lhs.GetRenderMeshNode()->GetWorldAABBox().Centre();
		const Maths::HVec3 rhsPosition = rhs.GetRenderMeshNode()->GetWorldAABBox().Centre();

		// distance comparision should be good enough
		if (camPos.DistanceSquared(lhsPosition) < camPos.DistanceSquared(rhsPosition))
			return true;
		else return false;
	};


	bool C_RenderTaskBackToFrontSort::operator()(const C_RenderTask& lhs, const C_RenderTask& rhs)
	{
		const C_SceneRenderer *pRenderer = lhs.m_pRenderMeshNode->GetRenderer();
	
		const C_RenderMeshCluster *plhsCluster = lhs.m_pRenderMeshNode->GetMeshClusterPtr(lhs.m_LodLevel, lhs.m_RenderMeshClusterIndex);
		const C_RenderMeshCluster *prhsCluster = rhs.m_pRenderMeshNode->GetMeshClusterPtr(rhs.m_LodLevel, rhs.m_RenderMeshClusterIndex);

		S32 lhsMatIdx = plhsCluster->GetAftkMaterialIndex();
		S32 rhsMatIdx = prhsCluster->GetAftkMaterialIndex();

		// material has the higher sorting priority
		if (lhsMatIdx<rhsMatIdx) return true;
		if (rhsMatIdx<lhsMatIdx) return false;

		const Maths::HVec3 camPos = Display::Device()->GetView()->GetViewSettings3D().GetCameraPosition();
		const Maths::HVec3 lhsPosition = lhs.GetRenderMeshNode()->GetWorldAABBox().Centre();
		const Maths::HVec3 rhsPosition = rhs.GetRenderMeshNode()->GetWorldAABBox().Centre();

		// distance comparision should be good enough
		if (camPos.DistanceSquared(lhsPosition) > camPos.DistanceSquared(rhsPosition))
			return true;
		else return false;
	};