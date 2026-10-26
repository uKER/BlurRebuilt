#include "stdafx.h"
#include "converter_shared/pvs/pvs.h"
#include "converter_shared/pvs/simplecollision.h"
#include "btl/utility/hash.h"
#include "btl/containers/chainedhashmap.h"
#include <core/maths\hvec\New\IncludeMinimal.h>
#include <core/maths\hvec\New\IncludeMinimal.inl>
#include <System/misc/Random.h>

//#pragma optimize("", off)

namespace Pvs {
namespace Build
{

PvsPolyInfo::PvsPolyInfo()
{
	m_generateAtCentroid = false;
	m_generateOnGrid = true;
	m_gridSizeXZ = 4.f;
	m_nSamplePointsY = 1;
	m_minHeight = 1.f;
	m_maxHeight = 1.f;
}

bool PvsPolyInfo::GeneratePvs(const Aftk::C_Polygon& p) const
{
	return true;
}

bool PvsPolyInfo::GenerateAtCentroid(const Aftk::C_Polygon& p) const
{
	return m_generateAtCentroid;
}

bool PvsPolyInfo::GenerateOnGrid(const Aftk::C_Polygon& p) const
{
	return m_generateOnGrid;
}

float PvsPolyInfo::GridSizeXZ(const Aftk::C_Polygon& p) const
{
	return m_gridSizeXZ;
}

S32 PvsPolyInfo::NumSamplePointsY(const Aftk::C_Polygon& p) const
{
	return m_nSamplePointsY;
}

float PvsPolyInfo::HeightAboveTriangle(const Aftk::C_Polygon& p, S32 n) const
{
	return m_minHeight + (m_maxHeight * n) / m_nSamplePointsY;
}

void PvsPolyInfo::SetGenerateOnGrid(bool b)
{
	m_generateOnGrid = b;
}

void PvsPolyInfo::SetGenerateAtCentroid(bool b)
{
	m_generateAtCentroid = b;
}

void PvsPolyInfo::SetGridSizeXZ(float size)
{
	m_gridSizeXZ = size;
}

void PvsPolyInfo::SetNumSamplePointsY(S32 n)
{
	m_nSamplePointsY = n;
}

void PvsPolyInfo::SetHeightAboveTriangle(float min, float max)
{
	m_minHeight = min;
	m_maxHeight = max;
}


/////////////////////////////////////////////////////////////////////////////////

PvsData::PvsData()
{
	m_min = Point3(FLT_MAX, FLT_MAX, FLT_MAX);
	m_max = Point3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	m_zoneSizeXZ = 32.f;
	m_zoneSizeY = 32.f;
	
	m_maxSamplePoints = 4000000; //-1;
}

void PvsData::SetZoneSize(float xz, float y)
{
	m_zoneSizeXZ = xz;
	m_zoneSizeY = y;
}

int AFTKGetPositionIndex(int v, const Aftk::C_Polygon& aftkPolygon, const Aftk::C_SceneMeshNode& aftkSceneMeshNode)
{
	const Aftk::C_PolygonVertex& aftkPolygonVertex = aftkPolygon.GetPolygonVertex(v);
	return aftkPolygonVertex.GetPositionIndex();
}

Aftk::Position3 AFTKGetTransformedPosition(int v, const Aftk::C_Polygon& aftkPolygon, const Aftk::C_SceneMeshNode& aftkSceneMeshNode, const Aftk::Matrix44& matrix)
{
	const Aftk::Position3& p = aftkSceneMeshNode.GetVertexPosition(AFTKGetPositionIndex(v, aftkPolygon, aftkSceneMeshNode));
	Aftk::Vector4F32 v1(p.x, p.y, p.z, 1.f);
	Aftk::Position3 pos = MakePoint(v1 * matrix);
	return pos;
}

typedef UtMaths::MPoint<UtMaths::VectorXYZ<float> > Point3;

FORCE_INLINE const Maths::C_Vertex ConvertHVec(const Point3& src) {return Maths::C_Vertex(src.X(), src.Y(), src.Z());}
FORCE_INLINE const Point3 ConvertHVec(const Maths::C_Vertex& src) {return Point3(src.X(), src.Y(), src.Z());}

void PvsData::AddMesh(const Aftk::C_SceneMeshNode& mesh, const Aftk::Matrix44& matrix, const PvsPolyInfo& polyInfo)
{
	int numPolys = mesh.GetNumPolygons(0);

	for (int i = 0; i < numPolys; i++)
	{
		// source polygon
		const Aftk::C_Polygon& aftkPolygon = mesh.GetPolygon(i);

		if (!polyInfo.GeneratePvs(aftkPolygon))
			continue;

		// generate points on the surface of the polygon
		Aftk::Position3 min, max;
		Set(&min, FLT_MAX, FLT_MAX, FLT_MAX);
		Set(&max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

		Aftk::Position3 vertices[3];

		vertices[0] = AFTKGetTransformedPosition(0, aftkPolygon, mesh, matrix);
		vertices[1] = AFTKGetTransformedPosition(1, aftkPolygon, mesh, matrix);
		vertices[2] = AFTKGetTransformedPosition(2, aftkPolygon, mesh, matrix);

		UpdateBounds(&min, &max, vertices[0]);
		UpdateBounds(&min, &max, vertices[1]);
		UpdateBounds(&min, &max, vertices[2]);

		SimpleCollision::Triangle triangle(ConvertHVec(vertices[1]), ConvertHVec(vertices[0]), ConvertHVec(vertices[2]));

		if (triangle.normal.Y() < 0.1f)
			continue;

		Maths::C_Normal negativeY(Maths::Constants<Maths::HVec3>::UnitAxisNegativeY());

		Maths::HVec3 centroid(	(vertices[0].X() + vertices[1].X() + vertices[2].X()) / 3.f,
								(vertices[0].Z() + vertices[1].Z() + vertices[2].Z()) / 3.f,
								(vertices[0].Y() + vertices[1].Y() + vertices[2].Y()) / 3.f);

		if (polyInfo.GenerateOnGrid(aftkPolygon))
		{
			// Use double the grid density but only include the extra points 
			// if they are not surrounded by other points.
			float gridSize = polyInfo.GridSizeXZ(aftkPolygon);
			float separation = polyInfo.GridSizeXZ(aftkPolygon) * 0.5f;
			float maxHeight = polyInfo.HeightAboveTriangle(aftkPolygon, polyInfo.NumSamplePointsY(aftkPolygon) - 1);

			for (float fi = ceil(min.x / separation) * separation; fi <= floor(max.x / separation) * separation; fi += separation)
			{
				for (float fk = ceil(min.z / separation) * separation; fk <= floor(max.z / separation) * separation; fk += separation)
				{
					Maths::HVec3 p(fi, max.Y() + maxHeight, fk);

					// Move 2cm towards the centre.
					{
						Maths::HVec3 v = (centroid - p);
						float d = v.Length();

						if (d > 0.02f)
							p += v / (d * 50.f);
					}

					SimpleCollision::RayCast rayCast(p, negativeY, max.Y() - min.Y() + maxHeight + 1.f);

					// Get intersection with triangle.
					SimpleCollision::Contact contact;
					contact.distance = rayCast.maxDistance;
					SimpleCollision::GetContact(rayCast, triangle, contact);

					if (contact.collision)
					{
						bool edgeX = Maths::Abs(fi - floor(fi / gridSize + 0.5f) * gridSize) > 0.001f;
						bool edgeZ = Maths::Abs(fk - floor(fk / gridSize + 0.5f) * gridSize) > 0.001f;
						AddSamplePoints(aftkPolygon, polyInfo, fi, fk, contact.position.Y(), false, edgeX || edgeZ);
					}
				}
			}
		}

		if (polyInfo.GenerateAtCentroid(aftkPolygon))
		{
			AddSamplePoints(aftkPolygon, polyInfo, centroid.X(), centroid.Y(), centroid.Z(), false, true);
		}
	}
}

bool PvsData::CanAddSamplePoint()
{
	if(m_maxSamplePoints < 0)
		return true;

	if(m_samplePoints.Size() < m_maxSamplePoints)
		return true;

	static bool printedWarning = false;
	if(!printedWarning)
	{
		LOGERROR( Tools::g_log, "Maximum number of PVS sample points (%d) reached. Level will have PVS errors!", m_maxSamplePoints);
		printedWarning = true;
	}

	return false;
}

void PvsData::AddSamplePoints(const Aftk::C_Polygon& aftkPolygon, const PvsPolyInfo& polyInfo, float x, float z, float base, bool directional, bool edge)
{
	for (int j = 0; j < polyInfo.NumSamplePointsY(aftkPolygon); j++)
	{
		float y = base + polyInfo.HeightAboveTriangle(aftkPolygon, j);
		if( CanAddSamplePoint() )
		{
			SamplePoint s;
			s.position = Point3(x, y, z);
			s.directional = directional;
			s.direction = Maths::Constants<Normal3>::Y();
			s.optional = edge;
			s.gridSize = polyInfo.GridSizeXZ(aftkPolygon);
			m_samplePoints.PushBack(s);

			m_min = Min(m_min, s.position);
			m_max = Max(m_max, s.position);

		}
	}
}

void PvsData::AddZone(const MathsStorage::Matrix44& matrix)
{
	m_zones.PushBack(matrix);
}

void PvsData::AddSamplePoint(const Point3& position, const Normal3& direction, bool directional)
{
	if( !CanAddSamplePoint() )
		return;

	SamplePoint s;
	s.position = position;
	s.direction = direction;
	s.directional = directional;
	s.optional = false;
	s.gridSize = 0.f;
	m_samplePoints.PushBack(s);

	m_min = Min(m_min, s.position);
	m_max = Max(m_max, s.position);
}

void PvsData::CreateCellsYSplitSub(const RenderDataConversion::Clustering::PointCloud::Node& node)
{
	if (node.GetNumPoints() > 0)
	{
		// Get new bounds from the points.
		Point3 min(node.GetMin());
		Point3 max(node.GetMax());
		min.y = FLT_MAX;
		max.y = -FLT_MAX;

		for (int i = 0; i < node.GetNumPoints(); i++)
		{
			min.y = Maths::Min(min.y, node.GetPoint(i).position.y - 2.f);
			max.y = Maths::Max(max.y, node.GetPoint(i).position.y + 2.f);
		}

		// Make Y bounds power of two.
		{
			float size =  max.y - min.y;
			float log2 = log(2.f);
			float f = log(size / m_zoneSizeY) / log2;
			float nextSize = exp(ceil(f) * log2) * m_zoneSizeY;
			float d = (nextSize - size);
			max.y += d;
		}

		// Split along y.
		RenderDataConversion::Clustering::PointCloud pointCloud;

		pointCloud.Initialise(min, max, RenderDataConversion::Clustering::Vector3(m_zoneSizeXZ, m_zoneSizeY + 0.01f, m_zoneSizeXZ));

		for (int i = 0; i < node.GetNumPoints(); i++)
			pointCloud.Add(node.GetPoint(i).position, node.GetPoint(i).id);

		// Create cells.
		if (pointCloud.GetRootPtr())
			CreateCells(*pointCloud.GetRootPtr());

		return;

		CreateCell(node);
	}
}

void PvsData::CreateCell(const RenderDataConversion::Clustering::PointCloud::Node& node)
{
	if (node.GetNumPoints() > 0)
	{
		//Point3 min(FLT_MAX, FLT_MAX, FLT_MAX);
		//Point3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		//for (int i = 0; i < node.GetNumPoints(); i++)
		//	UpdateBounds(&min, &max, node.GetPoint(i).position);

		Point3 centre = Lerp(node.GetMax(), node.GetMin(), 0.5f);
		Vector3 size = (node.GetMax() - node.GetMin()) * 0.5f;// + Vector3(1.f, 1.f, 1.f);

		m_cells.PushBack();
		m_inverseMatrices.PushBack();

		MathsStorage::Matrix44 matrix = {	size.X(), 0.f, 0.f, 0.f,
											0.f, size.Y(), 0.f, 0.f,
											0.f, 0.f, size.Z(), 0.f,
											centre.X(), centre.Y(), centre.Z(), 1.f};

		m_cells.Back().matrix = matrix;

		{
			Maths::Mat44 mat44 = Maths::Load<Maths::HMat44, MathsStorage::Matrix44>(&matrix);
			Maths::Mat44 imat44 = Maths::Inverse(mat44);
			Maths::Store(imat44, &m_inverseMatrices.Back());
		}

		RenderDataConversion::Clustering::PointCloud cellPoints;
		cellPoints.Initialise(node.GetMin(), node.GetMax(), 4.f);

		for (int i = 0; i < node.GetNumPoints(); i++)
		{
			if (!m_samplePoints[node.GetPoint(i).id].optional)
			{
				cellPoints.Add(node.GetPoint(i).position, 0);
			}
		}

		for (int i = 0; i < node.GetNumPoints(); i++)
		{
			bool add = true;

			if (m_samplePoints[node.GetPoint(i).id].optional)
			{
				Point3 p(node.GetPoint(i).position);

				float gridSize = m_samplePoints[node.GetPoint(i).id].gridSize * 0.5f;

				Vector3 s1(1.f, 1.f, 1.f);
				Vector3 s2(gridSize + 0.1f, gridSize + 0.1f, gridSize + 0.1f);
				Vector3 vx(gridSize + 0.2f, 0.f, 0.f);
				Vector3 vz(0.f, 0.f, gridSize + 0.2f);

				// Look for points around this point.
				{
					S32 count = 0;
					count += cellPoints.Count(p + vx - s2, p + vx + s2) > 0;
					count += cellPoints.Count(p - vx - s2, p - vx + s2) > 0;
					count += cellPoints.Count(p + vz - s2, p + vz + s2) > 0;
					count += cellPoints.Count(p - vz - s2, p - vz + s2) > 0;

					if (count == 4)
						add = false;
				}

				// Look for points close to this one.
				if (cellPoints.Count(p - s1, p + s1))
					add = false;
				
				if (add)
					cellPoints.Add(node.GetPoint(i).position, 0);
			}

			if (add)
			{
				m_cells.Back().samplePoints.PushBack();
				SamplePoint& samplePoint(m_cells.Back().samplePoints.Back());
				samplePoint.directional = false;
				samplePoint.position = node.GetPoint(i).position;
				samplePoint.gridSize = m_samplePoints[node.GetPoint(i).id].gridSize;
				samplePoint.optional = m_samplePoints[node.GetPoint(i).id].optional;
			}
		}
	}
}

void PvsData::CreateCellsYSplit(const RenderDataConversion::Clustering::PointCloud::Node& node)
{
	CreateCellsYSplitSub(node);

	for (int i = 0; i < node.GetNumSubNodes(); i++)
		CreateCellsYSplit(node.GetSubNode(i));
}

void PvsData::CreateCells(const RenderDataConversion::Clustering::PointCloud::Node& node)
{
	CreateCell(node);

	for (int i = 0; i < node.GetNumSubNodes(); i++)
		CreateCells(node.GetSubNode(i));
}


void PvsData::Process()
{
	// Add zones.
	for (int zone = 0; zone < m_zones.Size(); zone++)
	{
		m_cells.PushBack();
		m_inverseMatrices.PushBack();
		m_cells.Back().matrix = m_zones[zone];
		Maths::Mat44 mat44 = Maths::Load<Maths::HMat44, MathsStorage::Matrix44>(&m_cells.Back().matrix);
		Maths::Mat44 imat44 = Maths::Inverse(mat44);
		Maths::Store(imat44, &m_inverseMatrices.Back());
	}

	// Add sample points to tree.
	RenderDataConversion::Clustering::PointCloud pointCloud;

	// Make XZ bounds power of two.
	for (int i = 0; i < 3; i += 2)
	{
		m_min[i] = floor(m_min[i] / m_zoneSizeXZ) * m_zoneSizeXZ - 1.f;
		m_max[i] = ceil((m_max[i] + 1.f) / m_zoneSizeXZ) * m_zoneSizeXZ - 1.f;
		float size =  m_max[i] - m_min[i];
		float log2 = log(2.f);
		float f = log(size / m_zoneSizeXZ) / log2;
		float nextSize = exp(ceil(f) * log2) * m_zoneSizeXZ;
		float d = (nextSize - size) * 0.5f;
		m_max[i] += d;
		m_min[i] -= d;
	}

	float height = m_max.Y() - m_min.Y();
	pointCloud.Initialise(m_min, m_max, RenderDataConversion::Clustering::Vector3(m_zoneSizeXZ, height, m_zoneSizeXZ));

	for (int i = 0; i < m_samplePoints.Size(); i++)
	{
		// See if the point is inside any zones.
		bool inZone = false;

		Maths::Vector4 v = Maths::MakeVector4(m_samplePoints[i].position.X(), m_samplePoints[i].position.Y(), m_samplePoints[i].position.Z(), 1.f);
		Maths::VF32 one = Maths::MakeVF32(1.f);

		for (int zone = 0; zone < m_zones.Size(); zone++)
		{
			Maths::Mat44 imat44 = Maths::Load<Maths::HMat44, MathsStorage::Matrix44>(&m_inverseMatrices[zone]);
			Maths::Vector4 v2 = v * imat44;

			if ((Abs(X(v2)) < one) & (Abs(Y(v2)) < one) & (Abs(Z(v2)) < one))
			{
				inZone = true;
				m_cells[zone].samplePoints.PushBack();
				SamplePoint& samplePoint(m_cells[zone].samplePoints.Back());
				samplePoint = m_samplePoints[i];
			}
		}

		if (!inZone)
			pointCloud.Add(m_samplePoints[i].position, i);
	}

	// Create cells.
	if (pointCloud.GetRootPtr())
		CreateCellsYSplit(*pointCloud.GetRootPtr());
}

ModelSamplePointSetup::ModelSamplePointSetup()
: nLongSegments(16)
, nLatSegments(8)
, nDistanceSegments(4)
, nSamplePoints(2000)
, fMinRadius(5.f)
, fMaxRadius(250.f)
{}

// Generate cells/samplepoints in a sphere around a model
void PvsData::GenerateModelSamplePoints(ModelSamplePointSetup &setup)
{
	Util::C_Random rnd;

	m_cells.Resize(setup.nLongSegments * setup.nLatSegments * setup.nDistanceSegments);

	for (int i=0; i<setup.nSamplePoints; i++)
	{
		Maths::HVec3 v = rnd.SphereRand();

		// classify v to a particular segment
		float fLong = (float)(atan2(v.Z(), v.X()) / M_PI);
		float fLat = v.Y();

		int iLong = (int)(((fLong + 1.f) * 0.5f) * (F32)setup.nLongSegments);
		int iLat = (int)(((fLat + 1.f) * 0.5f) * (F32)setup.nLatSegments);

		assert(iLong>=0 && iLong<setup.nLongSegments);
		assert(iLat>=0 && iLat<setup.nLatSegments);

		for (int iDist=0; iDist<setup.nDistanceSegments; iDist++)
		{
			float f1 = 1.f/(float)setup.fMinRadius;
			float f2 = 1.f/(float)setup.fMaxRadius;
			float fx = f1 + (f2-f1) * (float)iDist/(float)(setup.nDistanceSegments-1);
			float fRadius = 1.f/fx;

			int iCell = iLong + iLat*setup.nLongSegments + iDist*setup.nLongSegments*setup.nLatSegments;

			SamplePoint item;
			item.direction.x = -v.X(), item.direction.y = -v.Y(), item.direction.z = -v.Z();
			item.position.x = v.X() * fRadius, item.position.y = v.Y() * fRadius, item.position.z = v.Z() * fRadius;
			item.directional = true;
			item.optional = false;
			m_cells[iCell].samplePoints.PushBack(item);
		}
	}	

	for (int i=0; i<m_cells.Size(); i++)
	{
		assert(m_cells[i].samplePoints.Size() > 0);
	}
}

void PvsData::WriteDataForPvsTool(const char* filename) const
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(filename, Sys::E_FileAccess_Write);

	if (pFile)
	{
		const S32 PVS_SAMPLE_FILE_VERSION = 1;
		const S32 PVS_TYPE_POSITION = 1;
		const S32 PVS_TYPE_ORIENTATION = 2;

		Sys::BinarySerialiser<Ut::NoEndianSwap> stream(*pFile);

		stream << 0 << PVS_SAMPLE_FILE_VERSION;
		stream << m_cells.Size();

		for (int i = 0; i < m_cells.Size(); i++)
		{
			stream << m_cells[i].samplePoints.Size();

			for (int j = 0; j < m_cells[i].samplePoints.Size(); j++)
			{
				const SamplePoint& p(m_cells[i].samplePoints[j]);

				if (p.directional)
					stream << PVS_TYPE_ORIENTATION;
				else
					stream << PVS_TYPE_POSITION;

				stream << p.position.X() << p.position.Y() << p.position.Z();
				stream << p.direction.X() << p.direction.Y() << p.direction.Z();
			}
		}

		delete pFile;
	}
}

//////////////////////////////////////////////////////////

void PvsData::ReadDataFromPvsTool(const char* filename)
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(filename, Sys::E_FileAccess_Read);

	if (pFile)
	{
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> stream(*pFile);

		stream >> m_pvsOutputData;

		delete pFile;
	}
}

S32 PvsData::GetNumPvsCells() const
{
	return m_cells.Size();
}

S32 PvsData::GetPvsIndex(int nodeIndex, int polygonIndex, const char* pDebugName) const
{
	return m_pvsOutputData.GetPvsIndex(nodeIndex, polygonIndex, pDebugName);
}

// return reference to array of PVS primitives
const Btl::Vector<DatabaseFilePrimitive> &PvsData::GetPvsPrimitives()
{
	return m_pvsOutputData.primitives;
}

const Btl::BitVector& PvsData::GetPvsBits(S32 pvsIndex) const
{
	return m_pvsOutputData.primitiveData[pvsIndex];
}

Btl::BitVector& PvsData::GetPvsBits(S32 pvsIndex)
{
	return m_pvsOutputData.primitiveData[pvsIndex];
}

////////////////////////////////////////////////////////////


}}
