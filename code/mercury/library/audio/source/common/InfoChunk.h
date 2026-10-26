#ifndef _INFOCHUNK_H_
#define _INFOCHUNK_H_

static const int markerNameSize=32;

namespace Audio
{

	////////////////////////////////////////////////////////////////////////////////
	//	Generic Info Chunk
	////////////////////////////////////////////////////////////////////////////////
	
	struct InfoChunk 
	{
		U32 uTag;			// generic tag 'INFO'
		U32	uSize;			// size (includin the tag and size variables)
		U32 uType;			// specific type
	};

	struct Marker 
	{
		U32 uMarkerID;
		U32 uSampleOffset;
		char pchName[markerNameSize];	
	};

	struct MarkerChunk : InfoChunk
	{
		U32 uNumMarkers;	// 
		Ut::Vector<Marker*> MarkerList;
	};

	struct StreamingChunk : InfoChunk
	{
		U32 uReadChunkSize;
		U32 uCircularBufferSize;
		U32 uNumPrecacheChunks;
		U32 uStreamStartBufferSize;
	};
	
}

#endif // _INFOCHUNK_H_
