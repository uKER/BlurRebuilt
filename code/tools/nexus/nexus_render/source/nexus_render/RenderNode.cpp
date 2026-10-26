#include <nexus_render/stdafx.h>

using namespace NexusRender;

bool globalScaleCompensation = true;

RenderNode::RenderNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	m_pSceneRenderer = pSceneRenderer;
	m_pAftkSceneNode = pAftkSceneNode;
	m_pParent = 0;
	m_activeLod = 0;
	memset(&m_nodeFlags, 0, sizeof(m_nodeFlags));

	//We can't pre-calculate our matrices if we're directly or indirectly animated.
	if (m_pAftkSceneNode->IsTransformIndirectlyAnimated() ||
		m_pAftkSceneNode->IsTransformAnimated())
	{
		m_worldMatrix.SetIdentity();
		m_nodeFlags.isStatic = false;
	}
	else
	{
		//Set the world matrix...
		m_worldMatrix =
			Tu::TemplateToNightmareMatrix44(m_pAftkSceneNode->GetWorldMatrix());

		m_worldMatrixTransposedInverse = 
			Tu::TemplateToNightmareMatrix44(m_pAftkSceneNode->GetWorldMatrixInverseTranspose());

		m_nodeFlags.isStatic = true;
	}
	m_worldViewProjectionMatrix.SetIdentity();

	{
		//We can take some mathmatical shortcuts later on if we know that our world matrix 
		//contains only uniform scaling.
		F32 sx, sy, sz;
		UtMaths::GetSRTS(sx, sy, sz, m_pAftkSceneNode->GetWorldMatrix());
		if (Aftk::EqualityTestFScalar(sx, sy) && Aftk::EqualityTestFScalar(sx, sz))
			m_nodeFlags.isUniformlyScaled = true;
		m_nodeFlags.isUniformlyScaled = false;
	}

	//Do we have a LOD group?
	if (pAftkSceneNode->GetParentPtr())
	{
		if (pAftkSceneNode->GetParentPtr()->GetType() == Aftk::E_ObjType_SceneLodGroup)
			m_nodeFlags.hasLodGroup = true;
		else
			m_nodeFlags.hasLodGroup = false;
	}

	//Initialise layer index array.
	for (U32 iLayer = 0; iLayer < m_pAftkSceneNode->GetNumLayers(); iLayer++)
	{
		Aftk::C_LayerNode* pLayer = m_pAftkSceneNode->GetLayerPtr(iLayer);
		S32 layerIndex = pSceneRenderer->GetLayerIndex(pLayer);
		if (layerIndex >= 0)
			m_layerIndices.PushBack(layerIndex);
	}
}

RenderNode::~RenderNode()
{
}

void RenderNode::SetActiveLodLevel(U32 lodLevel)
{
	m_activeLod = lodLevel;
}

U32 RenderNode::GetActiveLodLevel() const
{
	return m_activeLod;
}

void RenderNode::AddChild(RenderNode* pNode)
{
	pNode->m_pParent = this;
	m_children.PushBack(pNode);
}

RenderNode* RenderNode::GetParentPtr()
{
	return m_pParent;
}

const SceneRenderer* RenderNode::GetScenePtr() const
{
	return m_pSceneRenderer;
}

SceneRenderer* RenderNode::GetScenePtr()
{
	return m_pSceneRenderer;
}

const RenderNode* RenderNode::GetParentPtr() const
{
	return m_pParent;
}


U32 RenderNode::AddInstance(RenderInstanceNode* pInstance)
{
	m_instances.PushBack(pInstance);
	return (U32)m_instances.Size()-1;
}

Tu::Status RenderNode::Rebuild()
{
	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderNode::Evaluate()
{
	return Tu::Status::SUCCESS;
}

Maths::HMat44 RenderNode::GetGizmoMatrix(const Maths::HMat44& worldMatrix, F32 distanceClamp)
{
	Maths::HMat44 m = worldMatrix;
	Maths::HMat43 nm = (Maths::HMat43)m;
	Maths::C_MatrixSRT srt = nm.GetSRT();

	F32 scaling[] = {srt.Scale().X(),srt.Scale().Y(),srt.Scale().Z()};
	Maths::HMat44 minusTrans(
		m.AX(),
		m.AY(),
		m.AZ(),
		Maths::HVec4(0,0,0,1));

	//Calculate distance to camera in world space.
	Maths::HVec3 cameraPos = m_pSceneRenderer->GetCameraPtr()->GetPosition();
	Maths::HVec3 objPos(m.AW().X(), m.AW().Y(), m.AW().Z());
	F32 distance = objPos.Distance(cameraPos);
	if (distanceClamp >= 0)
	{
		if (distance > distanceClamp)
			distance = distanceClamp;
	}
	F32 zoomScale = distance * 20;

	F32 finalScale[3];
	if (fabsf(scaling[0]) < 1e-7f)
		finalScale[0] = 0;
	else
		finalScale[0] = zoomScale*(1/scaling[0]);

	if (fabsf(scaling[1]) < 1e-7f)
		finalScale[1] = 1;
	else
		finalScale[1] = zoomScale*(1/scaling[1]);

	if (fabsf(scaling[2]) < 1e-7f)
		finalScale[2] = 0;
	else
		finalScale[2] = zoomScale*(1/scaling[2]);
	
	Maths::HMat44 scale(
		Maths::HVec4(finalScale[0], 0, 0, 0),
		Maths::HVec4(0, finalScale[1], 0, 0),
		Maths::HVec4(0, 0, finalScale[2], 0),
		Maths::HVec4(0, 0, 0, 1));

	Maths::HMat44 result = scale * minusTrans;
	result.AW(m.AW());
	return result;
}


const Maths::HMat44& RenderNode::GetLocalMatrix() const
{
	return m_localMatrix;
}

Maths::HMat44 RenderNode::GetGizmoMatrixBillboard()
{
	D3DXMATRIX mView;
	m_pSceneRenderer->GetCameraPtr()->GetViewMatrix(mView);
	F32 zoomScale = m_pSceneRenderer->GetZoomScale()*400;
	
	Maths::HMat44 mViewNm;
	mViewNm.AX(Maths::HVec4(mView.m[0][0], mView.m[0][1], mView.m[0][2], mView.m[0][3]));
	mViewNm.AY(Maths::HVec4(mView.m[1][0], mView.m[1][1], mView.m[1][2], mView.m[1][3]));
	mViewNm.AZ(Maths::HVec4(mView.m[2][0], mView.m[2][1], mView.m[2][2], mView.m[2][3]));
	mViewNm.AW(Maths::HVec4(0,0,0,1));

	mViewNm = mViewNm.Transposed();

	Maths::HMat44 scale(
		Maths::HVec4(zoomScale, 0, 0, 0),
		Maths::HVec4(0, zoomScale, 0, 0),
		Maths::HVec4(0, 0, zoomScale, 0),
		Maths::HVec4(0, 0, 0, 1));

	Maths::HMat44 temp = scale * mViewNm;
	temp.AW(mViewNm.AW());

	return temp;
}

RenderInstanceNode* RenderNode::GetInstance(U32 index)
{
	return m_instances[index];
}

const RenderInstanceNode* RenderNode::GetInstance(U32 index) const
{
	return m_instances[index];
}

U32 RenderNode::GetNumInstances() const
{
	return (U32)m_instances.Size();
}

Maths::HVec3 RenderNode::GetGizmoScale() const
{
	Maths::HMat44 m = Tu::TemplateToNightmareMatrix44(m_pAftkSceneNode->GetWorldMatrix());
	Maths::HMat43 nm = (Maths::HMat43)m;
	Maths::C_MatrixSRT srt = nm.GetSRT();

	Maths::HVec3 scaling(srt.Scale().X(),srt.Scale().Y(),srt.Scale().Z());
	//Prevent div/0
	for (U32 i = 0; i < 3; i++)
	{
		if (scaling[i] < 1e-7f)
		{
			scaling[i] = 1e-7f;
		}
		scaling[i] = 1/scaling[i];
	}

	return scaling;
}
/*
const Maths::C_MatrixSRT& RenderNode::GetLocalSRT() const
{
	return m_localSRT;
}

const Maths::C_MatrixSRT& RenderNode::GetWorldSRT() const
{
	return m_worldSRT;
}
*/
bool RenderNode::IsSelected() const
{
	return m_nodeFlags.isSelected;
}

Tu::Status RenderNode::OnSelected()
{
	m_nodeFlags.isSelected = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderNode::OnDeselected()
{
	m_nodeFlags.isSelected = false;
	return Tu::Status::SUCCESS;
}

void RenderNode::FrustumCull(const Camera& camera)
{
	//Don't cull dynamic objects.
	//TODO: cull dynamic objects at some point. The problem is that only staic world bounds are
	//provided by the aftk.
	if (!m_nodeFlags.isStatic)
	{
		m_nodeFlags.isCulled = false;
		return;
	}

	const Aftk::Position3& vCentre = 
		m_pAftkSceneNode->GetWorldBounds().GetCentre();
	//Get the camera frustum.
	const Frustum& frustum = camera.GetFrustum();
	Maths::HVec3 pos(vCentre[0], vCentre[1], vCentre[2]);
	F32 radiusSq = m_pAftkSceneNode->GetWorldBounds().GetBoundingBoxRadiusSquared();
	
	//Does our frustum contain this node's bounding sphere?
	if (frustum.Contains(Sphere(pos, radiusSq)))
	{
		//We're visible! :D
		m_nodeFlags.isCulled = false;
	}
	else
	{
		//We've been culled :(
		m_nodeFlags.isCulled = true;
	}
}

bool RenderNode::IsVisible() const
{
	if (m_nodeFlags.isCulled || 
		m_nodeFlags.isHiddenByLodGroup || 
		m_nodeFlags.isHidden ||
		m_nodeFlags.isHiddenByLayer ||
		m_nodeFlags.isHiddenExpandedSource)
		return false;

	return true;
}

Tu::Status RenderNode::Update(
	bool timeChanged,
	bool scaleChanged,
	bool skinningModeChanged)
{
	m_nodeFlags.hasInitialUpdate = false;

	Tu::Status status;
	/*
	//If we're a static node that's already been evaluated once then skip this code.
	if ((!m_nodeFlags.hasInitialUpdate) || (!m_nodeFlags.isStatic && timeChanged))
	{
		//Perform scale compensation.
		if ((m_pAftkSceneNode->GetType() == AFTK_NODE_TYPE_SCENE_JOINT) &&
			((AftkSceneJointNode*)m_pAftkSceneNode)->SegmentScaleCompensation())
		{
			m_localMatrix = Tu::TemplateToNightmareMatrix44(m_pAftkSceneNode->GetLocalMatrix(time));
			m_localSRT = ((Maths::HMat43)m_localMatrix).GetSRT();

			//Compensate for scaling.
			F32 scale[3];
			if (m_pParent)
			{
				if ((m_pParent->m_pAftkSceneNode->GetType() == AFTK_NODE_TYPE_SCENE_JOINT) &&
					((AftkSceneJointNode*)m_pParent->m_pAftkSceneNode)->SegmentScaleCompensation())
				{
					scale[0] = m_pParent->m_localSRT.Scale().X();
					scale[1] = m_pParent->m_localSRT.Scale().Y();
					scale[2] = m_pParent->m_localSRT.Scale().Z();

					//Prevent div/0
					for (U32 i = 0; i < 3; i++)
					{
						if (fabsf(scale[i]) < 1e-7f)
							scale[i] = 0.f;
						else
							scale[i] = 1/scale[i];
					}

					scale[0] *= m_localSRT.Scale().X();
					scale[1] *= m_localSRT.Scale().Y();
					scale[2] *= m_localSRT.Scale().Z();

					m_worldNonCompensatedScaleExclusive = Maths::HVec3(
						1.f,
						1.f,
						1.f);
				}
				else
				{
					scale[0] = m_pParent->m_worldSRT.Scale().X();
					scale[1] = m_pParent->m_worldSRT.Scale().Y();
					scale[2] = m_pParent->m_worldSRT.Scale().Z();

					m_worldNonCompensatedScaleExclusive = Maths::HVec3(
						m_pParent->m_worldSRT.Scale().X(),
						m_pParent->m_worldSRT.Scale().Y(),
						m_pParent->m_worldSRT.Scale().Z());
				}

				scale[0] *= m_localSRT.Scale().X();
				scale[1] *= m_localSRT.Scale().Y();
				scale[2] *= m_localSRT.Scale().Z();
				scale[0] *= m_pParent->m_worldNonCompensatedScaleExclusive.X();
				scale[1] *= m_pParent->m_worldNonCompensatedScaleExclusive.Y();
				scale[2] *= m_pParent->m_worldNonCompensatedScaleExclusive.Z();
			}
			else
			{
				scale[0] = m_localSRT.Scale().X();
				scale[1] = m_localSRT.Scale().Y();
				scale[2] = m_localSRT.Scale().Z();
			}


			//Rotation
			Maths::C_MatrixRotateRPH parentRPH =
				((Maths::HMat33)m_pParent->m_worldMatrix).GetRotationRPH();
            Maths::HMat44 parentRotation(parentRPH);

			Maths::C_MatrixRotateRPH localRPH;
			Maths::HMat44 localRotation(localRPH);

			Maths::HMat44 rotation = parentRotation * localRotation;
			Maths::C_MatrixRotateRPH rph = ((HMat33)rotation).GetRotationRPH();
			
			//Translation
			Maths::HVec3 translation = m_localSRT.Translate();
			if (m_pParent)
			{
				F32 tempScale[3];
				tempScale[0] = m_pParent->m_localSRT.Scale().X();
				tempScale[1] = m_pParent->m_localSRT.Scale().Y();
				tempScale[2] = m_pParent->m_localSRT.Scale().Z();
				tempScale[0] *= m_pParent->m_worldNonCompensatedScaleExclusive.X();
				tempScale[1] *= m_pParent->m_worldNonCompensatedScaleExclusive.Y();
				tempScale[2] *= m_pParent->m_worldNonCompensatedScaleExclusive.Z();
				translation = translation * Maths::HVec3(
					tempScale[0],
					tempScale[1],
					tempScale[2]);
				translation = translation + m_pParent->m_worldSRT.Translate();
			}

			m_worldSRT = C_MatrixSRT(HVec3(scale[0], scale[1], scale[2]), rph, translation);
			//Store our world matrix.
			m_worldMatrix = (Maths::HMat44)Maths::HMat43(m_worldSRT);
			g_debugManager << "Scene node [" << m_pAftkSceneNode->GetName() << "] " << 
				" trans " << m_worldMatrix.AW().X() << "," << m_worldMatrix.AW().Y() << "," << m_worldMatrix.AW().Z() <<
				" scale comp " << m_worldSRT.Scale().X() << "," << m_worldSRT.Scale().Y() << "," << m_worldSRT.Scale().Z() << "\n";
		}
		else
		{
			m_worldNonCompensatedScaleExclusive = Maths::HVec3(
				1.f,
				1.f,
				1.f);

			m_localMatrix = 
				Tu::TemplateToNightmareMatrix44(m_pAftkSceneNode->GetLocalMatrix(time));

			//Set the world matrix...
			m_worldMatrix =
				Tu::TemplateToNightmareMatrix44(
				m_pAftkSceneNode->GetWorldMatrix(time));

			m_worldMatrixTransposedInverse = 
				Tu::TemplateToNightmareMatrix44(m_pAftkSceneNode->GetWorldMatrixInverseTranspose(time));

			m_localSRT = ((Maths::HMat43)m_localMatrix).GetSRT();
			m_worldSRT = ((Maths::HMat43)m_worldMatrix).GetSRT();
		}
	}
	if (timeChanged)
	{
		status = Evaluate(time);
		if (!status) return status;
	}
*/
	//If we're a static node that's already been evaluated once then skip this code.
	if ((!m_nodeFlags.hasInitialUpdate) || (!m_nodeFlags.isStatic && timeChanged))
	{
		//Set the world matrix...
		m_worldMatrix =
			Tu::TemplateToNightmareMatrix44(
			m_pAftkSceneNode->GetWorldMatrix());

		m_worldMatrixTransposedInverse = 
			Tu::TemplateToNightmareMatrix44(m_pAftkSceneNode->GetWorldMatrixInverseTranspose());
	}
	if (timeChanged)
	{
		status = Evaluate();
		if (!status) return status;
	}

	//Set visibility by layer.
	m_nodeFlags.isHiddenByLayer = false;
	for (U32 iLayer = 0; iLayer < (U32)m_layerIndices.Size(); iLayer++)
	{
		if (!m_pSceneRenderer->GetLayerInfoPtr(m_layerIndices[iLayer])->isVisible)
		{
			m_nodeFlags.isHiddenByLayer = true;
			return Tu::Status::SUCCESS;
		}
	}

	const Camera& camera = *m_pSceneRenderer->GetCameraPtr();
	//Set cull state.
	FrustumCull(camera);

	if (m_nodeFlags.isCulled)
		return Tu::Status::SUCCESS;

	HideByLodGroup();
	if (m_nodeFlags.isHiddenByLodGroup)
		return Tu::Status::SUCCESS;

	Maths::HMat44 cameraViewMatrix;
	Maths::HMat44 cameraProjectionMatrix;
	// Get the projection & view matrix from the camera class
	camera.GetViewMatrix(cameraViewMatrix);
	camera.GetProjectionMatrix(cameraProjectionMatrix);
	m_worldViewProjectionMatrix = m_worldMatrix * cameraViewMatrix * cameraProjectionMatrix;
	m_nodeFlags.hasInitialUpdate = true;

	if (m_pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneNode)
	{
		m_nodeFlags.isHidden = (!m_pSceneRenderer->GetSettingsPtr()->renderNulls);
	}
	else if (m_pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneJoint)
	{
		m_nodeFlags.isHidden = (!m_pSceneRenderer->GetSettingsPtr()->renderJoints);
	}

	if (!m_nodeFlags.isHidden)
		SetActiveLodLevel();
	
	return Tu::Status::SUCCESS;
}

void RenderNode::HideByLodGroup()
{
	m_nodeFlags.isHiddenByLodGroup = false;
	//Set level of detail for members of a lod group.
	if (!m_nodeFlags.hasLodGroup)
		return;

	Aftk::C_SceneLodGroupNode* pAftkSceneLodGroupNode = 
		(Aftk::C_SceneLodGroupNode*)m_pAftkSceneNode->GetParentPtr();

	//Skip LOD groups with fewer than two levels of detail.
	if (pAftkSceneLodGroupNode->GetNumLodLevels() > 1)
	{
		//Calculate the distance using the bounds of the first lod.
		F32 distance;
		{
			const Aftk::C_GroupLodLevelInfo* pInfo = 
				&pAftkSceneLodGroupNode->GetLodLevelInfo(0);
			//We're parented to a lod group - set visibility flag.
			Maths::HVec3 cameraPosWorld(m_pSceneRenderer->GetCameraPtr()->GetPosition());
			Aftk::Position3 temp = pInfo->GetSceneNodePtr()->GetWorldBounds().GetCentre();
			Maths::HVec3 sceneNodePosWorld(Tu::TemplateToNightmare(temp));
			distance = sceneNodePosWorld.Distance(cameraPosWorld);
		}

		//Find the lod level our current node represents.
		S32 thisLodNumber;
		U32 groupActiveLod;

		//Pass 1: Find active lod level.
		for (U32 iLodLevel = 0; iLodLevel < pAftkSceneLodGroupNode->GetNumLodLevels(); iLodLevel++)
		{
			const Aftk::C_GroupLodLevelInfo* pInfo = 
				&pAftkSceneLodGroupNode->GetLodLevelInfo(iLodLevel);

			if (iLodLevel == pAftkSceneLodGroupNode->GetNumLodLevels()-1)
			{
				//We've reached the lowest LOD
				groupActiveLod = iLodLevel;
				break;
			}

			if (distance < pInfo->GetThreshold())
			{
				groupActiveLod = iLodLevel;
				break;
			}
		}
		
		//Pass 2: Find which lod level our node is.
		for (U32 iLodLevel = 0; iLodLevel < pAftkSceneLodGroupNode->GetNumLodLevels(); iLodLevel++)
		{
			const Aftk::C_GroupLodLevelInfo* pInfo = 
				&pAftkSceneLodGroupNode->GetLodLevelInfo(iLodLevel);
			if (pInfo->GetSceneNodePtr() == m_pAftkSceneNode)
			{
				thisLodNumber = iLodLevel;
				break;
			}
		}

		//We didn't find the lod item node.
		if (thisLodNumber == -1)
			return;

		if (thisLodNumber != groupActiveLod)
			m_nodeFlags.isHiddenByLodGroup = true;
	}
}

void RenderNode::SetActiveLodLevel()
{
}

void RenderNode::Clear()
{
}

RenderNode::ERenderNodeType RenderNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_NODE;
}

bool RenderNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (type == RenderNode::GetType()) 
		return true;
	return false;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RenderNode::RenderNode()
{
}

#endif
