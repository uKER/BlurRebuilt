#include "stdafx.h"
#include "racingphysics/collisions/generatespheres.h"

#include "racingphysics/collisions/raycast.h"
#include "racingphysics/physics/random.h"
#include "racingphysics/collisions/simplemesh.h"

//#ifndef _CONVERTER
//#include "render\d3d.h"
//#include "render\draw.h"
//#include "render\render.h"
//#include "render\viewport.h"
//#endif

namespace RacingPhysics
{

GenerateSpheres GenerateSpheres::s_instance;

//#pragma optimize("", off)

static float s_stepSize = 0.04f;
static float s_minDistanceFromEdge = 0.1f;
static float s_maxDistanceFromEdge = 0.4f;
static float s_clusterSize = 0.5f;
static float s_clusterSizeLevel2 = 1.f;
static int s_clusterIterations = 20;

//using namespace Maths;

////////////////////////////////////////////////

void GenerateSpheres::Generate(const SimpleMesh* pSimpleMesh)
{
	Shutdown();

	m_pSimpleMesh = pSimpleMesh;
	
	if (m_pSimpleMesh && m_pSimpleMesh->GetNumTriangles() > 0)
		Generate();

	m_pSimpleMesh = 0;
}

void GenerateSpheres::Load(const char* filename)
{
	RP_ASSERT(!"Should have dirty-disked before now.");

	Shutdown();
/*
	// load data
	Sys::I_File* f = Sys::GetDefaultFileSystem()->Open("DontShip\\carcollision.dat", Sys::E_FileAccess_Read);

	if (f)
	{
		Allocate(m_pLoadedMesh);
		Sys::BinaryDeserialiser<Ut::BigEndianToNative> ar(*f);
		int version;
		ar >> version;
		ar >> *m_pLoadedMesh;
		ar >> pTree;
		m_pSimpleMesh = Pointer(m_pLoadedMesh);
		delete f;
	}
	*/
}

void GenerateSpheres::Shutdown()
{
	m_clusterInput.Clear();
	m_collisionCache.Clear();
	m_clusterList.Clear();
	Free(pTree);
	Free(m_pLoadedMesh);
}

/////////////////////////////////////////////////////////////

void GenerateSpheres::Generate()
{
	CalculateExtents();
	BuildCollisionCache();
	GeneratePoints();
	MakeClusters(s_clusterSize);
	BuildHierarchy();
}

void GenerateSpheres::CalculateExtents()
{
	m_min = Maths::Constants<Point3R>::Max();
	m_max = Maths::Constants<Point3R>::Min();

	for (int i = 0; i < m_pSimpleMesh->GetNumTriangles(); i++)
	{
		const SimpleMesh::Triangle& triangle(m_pSimpleMesh->GetTriangle(i));

		UpdateBounds(&m_min, &m_max, Transform(triangle.v0));
		UpdateBounds(&m_min, &m_max, Transform(triangle.v1));
		UpdateBounds(&m_min, &m_max, Transform(triangle.v2));
	}
}

Point3R GenerateSpheres::Transform(const Point3R& p) const
{
	//return Point3R(p.X(), p.Y() * (1.f + Abs(p.Z() / 2.f)), p.Z());
	return p;
	//return Point3R(p.X(), p.Y() * 2.f, p.Z());
	//float f = 1.f;//1.f + Abs(p.Z()) / 2.f;
	//return MakePoint(MakeVector(p) * f);
}

Point3R GenerateSpheres::Restore(const Point3R& p) const
{
	return p;
	//return Point3R(p.X(), p.Y() / 2.f, p.Z());
	//float f = 1.f;//(-1.f + Sqrt(1.f + Abs(p.Z()) * 2.f)) / Abs(p.Z());
	//return MakePoint(MakeVector(p) * f);
}

void GenerateSpheres::BuildCollisionCache()
{
	for (int i = 0; i < m_pSimpleMesh->GetNumTriangles(); i++)
	{
		const SimpleMesh::Triangle& triangle(m_pSimpleMesh->GetTriangle(i));

		m_collisionCache.AddTriangleNoSharing(	Transform(triangle.v0), 
												Transform(triangle.v1),
												Transform(triangle.v2));
	}

	m_collisionCache.Finish();
}

static float s_testHeight = 0.25f;
static float s_testGradient = 0.2f;

bool GenerateSpheres::Inside(const Point3R& p) const
{
	float size = Magnitude(m_max - m_min) + 1.f;
	float f = 1.f + s_testGradient * (p.Y() - m_min.Y()) / (m_max.Y() - m_min.Y());
	Point3R p2(p.X() * f, m_min.Y() + s_testHeight, p.Z() * ((p.Z() > 0.f) ? 1.f : f));

	Normal3R n[] = {Maths::Constants<Normal3R>::Y(),
					-Maths::Constants<Normal3R>::Y(),
					 Maths::Constants<Normal3R>::X(),
					-Maths::Constants<Normal3R>::X(),
					 Maths::Constants<Normal3R>::Z(),
					 -Maths::Constants<Normal3R>::Z()
					};

	bool inside = true;

	for (int i = 0; i < sizeof(n) / sizeof(n[0]); i++)
	{
		RayCast rayCast(p2, n[i], size);
		int hit = rayCast.CountCollisions(m_collisionCache);
		if ((hit % 2) == 0)
			inside = false;
	}

	return inside;
}

float GenerateSpheres::DistanceFromSurface(const Point3R& p) const
{
	FloatVR size = Magnitude(m_max - m_min) + 1.f;
	FloatVR f = 1.f + s_testGradient * (p.Y() - m_min.Y()) / (m_max.Y() - m_min.Y());
	Point3R p2(p.X() * f, m_min.Y() + s_testHeight, p.Z());// * ((p.Z() > 0.f) ? 1.f : f));

	Normal3R n[] = {//Constants<Normal3R>::Y(),
					//-Constants<Normal3R>::Y(),
					 Maths::Constants<Normal3R>::X(),
					-Maths::Constants<Normal3R>::X(),
					 Maths::Constants<Normal3R>::Z(),
					 -Maths::Constants<Normal3R>::Z()
					};

	FloatVR dmin = FLT_MAX;

	for (int i = 0; i < sizeof(n) / sizeof(n[0]); i++)
	{
		RayCast rayCast(p2, n[i], size);
		CollisionCache::Contact contact = rayCast.GetCollision(m_collisionCache);
		if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE) 
			dmin = Min(FloatVR(contact.intersectionDistance), dmin);
	}

	dmin = Min(dmin, Abs(p.Y() - m_min.Y()));
	dmin = Min(dmin, Abs(p.Y() - m_max.Y()));

	return dmin;
}

void GenerateSpheres::GeneratePoints()
{
	Vector3R size = m_max - m_min;
	float step = s_stepSize;

	int nx = int(size.X() / step);
	int ny = int(size.Y() / step);
	int nz = int(size.Z() / step);

	for (int ix = 0; ix < nx; ix++)
	for (int iy = 0; iy < ny; iy++)
	for (int iz = 0; iz < nz; iz++)
	{
		float x = m_min.X() + size.X() * ((float) ix / float(nx - 1));
		float y = m_min.Y() + size.Y() * ((float) iy / float(ny - 1));
		float z = m_min.Z() + size.Z() * ((float) iz / float(nz - 1));

		Point3R p(x, y, z);

		if (Inside(p))
		{
			float d = DistanceFromSurface(p);
			
			if (d >= s_minDistanceFromEdge && d <= s_maxDistanceFromEdge)
				m_clusterInput.PushBack(Sphere(p, 0.f));
		}
	}
}

GenerateSpheres::Cluster* FindNearestCluster(GenerateSpheres::ClusterList& clusters, const Vector3R& p)
{
	float min = FLT_MAX;
	GenerateSpheres::Cluster* nearest = 0;

	for (GenerateSpheres::ClusterList::Iterator cluster = clusters.Begin(); cluster != clusters.End(); ++cluster)
	{
		float d = Magnitude(cluster->GetCentroid().Mean() - p);

		if (d < min)
		{
			nearest = &*cluster;
			min = d;
		}
	}

	//RP_ASSERT(nearest);
	return nearest;
}

void GenerateSpheres::MakeClusters(float maxClusterSize)
{
	Vector3R size = m_max - m_min;

	for (int i = 0; i < m_clusterInput.Size(); i++)
	{
		Vector3R v = MakeVector(m_clusterInput[i].position);
		Cluster* pCluster = FindNearestCluster(m_clusterList, v);

		if (!pCluster || Magnitude(pCluster->GetCentroid().Mean() - v) > FloatVR(maxClusterSize))
		{
			m_clusterList.PushBack(Cluster(m_clusterInput[i]));
			pCluster = &m_clusterList.Back();
		}

		pCluster->items.PushBack(m_clusterInput[i]);
	}

	for (int count = 0; count < s_clusterIterations; count++)
	{
		// insert the clusters into a new list
		ClusterList newClusters;

		bool sameClusters = true;

		for (ClusterList::Iterator cluster = m_clusterList.Begin(); cluster != m_clusterList.End(); ++cluster)
		{
			if (cluster->items.Size() > 0)
			{
				Cluster::CentroidType newCentroid = cluster->ComputeCentroid();
				newClusters.PushBack(Cluster(newCentroid));

				if (Magnitude(newCentroid.Mean() - cluster->GetCentroid().Mean()) > 0.001f)
					sameClusters = false;
			}
		}

		if (sameClusters)
			break;

		for (ClusterList::Iterator cluster = m_clusterList.Begin(); cluster != m_clusterList.End(); ++cluster)
		{
			for (int i = 0; i < cluster->items.Size(); i++)
			{
				const Vector3R& p = MakeVector(cluster->items[i].position);

				// find the nearest cluster
				Cluster* newCluster = FindNearestCluster(newClusters, p);

				// add the item
				newCluster->items.PushBack(cluster->items[i]);
			}
		}

		Swap(m_clusterList, newClusters);
	}
}

void GenerateSpheres::BuildHierarchy()
{
	//return;
	m_clusterInput.Clear();

	Allocate(pTree);
	
	for (ClusterList::Iterator cluster = m_clusterList.Begin(); cluster != m_clusterList.End(); ++cluster)
	{
		float d_max = 0.f;
		Point3R c = Restore(MakePoint(cluster->GetCentroid().Mean()));

		for (int i = 0; i < cluster->items.Size(); i++)
		{
			Point3R p = Restore(cluster->items[i].position);
			
			float d = Magnitude(c - p) + cluster->items[i].radius;
			d_max = Max(d_max, d);
		}

		Sphere sphere(c, d_max);

		m_clusterInput.PushBack(sphere);
	}

	m_clusterList.Clear();

	MakeClusters(s_clusterSizeLevel2);

	for (ClusterList::Iterator cluster = m_clusterList.Begin(); cluster != m_clusterList.End(); ++cluster)
	{
		pTree->subNodes.PushBack();

		float d_max = 0.f;

		for (int i = 0; i < cluster->items.Size(); i++)
		{
			const Vector3R& p = MakeVector(cluster->items[i].position);
			float d = Magnitude(cluster->GetCentroid().Mean() - p) + cluster->items[i].radius;
			d_max = Max(d_max, d);

			pTree->subNodes.Back().subNodes.PushBack();
			pTree->subNodes.Back().subNodes.Back().sphere = cluster->items[i];
		}

		Sphere sphere(MakePoint(cluster->GetCentroid().Mean()), d_max);
		pTree->subNodes.Back().sphere.position = MakePoint(cluster->GetCentroid().Mean());
		pTree->subNodes.Back().sphere.radius = d_max;
	}
}

/////////////////////////////////////////////////////////////

static bool g_showClusters = true;
static bool g_showMesh = true;
static int g_showSpheres = 2;

void GenerateSpheres::DisplayDebug() const
{
	/*Viewport3D::CullingOff();
	DefaultEffect::SetVertexMode(VERTEX_MODE_TRANSFORM);
	DefaultEffect::SetBlendMode(BLEND_MODE_SOURCE);
    DefaultEffect::SetRenderMode(RENDER_MODE_DIFFUSE);
	DefaultEffect::AutoRenderStateChange<D3DRS_ZENABLE, D3DZB_FALSE> autoZ;
	Viewport3D::SetWorldTransformation_Full();

	if (g_showMesh && m_pLoadedMesh != 0)
	{
		for (int i = 0; i < m_pLoadedMesh->GetNumTriangles(); i++)
		{
			const SimpleMesh::Triangle& triangle(m_pLoadedMesh->GetTriangle(i));

			DynamicVB::Begin(D3DPT_LINESTRIP);
			DynamicVB::SetVertexDataColor(D3DVSDE_DIFFUSE, ColourARGB::Blue);

			DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, triangle.v0.X(), triangle.v0.Y(), triangle.v0.Z(), 1);
			DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, triangle.v1.X(), triangle.v1.Y(), triangle.v1.Z(), 1);	
			DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, triangle.v2.X(), triangle.v2.Y(), triangle.v2.Z(), 1);
			DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, triangle.v0.X(), triangle.v0.Y(), triangle.v0.Z(), 1);

			DynamicVB::End();
		}
	}

	if (g_showClusters)
	{
		for (ClusterList::ConstIterator cluster = m_clusterList.Begin(); cluster != m_clusterList.End(); ++cluster)
		{
			for (int i = 0; i < cluster->items.Size(); i++)
			{
				Point3R p(cluster->items[i].position);
				//p.Y() = m_min.Y() + s_testHeight;

				DynamicVB::Begin(D3DPT_LINELIST);
				DynamicVB::SetVertexDataColor(D3DVSDE_DIFFUSE, (ColourARGB) Noisei((int) &*cluster));

				DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p.X() + 0.025f, p.Y(), p.Z(), 1);
				DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p.X() - 0.025f, p.Y(), p.Z(), 1);
				DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p.X(), p.Y() + 0.025f, p.Z(), 1);
				DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p.X(), p.Y() - 0.025f, p.Z(), 1);
				DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p.X(), p.Y(), p.Z() + 0.025f, 1);
				DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p.X(), p.Y(), p.Z() - 0.025f, 1);

				DynamicVB::End();
			}
		}
	}

	if (pTree != 0 && g_showSpheres > 0) DisplaySpheres(*pTree, 0);*/
}

void GenerateSpheres::DisplaySpheres(const GenerateSpheres::Node& node, int level) const
{
	/*if (level == g_showSpheres)
	{
		D3DXMATRIXA16 m;
		D3DXMatrixTranslation(&m, node.sphere.position.X(),  node.sphere.position.Y(),  node.sphere.position.Z());
		Viewport3D::SetWorldTransformation_Full(m);

		DynamicVB::Begin(D3DPT_LINESTRIP);

		DynamicVB::SetVertexDataColor(D3DVSDE_DIFFUSE, (ColourARGB) Noisei((int) & node.sphere));

		float rs =  node.sphere.radius;

		for (int i = -3; i <= 3; i++)
		{
			float y = i * rs / 3;
			float rc = sqrt(rs * rs - y * y);

			for (int j = 0; j <= 10; j++)
			{
				float b = j * 2 * Pi / 10;
				DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, rc * sin(b), y, rc * cos(b), 1);
			}
		}

		DynamicVB::End();
	}

	for (int i = 0; i < node.subNodes.Size(); i++)
		DisplaySpheres(node.subNodes[i], level + 1);*/
}

}

