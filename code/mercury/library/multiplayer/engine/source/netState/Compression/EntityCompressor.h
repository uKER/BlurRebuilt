#ifndef _ENTITY_COMPRESSOR
#define _ENTITY_COMPRESSOR

#include "netStream/netSerialise.h"

class C_Compression : public netSerialisable
{
public:
	C_Compression(const int uStateSize);
	C_Compression(const C_Compression& xCopy);
	~C_Compression();
	void	ClearDifference()	{memset(m_pszDataFlags, 0, m_uDataFlagSize);}
	void	FullState()			{memset(m_pszDataFlags, 255, m_uDataFlagSize);}

	void	FlagDifferent(int i);
	bool	IsDifferent(int i) const;

	bool	Compress(class C_StateDataBuffer& xStateBuffer, int uLastState);
	bool	Compress(const class C_StateData& xDest, const class C_StateData& xRef);
	bool	UnCompress(class C_StateData& xDest, const class C_StateData& xRef);

	virtual bool serialise(class netStream* stream);
	virtual bool deserialise(class netStream* stream);

	// debug/util
	int		DifferenceSize() const;
	int		CompressionHeaderSize() const;
	int		CompressionHeaderSizeNoRLE() const;
	bool	IsFullState() const	
			{
				for(int i = 0; i < m_uDataFlagSize; ++i)
				{
					if(m_pszDataFlags[i] != -1)
						return false;
				}
				return true;
			}

private:
	const int	m_uStateSize;
	int			m_uDataFlagSize; // TODO: should be const!
	char*		m_pszDataFlags;
};


#endif