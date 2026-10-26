#include <nexus_render/stdafx.h>

using namespace NexusRender;

RenderSplineNode::RenderSplineNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode)
: RenderNode(
	pSceneRenderer,
	pAftkSceneNode)
{
}

RenderSplineNode::~RenderSplineNode()
{
	m_splineVBWrapper.Destroy();
}

Tu::Status RenderSplineNode::Rebuild()
{
	Aftk::C_SceneSplineNode* pSpline = (Aftk::C_SceneSplineNode*)GetAftkSceneNodePtr();
//	pSpline->SetDegree(1);
	if (pSpline->GetNumCVs() < 1)
	{
		m_nodeFlags.isBuilt = true;
		return Tu::Status::SUCCESS;
	}

	S32 numSubDivisions;
	F32 stepSize = 0;

	//Step in terms of cvs
	if (pSpline->GetDegree() == 1)
	{
		numSubDivisions = pSpline->GetNumCVs();
		pSpline->SetNumSubDivisions(numSubDivisions);
	}
	//Step in terms if distance.
	else
	{
		numSubDivisions = pSpline->GetNumCVs()*10;
		pSpline->SetNumSubDivisions(numSubDivisions);
	}

	if (!pSpline->GetEvaluationStepSize(stepSize))
		return Tu::Status::FAILURE;
	
	//Clear old buffer.
	m_splineVBWrapper.Destroy();

	m_splineVBWrapper.Create(
		sizeof(D3DVertexPosCol),
		((numSubDivisions+pSpline->GetNumCVs()))*2,
		1);

	m_splineVBWrapper.Lock();
	D3DVertexPosCol v;
	v.SetColour(0, 0, 204, 0);

	U32 vtx = 0;

	F32 acc0 = 0;
	F32 acc1 = stepSize;
	for (S32 i = 0; i < numSubDivisions; i++)
	{
		F32 u0;
		F32 u1;

		if (pSpline->GetDegree() == 1)
		{
			u0 = (F32)i;
			u1 = (F32)i+1.f;
		}
		else
		{
			pSpline->EvaluateLength(
				u0,
				acc0);
			pSpline->EvaluateLength(
				u1,
				acc1);

			acc0+=stepSize;
			acc1+=stepSize;
		}

		Aftk::Position3 aftkCV0;
		if (!pSpline->Evaluate(aftkCV0, u0))
			return Tu::Status::FAILURE;
		Maths::HVec3 cv0(aftkCV0[0], aftkCV0[1], aftkCV0[2]);

		Aftk::Position3 aftkCV1;
		if (!pSpline->Evaluate(aftkCV1, u1))
			return Tu::Status::FAILURE;
		Maths::HVec3 cv1(aftkCV1[0], aftkCV1[1], aftkCV1[2]);

		v.SetPosition(cv0); 
		m_splineVBWrapper.SetVertex(vtx, &v);
		v.SetPosition(cv1); 
		m_splineVBWrapper.SetVertex(vtx+1, &v);

		vtx += 2;
	}

//	v.SetColour(255, 255, 204, 0);
	//Render normals
	for (U32 iCV = 0; iCV < pSpline->GetNumCVs(); iCV++)
	{
		F32 u0 = (F32)iCV + 0.45f;
		F32 mid = (F32)iCV + 0.5f;
		F32 u1 = (F32)iCV + 0.55f;

		Aftk::Position3 aftkCV0;
		if (!pSpline->Evaluate(aftkCV0, u0))
			return Tu::Status::FAILURE;
		Maths::HVec3 cv0(aftkCV0[0], aftkCV0[1], aftkCV0[2]);

		Aftk::Position3 aftkMid;
		if (!pSpline->Evaluate(aftkMid, mid))
			return Tu::Status::FAILURE;
		Maths::HVec3 midPoint(aftkMid[0], aftkMid[1], aftkMid[2]);

		Aftk::Position3 aftkCV1;
		if (!pSpline->Evaluate(aftkCV1, u1))
			return Tu::Status::FAILURE;
		Maths::HVec3 cv1(aftkCV1[0], aftkCV1[1], aftkCV1[2]);

		F32 length;
		Maths::HVec3 dir = cv1 - cv0;
		length = dir.Length();
		dir.Normalise();

		Maths::HVec3 orientationVector = 
			dir ^ Maths::HVec3(0.f, 1.f, 0.f);

		v.SetPosition(midPoint); m_splineVBWrapper.SetVertex(vtx, &v);
		v.SetPosition(midPoint + orientationVector * 2.f); m_splineVBWrapper.SetVertex(vtx+1, &v);
		
		vtx += 2;
	}
	
	m_splineVBWrapper.Unlock();

	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderSplineNode::Update(
	bool timeChanged,
	bool scaleChanged,
	bool skinningModeChanged) 
{
	Tu::Status status;
	status = RenderNode::Update(
		timeChanged,
		scaleChanged,
		skinningModeChanged);
	if (!status) return status;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderSplineNode::Evaluate()
{
	Tu::Status status;
	status = RenderNode::Evaluate();
	if (!status) return status;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderSplineNode::Draw(U32 lodLevel)
{
	m_splineVBWrapper.Draw(D3DPT_LINELIST);
	return Tu::Status::SUCCESS;
}

RenderNode::ERenderNodeType RenderSplineNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_SPLINE;
}

bool RenderSplineNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderSplineNode::GetType()) 
		return true;

	return false;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RenderSplineNode::RenderSplineNode()
{
}

#endif
