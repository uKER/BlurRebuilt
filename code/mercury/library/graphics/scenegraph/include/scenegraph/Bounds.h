#ifndef SCENE_GRAPH_BOUNDS_H
#define SCENE_GRAPH_BOUNDS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace RenderingData
{
	struct Bounds;
}

namespace SceneGraph
{

// stores either axis aligned BB, or sphere

class C_Bounds
{
public:
	void 						SetAxisBox(const RenderingData::Bounds &bounds);
	void 						SetAxisBox(const Maths::HVec3& minPoint, const Maths::HVec3& maxPoint);
	
	void 						SetSphere(const Maths::C_Sphere& sphere);
	Maths::C_Sphere				GetSphere() const;
	void 						SetSpherePosition(const Maths::HVec3& position);

	Maths::HVec3				GetCentre() const { return Maths::Load<Maths::HVec3>(&centre); }
	F32							GetRadius() const { return size.w; }

	void						SetNearFarDistance(const F32 near, const F32 far);
	F32							GetNearDistance() const { return clip.x; }
	F32							GetFarDistance() const { return clip.y; }

	MathsStorage::Float4A16		centre;		// centre, 1.0 in w
	MathsStorage::Float4A16		size;		// size, radius in w
	MathsStorage::Float4A16		clip;		// clip, x near clip, y far clip
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_BOUNDS_H
