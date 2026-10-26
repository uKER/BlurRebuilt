#include <nexus_render/stdafx.h>
#include <nexus_render/BoundingBox.h>

using namespace NexusRender;

BoundingBox::BoundingBox()
{
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::SetBounds(const Aftk::C_Bounds& bounds)
{
	SetBounds(
		Tu::TemplateToNightmare(bounds.GetMin()),
		Tu::TemplateToNightmare(bounds.GetMax()));
}

void BoundingBox::SetBounds(const Maths::HVec3& min, const Maths::HVec3& max)
{
	Maths::HMat44 m;
	m.SetIdentity();
	m_pr.Clear();
	m_pr.DrawBox(
		m,
		min,
		max, 
		Maths::HVec4(1,0,0,0));
}

void BoundingBox::Draw()
{
	m_pr.Draw();
}

