#include <stdafx.h>

using namespace Aftk;

#define AFTK_RESIZE_UVSETS_IF_NEEDED \
while (set >= (U32)m_textureCoordinateIndices.Size()) \
{ \
if (set > 100) \
{\
	LOGFATAL(g_aftkMainLogger, "Encountered PV UV set addition above 100!");\
}\
AddTextureCoordinateSetIndicesI(-1,-1,-1);\
}

C_PolygonVertex::C_PolygonVertex()
{
	OneTimeInit();
}

C_PolygonVertex::~C_PolygonVertex()
{
}

C_PolygonVertex::C_PolygonVertex(const C_PolygonVertex &rhs)
{
	*this = rhs;
}

C_PolygonVertex& C_PolygonVertex::operator =(const C_PolygonVertex &rhs)
{
	m_dataChannels =				rhs.m_dataChannels;
	m_normalIndex =					rhs.m_normalIndex; 
	m_positionIndex =				rhs.m_positionIndex;
	m_textureCoordinateIndices =	rhs.m_textureCoordinateIndices;

	return *this;
}

bool C_PolygonVertex::EqualityTest(const C_PolygonVertex& rhs, F32 epsilon) const
{
	//Dimensions are equal, now compare contents
	if (m_positionIndex != rhs.m_positionIndex)
	{
		return false;
	}

	if ((m_dataChannels.Size() != rhs.m_dataChannels.Size())
		||
		(m_textureCoordinateIndices.Size() != rhs.m_textureCoordinateIndices.Size())
		)
	{
		return false;
	}

	if (m_normalIndex != rhs.m_normalIndex)
	{
		return false;
	}

	//Perform a threshold equality test on the arbitrary data channels.
	{
		U32 blockStart = 0;
		for (U32 iChannel = 0; iChannel < (U32)GetNumData(); iChannel++)
		{
			E_GenericAttributeType lhsChannelDataType = 
				(E_GenericAttributeType)m_dataChannels[blockStart];
			E_GenericAttributeType rhsChannelDataType = 
				(E_GenericAttributeType)rhs.m_dataChannels[blockStart];
			//Return false if any of the channels use different data types.
			if (lhsChannelDataType != rhsChannelDataType)
			{
				return false;
			}

			U32 channelDataTypeSize = GetDataTypeSize(lhsChannelDataType);

			//Both the lhs and rhs channels are of the same type. Perform an appropriate comparison
			//for this type.
			switch(lhsChannelDataType)
			{
			case E_GenericAttributeType_U8:
			case E_GenericAttributeType_U16:
			case E_GenericAttributeType_U32:
			case E_GenericAttributeType_U64:
			case E_GenericAttributeType_S8:
			case E_GenericAttributeType_S16:
			case E_GenericAttributeType_S32:
			case E_GenericAttributeType_S64:
			case E_GenericAttributeType_Boolean:
			case E_GenericAttributeType_Vector2S32:
			case E_GenericAttributeType_Vector2S16:
			case E_GenericAttributeType_Vector2S8:
			case E_GenericAttributeType_Vector2U32:
			case E_GenericAttributeType_Vector2U16:
			case E_GenericAttributeType_Vector2U8:
			case E_GenericAttributeType_Vector3S32:
			case E_GenericAttributeType_Vector3S16:
			case E_GenericAttributeType_Vector3S8:
			case E_GenericAttributeType_Vector3U32:
			case E_GenericAttributeType_Vector3U16:
			case E_GenericAttributeType_Vector3U8:
			case E_GenericAttributeType_Vector4S32:
			case E_GenericAttributeType_Vector4S16:
			case E_GenericAttributeType_Vector4S8:
			case E_GenericAttributeType_Vector4U32:
			case E_GenericAttributeType_Vector4U16:
			case E_GenericAttributeType_Vector4U8:
			case E_GenericAttributeType_Vector4U64:
			case E_GenericAttributeType_Vector4S64:
			case E_GenericAttributeType_Vector3U64:
			case E_GenericAttributeType_Vector3S64:
			case E_GenericAttributeType_Vector2U64:
			case E_GenericAttributeType_Vector2S64:
				if (memcmp(&(m_dataChannels[blockStart+1]), &(rhs.m_dataChannels[blockStart+1]), channelDataTypeSize) != 0) 
				{
					//Bytes are not equal!
					return false;
				}

				break;

			case E_GenericAttributeType_F32:
				if (!EqualityTestFScalar((F32)m_dataChannels[blockStart+1], (F32)rhs.m_dataChannels[blockStart+1], epsilon))
				{
					return false;
				}
				break;

			case E_GenericAttributeType_F64:
				if (!EqualityTestFScalar((F64)m_dataChannels[blockStart+1], (F64)rhs.m_dataChannels[blockStart+1], epsilon))
				{
					return false;
				}
				break;

			case E_GenericAttributeType_Vector2F32:
			{
				const Vector2F32* pLhs = (const Vector2F32*)GetDataPtr(iChannel);
				const Vector2F32* pRhs = (const Vector2F32*)rhs.GetDataPtr(iChannel);
				if (!EqualityTestFVector(*pLhs, *pRhs, epsilon))
				{
					return false;
				}
				break;
			}
			case E_GenericAttributeType_Vector2F64:
			{
				const Vector2F64* pLhs = (const Vector2F64*)GetDataPtr(iChannel);
				const Vector2F64* pRhs = (const Vector2F64*)rhs.GetDataPtr(iChannel);
				if (!EqualityTestFVector(*pLhs, *pRhs, epsilon))
				{
					return false;
				}
				break;
			}
			case E_GenericAttributeType_Vector3F32:
			{
				const Vector3F32* pLhs = (const Vector3F32*)GetDataPtr(iChannel);
				const Vector3F32* pRhs = (const Vector3F32*)rhs.GetDataPtr(iChannel);
				if (!EqualityTestFVector(*pLhs, *pRhs, epsilon))
				{
					return false;
				}
				break;
			}
			case E_GenericAttributeType_Vector3F64:
			{
				const Vector3F64* pLhs = (const Vector3F64*)GetDataPtr(iChannel);
				const Vector3F64* pRhs = (const Vector3F64*)rhs.GetDataPtr(iChannel);
				if (!EqualityTestFVector(*pLhs, *pRhs, epsilon))
				{
					return false;
				}
				break;
			}
			case E_GenericAttributeType_Vector4F64:
			{
				const Vector4F64* pLhs = (const Vector4F64*)GetDataPtr(iChannel);
				const Vector4F64* pRhs = (const Vector4F64*)rhs.GetDataPtr(iChannel);
				if (!EqualityTestFVector(*pLhs, *pRhs, epsilon))
				{
					return false;
				}
				break;
			}
			case E_GenericAttributeType_Vector4F32:
			{
				const Vector4F32* pLhs = (const Vector4F32*)GetDataPtr(iChannel);
				const Vector4F32* pRhs = (const Vector4F32*)rhs.GetDataPtr(iChannel);
				if (!EqualityTestFVector(*pLhs, *pRhs, epsilon))
				{
					return false;
				}
				break;
			}

			case E_GenericAttributeType_Matrix44F32:
			{
				const Matrix44* pLhs = (const Matrix44*)GetDataPtr(iChannel);
				const Matrix44* pRhs = (const Matrix44*)rhs.GetDataPtr(iChannel);
				if (!EqualityTestFVector(*pLhs, *pRhs, epsilon))
				{
					return false;
				}
				break;
			}

			case E_GenericAttributeType_Null:
				//Null types not supported in arbitrary data channel.
				AFTK_ASSERT(lhsChannelDataType != E_GenericAttributeType_Null);
				break;			
			case E_GenericAttributeType_String:
				//Strings not supported in arbitrary data channel.
				AFTK_ASSERT(lhsChannelDataType != E_GenericAttributeType_String);
				break;
			case E_GenericAttributeType_Unknown:
				//Unknown types not supported in arbitrary data channel.
				AFTK_ASSERT(lhsChannelDataType != E_GenericAttributeType_String);
				break;

			default:
				AFTK_ASSERT(!("Equality test failed due to unknown data channel type."));
				break;
			}

			//Skip to the next block.
			blockStart += GetDataTypeSize(lhsChannelDataType) + 1; 
		}
	}

	for (U32 i = 0; i < (U32)m_textureCoordinateIndices.Size(); i++)
	{
		if ((m_textureCoordinateIndices[i].binormal != rhs.m_textureCoordinateIndices[i].binormal)
			||
			(m_textureCoordinateIndices[i].tangent != rhs.m_textureCoordinateIndices[i].tangent)
			||
			(m_textureCoordinateIndices[i].textureCoordinate != rhs.m_textureCoordinateIndices[i].textureCoordinate))
		{
			return false;
		}
	}

	return true;
}

bool C_PolygonVertex::LessThanTest(const C_PolygonVertex& rhs, F32 epsilon) const
{
	if (EqualityTest(rhs, epsilon))
	{
		return false;
	}
	else if (m_positionIndex >= rhs.m_positionIndex)
	{
		return false;
	}

	return true;
}

void C_PolygonVertex::SetPositionIndex(S32 index)
{
	m_positionIndex = index;
}

void C_PolygonVertex::SetNormalIndex(S32 index)
{
	m_normalIndex = index;
}

void C_PolygonVertex::SetTextureCoordinateIndex(U32 set, S32 index)
{
	AFTK_RESIZE_UVSETS_IF_NEEDED;
	m_textureCoordinateIndices[set].textureCoordinate = index;
}

void C_PolygonVertex::SetTextureCoordinateSetIndices(
	U32 set,
	S32 textureCoordinateIndex, 
	S32 tangentIndex, 
	S32 binormalIndex)
{
	AFTK_RESIZE_UVSETS_IF_NEEDED;

	m_textureCoordinateIndices[set].textureCoordinate = textureCoordinateIndex;
	m_textureCoordinateIndices[set].tangent = tangentIndex;
	m_textureCoordinateIndices[set].binormal = binormalIndex;
}

void C_PolygonVertex::DeleteTextureCoordinateIndices(U32 set)
{
	m_textureCoordinateIndices.EraseByIndex(set);
}

void C_PolygonVertex::DeleteTextureCoordinateIndices()
{
	m_textureCoordinateIndices.Clear();
}

void C_PolygonVertex::SetTangentIndex(U32 set, S32 index)
{
	AFTK_RESIZE_UVSETS_IF_NEEDED;
	m_textureCoordinateIndices[set].tangent = index;
}

void C_PolygonVertex::SetBinormalIndex(U32 set, S32 index)
{
	AFTK_RESIZE_UVSETS_IF_NEEDED;
	m_textureCoordinateIndices[set].binormal = index;
}

void C_PolygonVertex::SetData(U32 channel, U8 data)
{
	SetData(channel, E_GenericAttributeType_U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, U16 data)
{
	SetData(channel, E_GenericAttributeType_U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, U32 data)
{
	SetData(channel, E_GenericAttributeType_U32, &data);
}

void C_PolygonVertex::SetData(U32 channel, U64 data)
{
	SetData(channel, E_GenericAttributeType_U64, &data);
}

void C_PolygonVertex::SetData(U32 channel, S8 data)
{
	SetData(channel, E_GenericAttributeType_S8, &data);
}

void C_PolygonVertex::SetData(U32 channel, S16 data)
{
	SetData(channel, E_GenericAttributeType_S16, &data);
}

void C_PolygonVertex::SetData(U32 channel, S32 data)
{
	SetData(channel, E_GenericAttributeType_S32, &data);
}

void C_PolygonVertex::SetData(U32 channel, S64 data)
{
	SetData(channel, E_GenericAttributeType_S64, &data);
}

void C_PolygonVertex::SetData(U32 channel, F32 data)
{
	SetData(channel, E_GenericAttributeType_F32, &data);
}

void C_PolygonVertex::SetData(U32 channel, F64 data)
{
	SetData(channel, E_GenericAttributeType_F64, &data);
}

void C_PolygonVertex::SetData(U32 channel, bool data)
{
	SetData(channel, E_GenericAttributeType_Boolean, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Matrix44& data)
{
	SetData(channel, E_GenericAttributeType_Matrix44F32, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2U8& data)
{
	SetData(channel, E_GenericAttributeType_Vector2U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2U16& data)
{
	SetData(channel, E_GenericAttributeType_Vector2U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2U32& data)
{
	SetData(channel, E_GenericAttributeType_Vector2U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2U64& data)
{
	SetData(channel, E_GenericAttributeType_Vector2U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2S8& data)
{
	SetData(channel, E_GenericAttributeType_Vector2U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2S16& data)
{
	SetData(channel, E_GenericAttributeType_Vector2U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2S32& data)
{
	SetData(channel, E_GenericAttributeType_Vector2U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2S64& data)
{
	SetData(channel, E_GenericAttributeType_Vector2U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2F32& data)
{
	SetData(channel, E_GenericAttributeType_Vector2F32, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector2F64& data)
{
	SetData(channel, E_GenericAttributeType_Vector2F64, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3U8& data)
{
	SetData(channel, E_GenericAttributeType_Vector3U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3U16& data)
{
	SetData(channel, E_GenericAttributeType_Vector3U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3U32& data)
{
	SetData(channel, E_GenericAttributeType_Vector3U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3U64& data)
{
	SetData(channel, E_GenericAttributeType_Vector3U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3S8& data)
{
	SetData(channel, E_GenericAttributeType_Vector3U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3S16& data)
{
	SetData(channel, E_GenericAttributeType_Vector3U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3S32& data)
{
	SetData(channel, E_GenericAttributeType_Vector3U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3S64& data)
{
	SetData(channel, E_GenericAttributeType_Vector3U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3F32& data)
{
	SetData(channel, E_GenericAttributeType_Vector3F32, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector3F64& data)
{
	SetData(channel, E_GenericAttributeType_Vector3F64, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4U8& data)
{
	SetData(channel, E_GenericAttributeType_Vector4U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4U16& data)
{
	SetData(channel, E_GenericAttributeType_Vector4U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4U32& data)
{
	SetData(channel, E_GenericAttributeType_Vector4U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4U64& data)
{
	SetData(channel, E_GenericAttributeType_Vector4U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4S8& data)
{
	SetData(channel, E_GenericAttributeType_Vector4U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4S16& data)
{
	SetData(channel, E_GenericAttributeType_Vector4U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4S32& data)
{
	SetData(channel, E_GenericAttributeType_Vector4U8, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4S64& data)
{
	SetData(channel, E_GenericAttributeType_Vector4U16, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4F32& data)
{
	SetData(channel, E_GenericAttributeType_Vector4F32, &data);
}

void C_PolygonVertex::SetData(U32 channel, const Vector4F64& data)
{
	SetData(channel, E_GenericAttributeType_Vector4F64, &data);
}

S32 C_PolygonVertex::GetPositionIndex() const
{
	return m_positionIndex;
}

S32 C_PolygonVertex::GetNormalIndex() const
{
	return m_normalIndex;
}

S32 C_PolygonVertex::GetTextureCoordinateIndex(U32 set) const
{
	return m_textureCoordinateIndices[set].textureCoordinate;
}

S32 C_PolygonVertex::GetTangentIndex(U32 set) const
{
	return m_textureCoordinateIndices[set].tangent;
}

S32 C_PolygonVertex::GetBinormalIndex(U32 set) const
{
	return m_textureCoordinateIndices[set].binormal;
}
/*
void C_PolygonVertex::GetData(U32 channel, Vector4F32& data) const
{
	data = *((Vector4F32*)GetDataPtr(channel));
}
*/
const void* C_PolygonVertex::GetDataPtr(U32 channel) const
{
	C_PolygonVertex* pThis = const_cast<C_PolygonVertex*>(this);
	return pThis->GetDataPtr(channel);
}

void* C_PolygonVertex::GetDataPtr(U32 channel)
{
	U32 blockStart = 0;
	//Find the start of the given channel.
	for (U32 iChannel = 0; iChannel < channel; iChannel++)
	{
		E_GenericAttributeType channelDataType = 
			(E_GenericAttributeType)m_dataChannels[blockStart];

		//Skip to the next block.
		blockStart += GetDataTypeSize(channelDataType) + 1; 
	}

	return &m_dataChannels[blockStart+1];
}

U32 C_PolygonVertex::GetNumData() const
{
	U32 blockStart = 0;
	U32 channelCount = 0;
	//Find the start of the given channel.
	while (blockStart < (U32)m_dataChannels.Size())
	{
		E_GenericAttributeType channelDataType = 
			(E_GenericAttributeType)m_dataChannels[blockStart];

		//Skip to the next block.
		blockStart += GetDataTypeSize(channelDataType) + 1; 
		channelCount++;
	}

	return channelCount;
}

U32 C_PolygonVertex::GetNumTextureCoordinateIndices() const
{
	return (U32)m_textureCoordinateIndices.Size();
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_PolygonVertex::OneTimeInit()
{
	m_normalIndex = -1;
	m_positionIndex = -1;
}

void C_PolygonVertex::Clear()
{
	m_normalIndex = -1;
	m_positionIndex = -1;

	m_dataChannels.Clear();
	m_textureCoordinateIndices.Clear();
}

S32 C_PolygonVertex::AddData(E_GenericAttributeType dataType, const void* pData)
{
	U32 blockStart = 0;
	U32 dataTypeSize = GetDataTypeSize(dataType);
	//Find the start of the given channel.
	U32 originalChannelCount = GetNumData();
	for (U32 iChannel = 0; iChannel < originalChannelCount; iChannel++)
	{
		E_GenericAttributeType channelDataType = 
			(E_GenericAttributeType)m_dataChannels[blockStart];

		//Skip to the next block.
		blockStart += GetDataTypeSize(channelDataType) + 1; 
	}
	AFTK_ASSERT(blockStart >= (U32)m_dataChannels.Size());

	//We're not already storing data for this channel. We need to increase the array size.
	m_dataChannels.Resize(m_dataChannels.Size() + dataTypeSize + 1);
	//Store the data type.
	m_dataChannels[blockStart] = (U8)dataType;
	//Insert the given data.
	memcpy(&m_dataChannels[blockStart+1], pData, dataTypeSize);

	return originalChannelCount;
}

void C_PolygonVertex::SetData(U32 channel, E_GenericAttributeType dataType, const void* pData)
{
	U32 blockStart = 0;
	U32 dataTypeSize = GetDataTypeSize(dataType);

#ifdef BUILD_CONFIG_DEBUG
	{
		U32 blockStart = 0;
		U32 nChannels = GetNumData();
		for (U32 iChannel = 0; iChannel < nChannels; iChannel++)
		{
			E_GenericAttributeType channelDataType = 
				(E_GenericAttributeType)m_dataChannels[blockStart];

			//Skip to the next block.
			blockStart += GetDataTypeSize(channelDataType) + 1; 
		}
		assert(blockStart == (U32)m_dataChannels.Size());
	}
#endif

	//Find the start of the given channel.
	for (U32 iChannel = 0; iChannel < channel; iChannel++)
	{
		E_GenericAttributeType channelDataType = 
			(E_GenericAttributeType)m_dataChannels[blockStart];

		//Skip to the next block.
		blockStart += GetDataTypeSize(channelDataType) + 1; 
	}

	if (blockStart == (U32)m_dataChannels.Size())
	{
		//We're not already storing data for this channel. We need to increase the array size.
		m_dataChannels.Resize(m_dataChannels.Size() + dataTypeSize + 1);
	}

	//Store the data type.
	m_dataChannels[blockStart] = (U8)dataType;
	//Insert the given data.
	memcpy(&m_dataChannels[blockStart+1], pData, dataTypeSize);
}

S32 C_PolygonVertex::AddTextureCoordinateIndexI(S32 index)
{
	C_TextureCoordinateIndices tci;
	tci.textureCoordinate = index;
	tci.tangent = -1;
	tci.binormal = -1;
	m_textureCoordinateIndices.PushBack(tci);
	return (S32)(m_textureCoordinateIndices.Size() -1);
}

S32 C_PolygonVertex::AddTextureCoordinateSetIndicesI(
	S32 textureCoordinateIndex,
	S32 tangentIndex, 
	S32 binormalIndex)
{
	C_TextureCoordinateIndices tci;
	tci.textureCoordinate = textureCoordinateIndex;
	tci.tangent = tangentIndex;
	tci.binormal = binormalIndex;
	m_textureCoordinateIndices.PushBack(tci);
	return (S32)(m_textureCoordinateIndices.Size() -1);
}

/*
void C_PolygonVertex::DeleteData()
{
	U32 blockStart = 0;
	for (U32 iChannel = 0; iChannel < GetNumData()-1; iChannel++)
	{
		E_GenericAttributeType channelDataType = 
			(E_GenericAttributeType)m_dataChannels[blockStart];

		//Skip to the next block.
		blockStart += GetDataTypeSize(channelDataType) + 1; 		
	}

	m_dataChannels.Resize(blockStart);
}
*/
void C_PolygonVertex::DeleteData()
{
	S32 dataCount = GetNumData();
	while(dataCount > 0)
	{
		DeleteData(dataCount-1);
		dataCount--;
	}
}

void C_PolygonVertex::DeleteData(U32 channel)
{
	if (m_dataChannels.Size() == 0)
		return;

	U32 blockStart = 0;
	for (U32 iChannel = 0; iChannel < channel; iChannel++)
	{
		E_GenericAttributeType channelDataType = 
			(E_GenericAttributeType)m_dataChannels[blockStart];

		//Skip to the next block.
		blockStart += GetDataTypeSize(channelDataType) + 1;
	}

	E_GenericAttributeType channelDataType = 
			(E_GenericAttributeType)m_dataChannels[blockStart];

	U32 blockSize = GetDataTypeSize(channelDataType) + 1;
	U32 toCopy = m_dataChannels.Size() - (blockStart+blockSize);
	if (toCopy > 0)
		memmove(&m_dataChannels[blockStart], &m_dataChannels[blockStart+blockSize], toCopy);
	m_dataChannels.Resize(toCopy);
}


bool C_PolygonVertex::operator== (const C_PolygonVertex& rhs) const
{
	if ((m_positionIndex != rhs.m_positionIndex) || 
		(m_normalIndex != rhs.m_normalIndex))
	{
		return false;
	}

	if ((m_textureCoordinateIndices.Size() != rhs.m_textureCoordinateIndices.Size()) ||
		(m_dataChannels.Size() != rhs.m_dataChannels.Size()))
	{
		return false;
	}

	if (m_textureCoordinateIndices.Size() > 0)
	{
		if (memcmp(&m_textureCoordinateIndices[0], &rhs.m_textureCoordinateIndices[0],sizeof(C_TextureCoordinateIndices)*m_textureCoordinateIndices.Size()) != 0)
			return false;
	}
	if (m_dataChannels.Size() > 0)
	{
		if (memcmp(&m_dataChannels[0], &rhs.m_dataChannels[0], sizeof(U8)*m_dataChannels.Size()) != 0)
			return false;
	}

	return true;
}


#endif
