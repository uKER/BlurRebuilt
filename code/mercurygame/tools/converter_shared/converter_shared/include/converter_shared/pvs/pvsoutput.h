#ifndef CONVERTER_SHARED_PVS_OUTPUT
#define CONVERTER_SHARED_PVS_OUTPUT

#ifdef _MSC_VER
#pragma once
#endif

#include "btl\containers\bitvector.h"

namespace Pvs
{

struct DatabaseFileHeader
{
	DatabaseFileHeader() : iVersion(-1) {} // set version to -1 if not loaded

	int iVersion; // output file version number (1)
	int nPrimitives;  // number of individual primitives that have their own bitfield
	int iBitfieldSizeInBytes; // size of each bitfield in bytes
};

template<typename Stream>
void Deserialise(Stream& in, DatabaseFileHeader& data)
{
	in >> data.iVersion;
	in >> data.nPrimitives;
	in >> data.iBitfieldSizeInBytes;
}

/////////////////////////////////////////////////

struct DatabaseFilePrimitive
{
	int iNode; // aftk index of the scene node
	int iTri; // index of the polygon within the scene node
};

template<typename Stream>
void Deserialise(Stream& in, DatabaseFilePrimitive& data)
{
	in >> data.iNode;
	in >> data.iTri;
}

////////////////////////////////////////////////////////////////////

class PvsOutputData
{
public:
	PvsOutputData() : readBitfields(true) {}

	DatabaseFileHeader header;
	Btl::Vector<DatabaseFilePrimitive> primitives;
	Btl::Vector<Btl::BitVector> primitiveData;

	int GetPvsIndex(int nodeIndex, int polygonIndex, const char* pDebugName) const;

	template<typename Stream>
	friend void Deserialise(Stream& in, PvsOutputData& data);

	bool readBitfields;

	void Clear()
	{
		m_map.Clear();
		primitives.Destroy();
		primitiveData.Destroy();
	}

private:
	typedef Btl::Map<Btl::Pair<int, int>, int> IndexMap;
	IndexMap m_map;
};

inline int PvsOutputData::GetPvsIndex(int nodeIndex, int polygonIndex, const char* pDebugName) const
{
	IndexMap::ConstIterator result = m_map.Find(Btl::MakePair(nodeIndex, polygonIndex));
	if (result != m_map.End())
		return result->GetValue();

	// could be an instance or something, in which case the polygon index would have been set to -1
	result = m_map.Find(Btl::MakePair(nodeIndex, -1));
	if (result != m_map.End())
		return result->GetValue();
	else if (primitiveData.Size())
	{
		// here when this primitive wasn't found in the PVS database
		// this won't break the export but it will make the object 'always visible' which is bad for performance
		static S32 errorCount = 0;
		if (errorCount <= 50)
		{
			LOGERROR( Debug::GetLogger("graphics.renderingdata"), "*** Couldn't find node ID %d, %d (%s) in PVS database ***\n", nodeIndex, polygonIndex, pDebugName ? pDebugName : "Null" );

			if(errorCount == 50)
			{
				LOGERROR( Debug::GetLogger("graphics.renderingdata"), "Further PVS errors suppressed\n");
			}
			errorCount++;
		}
		return header.nPrimitives;
	}
	else
		return -1; // data not loaded
}

template<typename Stream>
void Deserialise(Stream& in, PvsOutputData& data)
{
	in >> data.header;

	data.primitives.Resize(data.header.nPrimitives);

	for (int i = 0; i < data.primitives.Size(); i++)
		in >> data.primitives[i];

	data.m_map.Clear();

	for (int i = 0; i < data.primitives.Size(); i++)
		data.m_map.Insert(PvsOutputData::IndexMap::DataType(Btl::MakePair(data.primitives[i].iNode, data.primitives[i].iTri), i));

	if (data.readBitfields)
	{
		data.primitiveData.Resize(data.header.nPrimitives + 1);

		for (int i = 0; i < data.primitives.Size(); i++)
		{
			data.primitiveData[i].Resize(data.header.iBitfieldSizeInBytes * 8);
			in.Read(data.primitiveData[i].Data(), data.header.iBitfieldSizeInBytes);
		}

		// add an entry to the end with all cells visible
		data.primitiveData.Back().Resize(data.header.iBitfieldSizeInBytes * 8);
		data.primitiveData.Back().SetBits();
	}
}

}

#endif