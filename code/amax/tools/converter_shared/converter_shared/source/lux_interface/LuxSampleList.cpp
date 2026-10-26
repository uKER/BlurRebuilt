#include "stdafx.h"

#include <system/misc/StringUtil.h>
#include <btl/Containers/set.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <converter_shared/lux_interface/LuxSampleList.h>

namespace LuxSampleList
{	
class C_SampleList;

// C_SampleData

C_SampleData::C_SampleData()
{
}

C_SampleData::~C_SampleData()
{
}

bool C_SampleData::operator==( const C_SampleData& data )
{
	if (m_Data.Size() != data.m_Data.Size())
		return false;

	for (S32 i = 0; i < m_Data.Size(); i++)
	{
		if (m_Data[i] != data.m_Data[i])
			return false;
	}
	return true;
}
// C_Sample

C_Sample::C_Sample()
	: m_Position( Maths::HVec3(0.0f, 0.0f, 0.0f) )
	, m_Index( -1 )
	, m_pSampleList( NULL )
	, m_SampleDataIndex( -1 )
{
}

C_Sample::C_Sample( C_SampleList* pSampleList, const S32 index, const Maths::HVec3 PCREF position )
	: m_Position( position )
	, m_Index( index )
	, m_pSampleList( pSampleList )
	, m_SampleDataIndex( -1 )
{
}

C_Sample::~C_Sample()
{
}

Maths::HVec3 C_Sample::GetPosition() const
{
	return m_Position;
}

// C_SampleList

C_SampleList::C_SampleList( const S32 dataElementCount )
	: m_DataElementCount( dataElementCount )
{
}

C_SampleList::~C_SampleList()
{
}

S32 C_SampleList::AddSamplePoint( const Maths::HVec3 PCREF pos )
{
	const S32 newSampleIndex = m_SampleList.Size();
	m_SampleList.PushBack( C_Sample(this, newSampleIndex, pos) );

	return newSampleIndex;
}

bool C_SampleList::WriteSamplePointsFile( const Util::C_String& filename )
{
	Util::C_String outputDir = Util::ExtractDirectory(filename);
	Sys::C_FileSystem::Instance().CreateDirectory( outputDir );

	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( filename, Sys::E_FileAccess_Write );
    
    if(pFile == NULL)
    {
        LOGERROR(Tools::g_log, "Failed to open output file %s\n", filename.Chars());
        return false;
    }       

	Sys::BinarySerialiserAutoEndian ar( *pFile );

    const S32 sampleCount = m_SampleList.Size();    
	for(S32 sampleNo=0;sampleNo<sampleCount; sampleNo++)
    {
		C_Sample& currSample = m_SampleList[sampleNo];
		Maths::HVec3 pos = currSample.GetPosition();
        F32 x = pos.X();
        F32 y = pos.Y();
        F32 z = pos.Z();

        ar << x << y << z;
    }  

	SAFE_DELETE_PTR( pFile );

	return true;
}

bool C_SampleList::ReadSamplePointsFile( const Util::C_String& filename )
{
	m_SampleList.Clear();
	m_SampleDataList.Clear();

    Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( filename, Sys::E_FileAccess_Read );
       
    if(pFile == NULL)
    {        
        LOGERROR(Tools::g_log, "Failed to open file %s\n", filename.Chars());
        return false;
    }           
              
	Sys::BinaryDeserialiserAutoEndian ar( *pFile );

	Sys::C_StreamSize fileSize = pFile->Length();

    const U32 sampleCount = (U32)(fileSize / (sizeof( F32 ) * 3));    
    LOGDEBUG(Tools::g_log,  "Reading %d sample point positions\n", sampleCount);
    
	m_SampleList.Reserve(sampleCount);
    for(U32 i=0; i<sampleCount; i++)
    {
        F32 x, y, z;

		ar >> x >> y >> z;
        
		m_SampleList.PushBack( C_Sample( this, i, Maths::HVec3( x, y, z ) ) );
    }

	SAFE_DELETE_PTR(pFile);

	return true;
}

bool C_SampleList::ReadSampleDataFile( const Util::C_String& filename )
{
    Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( filename, Sys::E_FileAccess_Read );
       
    if(pFile == NULL)
    {        
        LOGERROR(Tools::g_log, "Failed to open file %s\n", filename.Chars());
        return false;
    }           
              
	Sys::BinaryDeserialiserAutoEndian ar( *pFile );

	U32 someData;
	ar >> someData; // was once used for the file size

    const S32 sampleCount = m_SampleList.Size();    

    LOGDEBUG(Tools::g_log,  "Reading %d sample data items\n", sampleCount);
    
	m_SampleDataList.Clear();
	m_SampleDataList.Resize( sampleCount );
    for(S32 sampleNo=0; sampleNo<sampleCount; sampleNo++)
    {		
		// Setup index from sample
		m_SampleList[sampleNo].m_SampleDataIndex = sampleNo;

		// Setup C_SampleData
		C_SampleData& newSampleData = m_SampleDataList[sampleNo];
		newSampleData.m_Data.Resize( m_DataElementCount );

		for(S32 dataElementNo=0; dataElementNo<m_DataElementCount; dataElementNo++)
		{
			ar >> newSampleData.m_Data[dataElementNo];
		}
    }

	SAFE_DELETE_PTR(pFile);

	return true;
}

void C_SampleList::CompressSampleData()
{
	//Associate between old and new sample data
	Btl::Vector<S32> associateIndex;
	//New sample data, redundant removed
	Btl::Vector<S32> newIndex;
	//Is it processed?
	Btl::Set<S32> indexProcessed;

	associateIndex.Resize(m_SampleDataList.Size());
	for (S32 i = 0; i < m_SampleDataList.Size(); i++)
	{
		if (indexProcessed.Find(i) != indexProcessed.End())
		{
			//Not processed
			associateIndex[i] = newIndex.Size();
			//Look if has equal SampleData
			for (S32 j = i + 1; j < m_SampleDataList.Size(); j++)
			{
				if (indexProcessed.Find(j) != indexProcessed.End())
				{
					if (m_SampleDataList[i] == m_SampleDataList[j])
					{
						// j == i
						associateIndex[j] = newIndex.Size();
						//Processed
						indexProcessed.Insert(j);
					}
				}
			}
			newIndex.PushBack(i);
		}
	}
	
	//Rebuild index
	for (S32 i = 0; i < m_SampleList.Size(); i++)
	{
		m_SampleList[i].m_Index = associateIndex[m_SampleList[i].m_Index];
	}
	
	//Rebuild data
	Btl::Vector<C_SampleData> newSampleDataList;
	newSampleDataList.Resize(newIndex.Size());

	for (S32 i = 0; i < newIndex.Size(); i++)
	{
		//Build
		newSampleDataList[i] = m_SampleDataList[newIndex[i]];
	}

	//Replace
	m_SampleDataList = newSampleDataList; 
}

//////////////////////////////////////////////////////////////////////////
// BSP Point Tree
//////////////////////////////////////////////////////////////////////////
void C_BSPPointTree::Build( S32 bucketSize, Maths::HVec3* positions, U32 numPositions )
{
	assert(numPositions > 0);

	//Build Index Src
	Btl::Vector<U32> indexPositionVector;
	indexPositionVector.Resize(numPositions);
	for (U32 i = 0; i < numPositions; i++)
		indexPositionVector[i] = i;

	m_pRootNode = InternalBuild(bucketSize, positions, numPositions, indexPositionVector);
}

//Internal Build
C_BSPPointTreeNode* C_BSPPointTree::InternalBuild(S32 bucketSize, Maths::HVec3* positions, U32 numPositions, Btl::Vector<U32>& indexPositionVector )
{
	if (indexPositionVector.Size() > bucketSize)
	{
		//Split nodes

		//Calculate plane
		Maths::C_AxisBox bounds;
		bounds.SetInvalidExtents();
		for( S32 i = 0; i < indexPositionVector.Size(); i++)
		{
			bounds.UpdateExtents( (Maths::C_Vertex)positions[indexPositionVector[i]] );
		}

		//Calculate longest axis
		Maths::HVec3 boundsDiagonal = bounds.Diagonal();
		Maths::HVec3 boundsCentre = bounds.Centre();

		U32 longestAxis = 0;
		Maths::HF32 longestAxisLength = boundsDiagonal.X();
		if(boundsDiagonal.Y() > longestAxisLength)
		{
			longestAxis = 1;
			longestAxisLength = boundsDiagonal.Y();
		}
		if(boundsDiagonal.Z() > longestAxisLength)
		{
			longestAxis = 2;
			longestAxisLength = boundsDiagonal.Z();
		}

		if(longestAxisLength > FLT_EPSILON)
		{
			// Generate split plane
			Maths::C_Normal splitNormal;
			switch(longestAxis)
			{
			case 0:
				splitNormal = Maths::C_Normal(1.0f, 0.0f, 0.0f);
				break;
			case 1:
				splitNormal = Maths::C_Normal(0.0f, 1.0f, 0.0f);
				break;
			case 2:
				splitNormal = Maths::C_Normal(0.0f, 0.0f, 1.0f);
				break;
			}

			Maths::C_Plane splitPlane( splitNormal, boundsCentre );
			
			Btl::Vector<U32> frontIndexPositionVector;
			Btl::Vector<U32> backIndexPositionVector;
			//Spit, using aux index vector
			for (S32 i = 0; i < indexPositionVector.Size(); i++)
			{
				if (splitPlane.Distance(positions[indexPositionVector[i]]) > 0.f)
				{
					//Front
					frontIndexPositionVector.PushBack(indexPositionVector[i]);
				}
				else
				{
					//Back
					backIndexPositionVector.PushBack(indexPositionVector[i]);
				}
			}

			//Create front and back nodes
			C_BSPPointTreeNode* pFrontBuffer = NULL;
			C_BSPPointTreeNode* pBackBuffer = NULL;
			if (frontIndexPositionVector.Size() > 0)
				pFrontBuffer = InternalBuild(bucketSize, positions, numPositions, frontIndexPositionVector);
			if (backIndexPositionVector.Size() > 0)
				pBackBuffer = InternalBuild(bucketSize, positions, numPositions, backIndexPositionVector);
			
			//Create node
			return AddBuildBranchNode(AddPlane(splitPlane), pFrontBuffer, pBackBuffer);
		}
	}

	//Return a leaf node with the rest of nodes
	return AddBuildLeafNode(positions, numPositions, indexPositionVector);

}

C_BSPPointTreeNode* C_BSPPointTree::AddBuildBranchNode(S32 planeIndex, C_BSPPointTreeNode* pFrontNode, C_BSPPointTreeNode* pBackNode)
{
	C_BSPPointTreeNode*	pNode = new C_BSPPointTreeNode(m_NodeVector.Size(), planeIndex, pFrontNode, pBackNode);
	m_NumNodes++;
	return pNode;
}

C_BSPPointTreeNode* C_BSPPointTree::AddBuildLeafNode(Maths::HVec3* positions, U32 numPositions, Btl::Vector<U32>& indexPositionVector)
{
	C_BSPPointTreeNode*	pNode = new C_BSPPointTreeNode(m_PositionVector.Size(), indexPositionVector.Size());
	for (S32 i = 0; i < indexPositionVector.Size(); i++)
		m_PositionVector.PushBack(C_IndexPosition(indexPositionVector[i], positions[indexPositionVector[i]]));
	m_NumNodes++;
	return pNode;
}

S32 C_BSPPointTree::AddPlane( Maths::C_Plane plane )
{
	S32 index = m_PlaneVector.Size();
	m_PlaneVector.PushBack(plane);
	return index;
}

bool C_BSPPointTree::Export(Sys::I_File* pFile ) const
{
	Sys::BinarySerialiserAutoEndian ar( *pFile );

	//Serialize Plane Vector
	ar << m_PlaneVector.Size();
	for (S32 i = 0; i < m_PlaneVector.Size(); i++)
		ar << m_PlaneVector[i];

	//Serialize Position Vector
	ar << m_PositionVector.Size();
	for (S32 i = 0; i < m_PositionVector.Size(); i++)
	{
		ar << m_PositionVector[i].m_Index;
		ar << m_PositionVector[i].m_Position;
	}

	//Serialize Nodes in preorder
	ar << m_NumNodes;

	if (m_pRootNode)
		WriteNode(m_pRootNode, ar);
	else
		ar << S32(0);

	return true;
}

void C_BSPPointTree::Read( Sys::I_File& file )
{
	Sys::BinaryDeserialiserAutoEndian ar( file );

	//Deserialize Plane Vector
	S32 planeVectorSize;
	ar >> planeVectorSize;
	m_PlaneVector.Resize(planeVectorSize);
	for (S32 i = 0; i < planeVectorSize; i++)
		ar >> m_PlaneVector[i];

	//Deserialize Position Vector
	S32 positionVectorSize;
	ar >> positionVectorSize;
	m_PositionVector.Resize(positionVectorSize);
	for (S32 i = 0; i < positionVectorSize; i++)
	{
		ar >> m_PositionVector[i].m_Index;
		ar >> m_PositionVector[i].m_Position;
	}

	//Get NumNodes
	S32 numNodes;
	ar >> numNodes;
	m_NodeVector.Resize(numNodes);
	m_NumNodes = 0;

	//Read
	m_pRootNode = ReadNode(ar);
}

C_BSPPointTreeNode* C_BSPPointTree::AddUnserializeNode()
{
	C_BSPPointTreeNode* pNode = &m_NodeVector[m_NumNodes];
	m_NumNodes++;
	return pNode;
}

//Read Node
template<typename Archive>
C_BSPPointTreeNode* C_BSPPointTree::ReadNode(Archive& ar)
{
	S32 index;
	ar >> index;
	switch (index)
	{
	case -1:
		{
			C_BSPPointTreeNode* pNode = AddUnserializeNode();
			//Leaf
			pNode->m_Index = -1;
			ar >> pNode->m_FirstPositionIndex >> pNode->m_PositionIndexCount;

			return pNode;
		}
		break;
	case 1:
		{
			C_BSPPointTreeNode* pNode = AddUnserializeNode();
			//Branch
			pNode->m_Index = 1;
			ar >> pNode->m_PlaneIndex;
			//Read front and back
			pNode->m_pFrontNode = ReadNode(ar);
			pNode->m_pBackNode = ReadNode(ar);

			return pNode;
		}
		break;
	case 0:
		return NULL; //Null node
		break;
	}

	return NULL;
}
template<typename Archive>
void C_BSPPointTree::WriteNode(C_BSPPointTreeNode* pNode, Archive& ar) const
{
	//(0 -> No Node, -1 -> Lead, 1 ->Branch
	if (pNode->m_Index == -1)
	{
		ar << -1;
		ar << pNode->m_FirstPositionIndex << pNode->m_PositionIndexCount;
	}
	else
	{
		ar << 1;
		ar << pNode->m_PlaneIndex;
		if (pNode->m_pFrontNode)
			WriteNode(pNode->m_pFrontNode, ar);
		else
			ar << 0;

		if (pNode->m_pBackNode)
			WriteNode(pNode->m_pBackNode, ar);
		else
			ar << 0;
	}
}
} // LuxSampleList

