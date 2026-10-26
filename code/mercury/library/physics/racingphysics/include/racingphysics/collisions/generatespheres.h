#ifndef RP_GENERATE_SPHERES_H
#define RP_GENERATE_SPHERES_H
#ifdef _MSC_VER
#pragma once
#endif

#include "simplemesh.h"
#include "collisioncache.h"
#include "cluster.h"
#include "btl/utility/scopedpointer.h"
#include "btl/containers/list.h"

namespace RacingPhysics
{

class GenerateSpheres
{
public:
	static GenerateSpheres& Instance() {return s_instance;}

	void Generate(const SimpleMesh* pSimpleMesh);
	void Load(const char* filename);
	void Shutdown();
	void DisplayDebug() const;

	///////////////////////////////////////////////////

	// output data
	struct Sphere
	{
		Sphere() {}
		Sphere(const Point3R& p, float r) : position(p), radius(r) {}

		Point3R position;
		float radius;

		// for clustering
		Vector3R GetCentroid() const {return MakeVector(position);}
	};

	struct Node
	{
		Sphere sphere;
		Btl::Vector<Node> subNodes;
	};

	Btl::ScopedPointer<Node> pTree;

	///////////////////////////////////////////////////////////

	// clustering
	typedef KMeans::Cluster<Sphere, KMeans::Centroid<Vector3R> > Cluster;
	typedef Btl::List<Cluster> ClusterList;

private:
	GenerateSpheres() {}
	static GenerateSpheres s_instance;

	void Generate();
	void CalculateExtents();
	void GeneratePoints();
	void BuildCollisionCache();
	bool Inside(const Point3R& p) const;
	float DistanceFromSurface(const Point3R& p) const;
	void MakeClusters(float maxClusterSize);
	void BuildHierarchy();
	void DisplaySpheres(const Node& node, int level) const;

	Point3R Transform(const Point3R& p) const;
	Point3R Restore(const Point3R& p) const;

	// temporary data for building
	Btl::ScopedPointer<SimpleMesh> m_pLoadedMesh;
	const SimpleMesh* m_pSimpleMesh;
	Btl::Vector<Sphere> m_clusterInput;
	Point3R m_min, m_max;
	CollisionCache m_collisionCache;
	ClusterList m_clusterList;
};

////////////////////////////////////////////////////////////////////////

template <typename Archive>
void Serialise(Archive& ar, const GenerateSpheres::Sphere& data)
{
	ar << data.position;
	ar << data.radius;
}

template <typename Archive>
void Deserialise(Archive& ar, GenerateSpheres::Sphere& data)
{
	ar.operator >> (data.position);
	ar >> data.radius;
}

template <typename Archive>
void Serialise(Archive& ar, const GenerateSpheres::Node& data)
{
	ar << data.sphere;
	ar << data.subNodes;
}

template <typename Archive>
void Deserialise(Archive& ar, GenerateSpheres::Node& data)
{
	ar >> data.sphere;
	ar >> data.subNodes;
}

//////////////////////////////////////////////////////////////////////

struct ExportCollisionData
{
	const static int VERSION = 0;

	SimpleMesh m_simpleMesh;
	Btl::ScopedPointer<GenerateSpheres::Node> m_pSphereData;
};

// serialisation
template <typename Archive>
void Deserialise(Archive& ar, ExportCollisionData& data)
{
	int version; ar >> version;
	ar >> data.m_simpleMesh;
	U32 b; ar >> b;

	if (b)
	{
		Allocate(data.m_pSphereData);
		ar >> *data.m_pSphereData;
	}
	
}

template <typename Archive>
void Serialise(Archive& ar, const ExportCollisionData& data)
{
	ar << ExportCollisionData::VERSION;
	ar << data.m_simpleMesh;

	if (data.m_pSphereData != 0)
	{
		ar << (U32) 1;
		ar << data.m_pSphereData;
	}
	else
	{
		ar << (U32) 0;
	}
}

//////////////////////////////////////////////////////////////

#ifdef _CONVERTER

class CarCollisionDataBuild : public ExportCollisionData
{
public:
	void Export(Aftk::C_Scene& scene, std::string& exportName);

	void operator () (Aftk::C_SceneMeshNode& mesh);
	void operator () (Aftk::C_SceneNode& node) {}

private:
	
};

#endif

}

#endif
