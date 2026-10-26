#ifndef RD_PRIMITIVE_H
#define RD_PRIMITIVE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "render.h"
#include "vertexbuffer.h"
#include "btl/containers/chainedhashmap.h"
#include "renderstates.h"

namespace RenderingData
{
	// This should be set up and passed to SceneBuild to add a primitive to a model.
	// After calling AddPrimitive() the object is no longer needed and can be discarded or reused.
	// All the members are public and can be changed as required.
	// Instances can also use this to add additional streams.
	class PrimitiveInfo
	{
	public:
		enum PrimitiveType
		{
			PRIMITIVE_TYPE_POINT,
			PRIMITIVE_TYPE_TRIANGLE,
			PRIMITIVE_TYPE_QUAD,
		};

		PrimitiveInfo() : primitiveType(PRIMITIVE_TYPE_TRIANGLE), batchID(-1) {}

		PrimitiveType primitiveType;

		// This must be the same on the base model and any instance streams,
		// and unique for each combination of materials.
		// See BatchIDGenerator below.
		S32 batchID;

		// For the base model only - ignored for instance streams.
		struct BaseInfo
		{
			BaseInfo() : effect(-1), sortCode(0), flags(0), sortType(SORT_TYPE_KEY), resourceBlockIndex(-1) {bounds.SetUniversal();}

			S32 resourceBlockIndex; // Block used to store the indices.
			S32 effect; // The index of the effect in the scene.
			U64 sortCode; // Used to sort the primitive within a render list.
			S32 flags; // User flags not used by rendering.
			SortType sortType; // Type of alpha sorting to use when rendering.
			BlendState blendState; // Blend states for the primitive.
			Bounds bounds; // Optional bounds information for the primitive.
		};

		BaseInfo base;

		struct StreamInfo
		{
			StreamInfo() : streamIndex(-1), materialIndex(-1), resourceBlockIndex(-1), instanceID(-1) {}

			S32 resourceBlockIndex; // Block used to store the vertices. Set to -1 if there are no vertices on the stream.
			S32 streamIndex; // The index of the vertex stream (not necessarily the index of the stream in the primitive).
			S32 materialIndex; // The index of the material in the scene.
			S32 instanceID; // Unique identifier for the current instance.
			VertexData vertices; // e.g. 3 vertices for a triangle. 
								 //	No vertices for material only.
								 // Vertex format for dynamic vertex data.
		};

		Btl::Vector<StreamInfo> streams;

		// If the vertex positions are standard F32_3 format, this will work out the bounds.
		void CalculateBoundsFromVertices();

		// Remap the UVs into the specified range.
		void RemapUVs(float maxUV);

		// Set the recommended sort code for the primitive.
		void GenerateSortCode( const U8 decalOrder = 0 );
		void GenerateSortCode( const U8 decalOrder, S32 iModelMtlIdx);

		struct TextureInfo
		{
			Util::C_String sUVSet; // Name of the uv set of the texture (optional)
			S32 textureIndex; // Index of the texture in the scene.
			S32 uvSet; // Original uv set of the texture (optional).
			float uScale, vScale; // Size in world space of one unit in texture space.
			float weight; // e.g. area of texture used by the triangle.
		};

		// Every texture used on the primitive should have an entry here 
		// so that the scale can be worked out for streaming.
		Btl::Vector<TextureInfo> textureInfo;

		// to do: helper functions for vertex compression
	};

	////////////////////////////////////////////////////////////////////////////////

	// Generate values suitable for use as a material combination index.
	class BatchIDGenerator 
	{
	public:
		BatchIDGenerator();

		// Generate a unique index for the materials used by the PrimitiveInfo object.
		// baseModelID = -1 for models that are not instanced.
		// PrimitiveInfo objects can be combined using 'previousIndex' (this only necessary if there are materials on the instance stream).
		S32 GetBatchID(const PrimitiveInfo& info, S32 baseModelID, S32 previousIndex = -1);

	private:
		struct BaseInfo
		{
			PrimitiveInfo::PrimitiveType primitiveType;
			S32 resourceBlockIndex;
			S32 effect;
			U64 sortCode;
			S32 flags;
			SortType sortType;
			BlendState blendState;
		};

		struct StreamInfo
		{
			S32 resourceBlockIndex;
			S32 streamIndex;
			S32 materialIndex;
			VertexFormat vertexFormat;
		};

		S32 GetBatchID(const BaseInfo& base, S32 baseModelID);
		S32 GetBatchID(const StreamInfo& stream, S32 previousIndex);

		friend bool operator == (const BaseInfo& t1, const BaseInfo& t2);
		friend bool operator == (const StreamInfo& t1, const StreamInfo& t2);
		friend U32 Hash(const BaseInfo& t);
		friend U32 Hash(const StreamInfo& t);

		typedef Btl::ChainedHashMap<Btl::Pair<BaseInfo, S32>, S32> BaseMap;
		typedef Btl::ChainedHashMap<Btl::Pair<StreamInfo, S32>, S32> StreamMap;
		BaseMap m_baseMap;
		StreamMap m_streamMap;
		S32 m_uniqueIndexCount;
	};
}

#endif
