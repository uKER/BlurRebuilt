#ifndef CS_GEN_TEXTURE_KEY_H
#define CS_GEN_TEXTURE_KEY_H

namespace TexAtlas
{
	class TextureKey
	{
	public:
		inline TextureKey();
	
		inline TextureKey(const TextureKey& rhs);

		inline bool TextureKey::operator ==(const TextureKey& rhs) const;

		inline const TextureKey& operator =(const TextureKey& rhs);

		inline void SetFileName(const std::string fileName);

		inline const std::string& FileName() const;

		inline std::string& FileName();

		inline void SetIsBumpMap(bool flag);

		inline bool IsBumpMap() const;

		inline U32 Flags() const;

		inline S32 BaseIndex() const;

		inline S32 LayerIndex() const;

		inline F32 URepeats() const;

		inline F32 VRepeats() const;

		inline void SetBaseIndex(S32 index);

		///\brief Images on layer 0 are used to determine crop sheet packing. 
		///
		///All subsequent layers are then placed into crop sheets with identical packing to that of the
		///first layer. Each layer has as many crop sheets as were required for layer 0. Crop sheets only
		///contain the content of a single layer.
		inline void SetLayerIndex(S32 index);

		inline void SetFlags(U32 flags);

		///\brief The number of U repeats that should be included in the crop sheet.
		inline void SetURepeats(F32 repeats);

		///\brief The number of V repeats that should be included in the crop sheet.
		inline void SetVRepeats(F32 repeats);
		
		inline bool operator < (const TextureKey& rhs) const;

		inline void Deserialise(Tu::GenericDiskReader& reader);

		inline void Serialise(Tu::GenericDiskWriter& writer) const;

		inline void Clear();
	
	private:
		std::string m_fileName;
		struct FlagBitfield
		{
			unsigned isBumpMap:1;
		};

		FlagBitfield m_flags;
		S32 m_baseIndex;
		S32 m_layerIndex;
		F32 m_uRepeats;
		F32 m_vRepeats;
	};

	typedef Ut::UtVector<Ut::DynamicVector<TexAtlas::TextureKey> >		TextureKeyArray;

	#include "../../source/TextureKey.inl"
}

#endif
