#ifndef CS_GEN_INPUT_LIST_H
#define CS_GEN_INPUT_LIST_H

namespace TexAtlas
{
	class InputList
	{
	public:
		InputList(const InputList& rhs);

		const InputList& operator =(const InputList& rhs);

		U32 AddRecord(const ImageDescription& r);

		const ImageDescription& Record(U32 index) const;

		ImageDescription& Record(U32 index);

		U32 NumRecords() const;

	private:
		Ut::UtVector<Ut::DynamicVector<ImageDescription> > m_textureList;
	};
}

#endif
