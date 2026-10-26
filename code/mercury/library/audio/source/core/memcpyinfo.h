#ifndef _MEMCPYINFO_H__

struct MemCpyInfo
{
	void *pDst;
	void *pSrc;
	int	cSize;
};

struct FrameUpdateFinishInfo
{
//jkc	CellSpursEventFlag *pEvent;
	int nMemcpys;
	MemCpyInfo cpyInfo[10];
};

#endif
