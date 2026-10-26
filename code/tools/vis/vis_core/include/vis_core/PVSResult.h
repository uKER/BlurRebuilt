#ifndef __PVSRESULT_H__
#define __PVSRESULT_H__

// contains the PVS of one sample cell - this is the result that the client sends back to the server
class PVSResult : public BaseObject
{
public:
	DECLARE_PERSISTENCE(PVSResult);

			PVSResult();
			PVSResult(int nPolyRefs);
			~PVSResult();

	// set one of the bits in the array
	inline bool SetBit(int i)
	{
		int iByte = i/8;
		int iBit = 1<<(i&7);

		if (iByte<0 || iByte >=m_iDataSize)
			return false;

		m_pData[iByte] |= iBit;
		return true;
	}

	// get one of the bits in the array
	inline bool GetBit(int i)
	{
		int iByte = i/8;
		int iBit = 1<<(i&7);

		return (m_pData[iByte] & iBit) != 0;
	}

	BYTE	*m_pData;
	int		m_iDataSize;

	int		m_iSamplePoint;

	void write( ost::Engine &ar );
	void read ( ost::Engine &ar );

};

#endif
