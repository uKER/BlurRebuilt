#pragma once

class CBinarySerialize
{
public:
	int		m_iFileLength;
	char	*m_pData;

			CBinarySerialize() : m_pData(0) {};
	void	Purge();			
	void	SerializeObject(BaseObject *pObject, int iGrowSize = 1024*128);
	BaseObject *ReadObject();
};
