#ifndef CONVERTER_SHARED_PVS
#define CONVERTER_SHARED_PVS

#ifdef _MSC_VER
#pragma once
#endif

#include "world_render/pvs.h"
#include "AFTK/aftk.h"
#include "btl/containers/bitvector.h"
#include "converter_shared/renderdataconversion/pointcloud.h"
#include "pvsoutput.h"

namespace Pvs {
namespace Build
{
	struct ModelSamplePointSetup
	{
		ModelSamplePointSetup();

		int nLongSegments;
		int nLatSegments;
		int nDistanceSegments;
		int nSamplePoints;
		float fMinRadius;
		float fMaxRadius;
	};

	class PvsPolyInfo
	{
	public:
		PvsPolyInfo();

		virtual bool GeneratePvs(const Aftk::C_Polygon& p) const;
		virtual bool GenerateAtCentroid(const Aftk::C_Polygon& p) const;
		virtual bool GenerateOnGrid(const Aftk::C_Polygon& p) const;
		virtual float GridSizeXZ(const Aftk::C_Polygon& p) const;
		virtual S32 NumSamplePointsY(const Aftk::C_Polygon& p) const;
		virtual float HeightAboveTriangle(const Aftk::C_Polygon& p, S32 n) const;

		void SetGenerateOnGrid(bool b);
		void SetGenerateAtCentroid(bool b);
		void SetGridSizeXZ(float size);
		void SetNumSamplePointsY(S32 n);
		void SetHeightAboveTriangle(float min, float max);

	private:
		bool m_generateAtCentroid;
		bool m_generateOnGrid;
		float m_gridSizeXZ;
		S32 m_nSamplePointsY;
		float m_minHeight;
		float m_maxHeight;
	};

	// Build Pvs data in the converter
	class PvsData : public Pvs::PvsData
	{
	public:
		PvsData();

		// Generate sample points over the mesh polygons.
		void AddMesh(const Aftk::C_SceneMeshNode& mesh, const Aftk::Matrix44& matrix, const PvsPolyInfo& polyInfo);

		// Generate cells/samplepoints in a sphere around a model
		void GenerateModelSamplePoints(ModelSamplePointSetup &setup);

		// Unit cube scaled by a matrix.
		void AddZone(const MathsStorage::Matrix44& matrix);
		
		// Add a single sample point.
		void AddSamplePoint(const Point3& position, const Normal3& direction, bool directional);

		// Set the size of automatically generated zones.
		void SetZoneSize(float xz, float y);
		
		// Call after adding all sample points and zones.
		void Process();

		// Write out cells and sample points for the Pvs tool to process.
		void WriteDataForPvsTool(const char* filename) const;

		// Read bitfields from the Pvs tool.
		void ReadDataFromPvsTool(const char* filename);

		// Number of Pvs cells.
		S32 GetNumPvsCells() const;

		// Get index for polygon / instance.
		S32 GetPvsIndex(int nodeIndex, int polygonIndex, const char* pDebugName) const;

		// Pvs data for a polygon / instance (1 bit per Pvs cell).
		const Btl::BitVector& GetPvsBits(S32 pvsIndex) const;

		// Pvs data for a polygon / instance (1 bit per Pvs cell).
		Btl::BitVector& PvsData::GetPvsBits(S32 pvsIndex);

		void SetMaxSamplePoints(S32 maxSamplePoints);

		// return reference to array of PVS primitives
		const Btl::Vector<DatabaseFilePrimitive> &GetPvsPrimitives();

	private:
		void CreateCell(const RenderDataConversion::Clustering::PointCloud::Node& node);
		void CreateCells(const RenderDataConversion::Clustering::PointCloud::Node& node);
		void CreateCellsYSplit(const RenderDataConversion::Clustering::PointCloud::Node& node);
		void CreateCellsYSplitSub(const RenderDataConversion::Clustering::PointCloud::Node& node);
		void AddSamplePoints(const Aftk::C_Polygon& aftkPolygon, const PvsPolyInfo& polyInfo, float x, float z, float base, bool directional, bool edge);
		bool CanAddSamplePoint();

		Btl::Vector<SamplePoint> m_samplePoints;
		Btl::Vector<MathsStorage::Matrix44> m_zones;

		Point3 m_min;
		Point3 m_max;
		float m_zoneSizeXZ;
		float m_zoneSizeY;

		S32 m_maxSamplePoints;

		PvsOutputData m_pvsOutputData;
	};

	// Write out data for the game.
	template <typename Archive>
	inline void Serialise(Archive& ar, PvsData& t)
	{
		SERIALISE_BASE_FREE(Pvs::PvsData, ar, t);
	}

	AUTO_DESERIALISE_FREE(PvsData)
}}

#endif