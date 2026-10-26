#ifndef AFTK_TEXTURE_COORDINATE_SET_H
#define AFTK_TEXTURE_COORDINATE_SET_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	#ifndef DOXYGEN_SHOULD_SKIP_THIS

	class C_TextureCoordinateSet
		{
		public:
			C_TextureCoordinateSet()
			{
			}

			C_TextureCoordinateSet(const C_TextureCoordinateSet& rhs)
			{
				*this = rhs;
			}

			~C_TextureCoordinateSet() 
			{
			}

			const C_TextureCoordinateSet& operator=(const C_TextureCoordinateSet& rhs)
			{
				m_uvwx = rhs.m_uvwx;
				m_tangents = rhs.m_tangents;
				m_binormals = rhs.m_binormals;
				m_name = rhs.m_name;
				return *this;
			}

			void SetTextureCoordinateInitialCapacity(U32 capacity)
			{
				if ((U32)m_uvwx.Capacity() < capacity)
				{
					m_uvwx.Reserve(capacity);
				}
			}

			void SetTangentInitialCapacity(U32 capacity)
			{
				if ((U32)m_tangents.Capacity() < capacity)
				{
					m_tangents.Reserve(capacity);
				}
			}

			void SetBinormalInitialCapacity(U32 capacity)
			{
				if ((U32)m_binormals.Capacity() < capacity)
				{
					m_binormals.Reserve(capacity);
				}
			}
			
			U32 AddTextureCoordinate(const TextureCoordinate4& uvwx)
			{
				m_uvwx.PushBack(uvwx);
				return (U32)m_uvwx.Size()-1;
			}

			U32 AddTangent(const Vector3F32& tangents)
			{
				m_tangents.PushBack(tangents);
				return (U32)m_tangents.Size()-1;
			}

			U32 AddBinormal(const Vector3F32& binormal)
			{
				m_binormals.PushBack(binormal);
				return (U32)m_binormals.Size()-1;
			}

			void SetName(const std::string& name)
			{
				m_name = name;
			}

			void SetTextureCoordinate(U32 instance, const TextureCoordinate4& uvwx)
			{
				m_uvwx[instance] = uvwx;
			}

			void SetTangent(U32 instance, const Vector3F32& tangents)
			{
				m_tangents[instance] = tangents;
			}

			void SetBinormal(U32 instance, const Vector3F32& binormal)
			{
				m_binormals[instance] = binormal;
			}

			U32 GetNumTextureCoordinates() const
			{
				return (U32)m_uvwx.Size();
			}

			U32 GetNumTangents() const
			{
				return (U32)m_tangents.Size();
			}

			U32 GetNumBinormals() const
			{
				return (U32)m_binormals.Size();
			}

			const std::string& GetName() const
			{
				return m_name;
			}

			TextureCoordinate4& GetTextureCoordinate(U32 instance)
			{
				return m_uvwx[instance];
			}

			const TextureCoordinate4& GetTextureCoordinate(U32 instance) const
			{
				return m_uvwx[instance];
			}

			TextureCoordinate4Array& GetTextureCoordinateArray()
			{
				return m_uvwx;
			}

			const TextureCoordinate4Array& GetTextureCoordinateArray() const
			{
				return m_uvwx;
			}

			Vector3F32& GetTangent(U32 instance)
			{
				return m_tangents[instance];
			}

			const Vector3F32& GetTangent(U32 instance) const
			{
				return m_tangents[instance];
			}

			Vector3F32& GetBinormal(U32 instance)
			{
				return m_binormals[instance];
			}

			const Vector3F32& GetBinormal(U32 instance) const
			{
				return m_binormals[instance];
			}

			bool operator== (const C_TextureCoordinateSet& rhs) const
			{
				if (m_name != rhs.m_name)
				{
					return false;
				}

				if ((m_uvwx.Size() != rhs.m_uvwx.Size()) ||
					(m_tangents.Size() != rhs.m_tangents.Size()) || 
					(m_binormals.Size() != rhs.m_binormals.Size()))
				{
					return false;
				}

				if (m_uvwx.Size() > 0)
				{
					if (memcmp(&m_uvwx[0], &rhs.m_uvwx[0], sizeof(TextureCoordinate4)*m_uvwx.Size() != 0))
						return false;
				}
				if (m_tangents.Size() > 0)
				{
					if (memcmp(&m_tangents[0], &rhs.m_tangents[0], sizeof(Vector3F32)*m_tangents.Size() != 0))
						return false;
				}
				if (m_binormals.Size() > 0)
				{
					if (memcmp(&m_binormals[0], &rhs.m_binormals[0], sizeof(Vector3F32)*m_binormals.Size() != 0))
						return false;
				}

				return true;
			}

			void Clear()
			{
				m_name.clear();
				m_uvwx.Clear();
				m_tangents.Clear();
				m_binormals.Clear();
			}

			Vector3F32Array& GetTangentArray()
			{
				return m_tangents;
			}

			Vector3F32Array& GetBinormalArray()
			{
				return m_binormals;
			}

			const Vector3F32Array& GetTangentArray() const
			{
				return m_tangents;
			}

			const Vector3F32Array& GetBinormalArray() const
			{
				return m_binormals;
			}

		private:
			Ut::UtVector< Ut::DynamicVector<TextureCoordinate4> > m_uvwx;
			Ut::UtVector< Ut::DynamicVector<Vector3F32> > m_tangents;
			Ut::UtVector< Ut::DynamicVector<Vector3F32> > m_binormals;
			
			//Texture coordinate set name.
			std::string m_name;

			friend class C_SceneMeshNode;
		};
	#endif
}
#endif
