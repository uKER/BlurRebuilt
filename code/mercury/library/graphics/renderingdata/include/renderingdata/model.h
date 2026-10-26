#ifndef RD_MODEL_H
#define RD_MODEL_H
#ifdef _MSC_VER
#pragma once
#endif

#include "effect.h"
#include "render.h"
#include "renderingdata/primitive.h"

// Nothing in here will reach the game, as render objects are all converted to render lists.

namespace RenderingData {
namespace Build
{
	class Scene;

	// Primitive data, without material or vertex streams.
	struct Primitive
	{
		Primitive() {bounds.SetEmpty();}

		S32 effect;
		S32 vertexDeclaration;
		U64 sortCode;
		S32 flags;
		S32 batchID;
		SortType sortType;
		PrimitiveType primitiveType;
		ResourceIndex indexBufferIndex;
		BlendState blendState;
		Bounds bounds; // Set when adding polygons to the primitive.

		struct Lod
		{
			S32 offset;
			S32 nIndices;
		};

		Btl::Vector<Lod> lods; // automatically generated lods
	};

	// Material and vertex data for a stream.
	struct PrimitiveStream
	{
		PrimitiveStream() : active(false) {}

		S32 materialIndex;
		ResourceIndex vertexBufferIndex;
		S32 streamIndex;
		S32 vertexOffset;
		S32 frequency;
		bool active;
	};

	// Stream data for a model.
	class ModelStream
	{
	public:
		ModelStream();

		friend class Build::Scene;

		S32 GetNumPrimitives() const;
		const PrimitiveStream& GetPrimitive(S32 i) const;

	private:
		void InsertPrimitive(S32 primitiveIndex, S32 materialIndex, ResourceIndex vertexBufferIndex, S32 streamIndex);

		Btl::Vector<PrimitiveStream> m_primitives;
	};

	// List of primitives and stream data.
	class Model : public RenderObject
	{
	public:
		Model();

		virtual RenderObjectType GetObjectType() const;
		virtual Model* Clone() const;

		S32 GetNumPrimitives() const;
		const Primitive& GetPrimitive(S32 i) const;

		S32 GetNumStreams() const;
		const ModelStream& GetModelStream(S32 i) const;

		// Base material is present whether a model or instance.
		S32 GetBaseMaterialIndex() const;
		void SetBaseMaterialIndex(S32 i);

		// Replaced by the instance material if the model is instanced.
		S32 GetInstanceMaterialIndex() const;
		void SetInstanceMaterialIndex(S32 i);

		// built by adding primitives in Build::Scene
		friend class Build::Scene;

		// If p is a Model, return a pointer to it, otherwise return a null pointer.
		static const Model* GetPointer(const RenderObject* p);
		static Model* GetPointer(RenderObject* p);

	private:
		S32 InsertPrimitive(S32 batchID, PrimitiveType primitiveType, const PrimitiveInfo::BaseInfo& base);
		S32 FindPrimitive(S32 batchID);

		Btl::Vector<Primitive> m_primitives;
		Btl::Vector<ModelStream> m_streams;
		S32 m_baseMaterialIndex;
		S32 m_instanceMaterialIndex;
	};

	// Instance of a model (with optional stream data).
	class Instance : public RenderObject
	{
	public:
		Instance();

		virtual RenderObjectType GetObjectType() const;
		virtual Instance* Clone() const;

		S32 GetBaseModelIndex() const;
		S32 GetMaterialIndex() const;
		S32 GetNumStreams() const;
		const ModelStream& GetModelStream(S32 i) const;
		U32 GetFlags() const;

		// Build
		void SetBaseModelIndex(S32 i);
		void SetMaterialIndex(S32 i);
		void SetFlags(U32 flags);

		// primitive streams added in Build::Scene
		friend class Build::Scene;

	private:
		Btl::Vector<ModelStream> m_streams;
		S32 m_materialIndex;
		S32 m_baseModelIndex;
		U32 m_flags;
	};

	// A number of instances of the same model.
	class InstanceBatch : public RenderObject
	{
	public:
		InstanceBatch();

		virtual RenderObjectType GetObjectType() const;
		virtual InstanceBatch* Clone() const;

		friend class Build::Scene;

	private:
		Instance m_instance;
		PrimitiveStream m_stream;
	};

	#include "noninterface/model.inl"
}}

#endif
