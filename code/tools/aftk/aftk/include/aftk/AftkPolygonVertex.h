#ifndef AFTK_POLYGON_VERTEX_H
#define AFTK_POLYGON_VERTEX_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_BlindData;

	class C_PolygonVertex
	{
	public:
		
		///\brief Constructor.
		///
		C_PolygonVertex();

		///\brief Destructor.
		~C_PolygonVertex();

		///\brief Copy constructor.
		///
		///@param rhs C_Polygon vertex to copy.
		C_PolygonVertex(const C_PolygonVertex& rhs);

		///\brief Asignment operator.
		///
		///@param rhs C_Polygon vertex to copy.
		C_PolygonVertex& operator=(const C_PolygonVertex& rhs);

		///\brief Tests this polygon vertex against another to determine if they are equal.
		///
		///@param rhs The polygon vertex to test against.
		///@param epsilon The distance threshold to test against.
		bool EqualityTest(const C_PolygonVertex& rhs, F32 epsilon = AFTK_EPSILON) const;

		///\brief Tests this polygon vertex against another to determine if it is less than the rhs.
		///
		///@param rhs The polygon vertex to test against.
		///@param epsilon The distance threshold to test against.
		bool LessThanTest(const C_PolygonVertex& rhs, F32 epsilon = AFTK_EPSILON) const;
		
		///\brief Set the position index for this polygon vertex.
		///
		///An index of -1 indicates that no data has been asigned.
		///
		///@param index Position index in the array of mesh vertex positions.
		void SetPositionIndex(S32 index);

		///\brief Set the normal index for this polygon vertex.
		///
		///An index of -1 indicates that no data has been asigned.
		///
		///@param index Normal index in the array of mesh vertex normals.
		void SetNormalIndex(S32 index);

		///\brief Set the value of a texture coordinate index in the given set.
		///
		///An texture coordinate index of -1 indicates that no data has been asigned.
		///
		///@param set The texture coordinate set to alter.
		///@param index Position index in the array of mesh vertex positions.
		void SetTextureCoordinateIndex(U32 set, S32 index);

		///\brief Set the value of the texture indices in the given set.
		///
		///Indices of -1 indicate that no data has been asigned.
		///
		///@param set The texture coordinate set to alter.
		///@param textureCoordinateIndex The texture coordinate index.
		///@param tangentIndex The tangent index.
		///@param binormalIndex The binormal index.
		void SetTextureCoordinateSetIndices(
			U32 set,
			S32 textureCoordinateIndex, 
			S32 tangentIndex, 
			S32 binormalIndex);

		///\brief Sets the tangent index.
		///
		///Tangent indices are automatically set by the mesh's 'CalculateTangentsAndBinormals' function.
		///@param set The texture coordinate set for this tangent index.
		///@param index The tangent index.
		void SetTangentIndex(U32 set, S32 index);

		///\brief Sets the binormal index.
		///
		///Binormal indices are automatically set by the mesh's 'CalculateTangentsAndBinormals' function.
		///@param set The texture coordinate set for this binormal index.
		///@param index The binormal index.
		void SetBinormalIndex(U32 set, S32 index);

		void DeleteTextureCoordinateIndices();

		///\brief Deletes the texture coordinate indices for a given UV set.
		///
		///This will also cause the tangent and binormal indices for this set to be erased.
		void DeleteTextureCoordinateIndices(U32 set);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.	
		void SetData(U32 channel, U8 data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.	
		void SetData(U32 channel, U16 data);
		
		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.	
		void SetData(U32 channel, U32 data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.	
		void SetData(U32 channel, U64 data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.	
		void SetData(U32 channel, S8 data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.	
		void SetData(U32 channel, S16 data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.	
		void SetData(U32 channel, S32 data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, S64 data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, F32 data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, F64 data);
		
		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2U8& data);
		
		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2U16& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2U32& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2U64& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2S8& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2S16& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2S32& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2S64& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2F32& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector2F64& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3U8& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3U16& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3U32& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3U64& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3S8& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3S16& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3S32& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3S64& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.

		void SetData(U32 channel, const Vector3F32& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector3F64& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.

		void SetData(U32 channel, const Vector4U8& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.

		void SetData(U32 channel, const Vector4U16& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.

		void SetData(U32 channel, const Vector4U32& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.

		void SetData(U32 channel, const Vector4U64& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector4S8& data);

		///\brief Sets the value of a given data channel.
		///
		///@param channel The channel to modify.
		///@param data The data value.
		void SetData(U32 channel, const Vector4S16& data);

		///\brief Adds data into a vertex channel.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@return The index at which the data was added in the channel array.
		void SetData(U32 channel, const Vector4S32& data);

		///\brief Adds data into a vertex channel.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@return The index at which the data was added in the channel array.
		void SetData(U32 channel, const Vector4S64& data);

		///\brief Adds data into a vertex channel.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@return The index at which the data was added in the channel array.
		void SetData(U32 channel, const Vector4F32& data);

		///\brief Adds data into a vertex channel.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@return The index at which the data was added in the channel array.
		void SetData(U32 channel, const Vector4F64& data);

		///\brief Adds data into a vertex channel.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@return The index at which the data was added in the channel array.
		void SetData(U32 channel, const Matrix44& data);

		///\brief Adds data into a vertex channel.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@return The index at which the data was added in the channel array.
		void SetData(U32 channel, bool data);

		///\brief Returns the vertex position index.
		///
		///Note that the return value is signed, with -1 marking that no data has yet been
		///assigned.
		///
		///@return Vertex position index.
		S32	GetPositionIndex() const;

		///\brief Returns the vertex normal index.
		///
		///Note that the return value is signed, with -1 marking that no data has yet been
		///assigned.
		///
		///@return Vertex normal index.
		S32	GetNormalIndex() const;

		///\brief Returns the vertex tangent index.
		///
		///Note that the return value is signed, with -1 marking that no tangent space data has
		///been created for this polygon vertex.
		///@return Vertex tangent index.
		S32 GetTangentIndex(U32 set) const;

		///\brief Returns the vertex binormal index.
		///
		///Note that the return value is signed, with -1 marking that no tangent space data has
		///been created for this polygon vertex.
		///@return Vertex binormal index.
		S32 GetBinormalIndex(U32 set) const;

		///\brief Returns the texture coordinate index for the given set.
		///
		///Note that the return value is signed, with -1 marking that no data has yet been
		///assigned.
		///
		///@param set Texture coordinate set to query.
		///@return texture coordinate position index.
		S32	GetTextureCoordinateIndex(U32 set = 0) const;

		///\brief Returns the data stored in the given channel.
		///
		///@param channel Data instance to query.
		///@param data value which is returned.
		template <class T>
		void GetData(T& data, U32 channel) const
		{
			data = *((T*)GetDataPtr(channel));
		}

		///\brief Returns a pointer to the specified vertex data instance.
		///
		///@param channel Data instance to query.
		///@return data pointer;
		void* GetDataPtr(U32 channel);

		///\brief Returns a pointer to the specified vertex data instance.
		///
		///@param channel Data instance to query.
		///@return data pointer;
		const void* GetDataPtr(U32 channel) const;

		///\brief Returns the number of texture coordinate indices.
		///
		///@return The number of texture coordinate indices.
		U32 GetNumTextureCoordinateIndices() const;
		
		///\brief Returns the number of data channels
		///
		///@return The number of data channels.
		U32	GetNumData() const;

		///\brief Clears the contents of this class to their initial state.
		///
		void Clear();

		bool operator== (const C_PolygonVertex& rhs) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void OneTimeInit();

		void DeleteData();

		///\brief Deletes the data for the given channel.
		///
		///@param The channel to remove.
		void DeleteData(U32 channel);

		///\brief Add a UV index to the array of UV indices for this polygon vertex.
		///
		///There must be a 1-1 relationship betweeen the number of UV indices and the number of UV sets.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@param textureCoordinateIndex The new texture coordinate index.
		///@return The index of the new index in the array of UV indices.
		S32	AddTextureCoordinateIndexI(S32 textureCoordinateIndex);

		///\brief Add a UV index to the array of UV indices for this polygon vertex.
		///
		///There must be a 1-1 relationship betweeen the number of UV indices and the number of UV sets.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@param textureCoordinateIndex The new texture coordinate index.
		///@param tangentIndex The new tangent index.
		///@param binormalIndex The new binormal index.
		///@return The index of the new index in the array of UV indices.
		S32	AddTextureCoordinateSetIndicesI(
			S32 textureCoordinateIndex, 
			S32 tangentIndex, 
			S32 binormalIndex);

		S32 AddData(E_GenericAttributeType dataType, const void* pData);

		void SetData(U32 channel, E_GenericAttributeType dataType, const void* pData);

		S32	m_positionIndex;
		S32	m_normalIndex;

	#ifdef AFTK_USE_MEMORY_POOL
		Ut::UtVector< Ut::DynamicVector< C_TextureCoordinateIndices, C_ArrayAllocator<C_TextureCoordinateIndices> > >	
			m_textureCoordinateIndices;

		Ut::UtVector< Ut::DynamicVector< U8 , C_ArrayAllocator<U8> > >
			m_dataChannels;
	#else
		Ut::UtVector< Ut::DynamicVector< C_TextureCoordinateIndices, Ut::ArrayAllocator<C_TextureCoordinateIndices> > >	
			m_textureCoordinateIndices;

		Ut::UtVector< Ut::DynamicVector< U8, Ut::ArrayAllocator<U8> > >
			m_dataChannels;
	#endif

		friend class C_SceneMeshNode;
		friend class C_NativePolygonVertex;

	#endif
	};

	typedef Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> > PolygonVertexArray;
}
#endif
