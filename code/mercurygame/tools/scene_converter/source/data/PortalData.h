#ifndef APP_PORTAL_DATA_H
#define APP_PORTAL_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>
#include <Portal/Builder.h>
#include <Portal/System.h>

/// Portal data object.
class C_DataPortalData : public Tools::ProcessGraph::C_Data
{
public:
	/// Default c'tor.
	explicit C_DataPortalData();

	/// Export the portal data to a file.
	bool C_DataPortalData::Export(Sys::I_File* pFile, const Util::E_Platform& rkPlatform) const;
	/// Return the portal builder object.
	Portal::C_Builder&	GetBuilder();
	// inherited from C_Data
	const Util::C_String GetName() const;

private:

	//!< Functor to count the nodes in the BSP tree.
	class NodeCounter
	{
	public:
		explicit NodeCounter() : m_iCount(0)
		{ return; }

		void operator () (RegionBSP::C_Region& /*rkRegion*/)
		{
			m_iCount++;
			return;
		}

		const U32 GetCount()
		{ return m_iCount; }

	private:
		U32 m_iCount;
	};

	//!< Node for BSP tree.
	struct Node
	{
		U32 m_iFrontIndex;
		U32 m_iBackIndex;
		U32 m_iPlaneIndex;
		U32 m_iRoomIndex;
	};


	//!< Functor to pull required data out of the regions.
	class NodeEnumerator
	{
	public:
		/// C'tor.
		explicit NodeEnumerator(const U32 iSize, Portal::C_Builder& rkBuilder) : m_Builder(rkBuilder), m_iSize(iSize), m_iNext(0), m_iNextRegion(0)
		{
			m_pElements = new Node[iSize];
			return; 
		}

		/// D'tor.
		~NodeEnumerator()
		{
			delete [] m_pElements;
		}

		/// Functor gather thingee ma jig.
		void operator() (RegionBSP::C_Region& rkRegion)
		{
			Node newNode;
			if(rkRegion.GetSplitPlane())	// Not a leaf node...
			{
				newNode.m_iBackIndex	= m_Builder.GetRoomBSPTree().GetSplitPlaneIndex(rkRegion.GetSplitPlane()->GetBack().GetSplitPlane());
				newNode.m_iFrontIndex	= m_Builder.GetRoomBSPTree().GetSplitPlaneIndex(rkRegion.GetSplitPlane()->GetFront().GetSplitPlane());
				newNode.m_iPlaneIndex	= m_Builder.GetRoomBSPTree().GetSplitPlaneIndex(rkRegion.GetSplitPlane());
				newNode.m_iRoomIndex	= rkRegion.GetRegionId();
				m_IndexMap.Insert(newNode.m_iPlaneIndex, m_iNextRegion);	// Map it!
				m_iNextRegion++;
			}
			else
			{
				newNode.m_iBackIndex	= -1;
				newNode.m_iFrontIndex	= -1;
				newNode.m_iPlaneIndex	= -1;
				newNode.m_iRoomIndex	= rkRegion.GetRegionId();
			}
			m_pElements[m_iNext] = newNode;
			++m_iNext;
		}

		/// Return a pointer to the element from it's index.
		Node* GetNodePtrAt(const U32 iIndex)
		{
			return &m_pElements[iIndex];
		}

		/// Return the mapped index.
		const U32 GetMappedIndex(const U32 iIndex)
		{	
			if(iIndex == ~0)
				return iIndex;
			
			const U32 iMappedIndex = m_IndexMap[iIndex];
			BIZ_WARNING_MSG(m_IndexMap.Find(iIndex) != m_IndexMap.End(), "Couldn't find index in the map.");
			return iMappedIndex;
		}

	private:
		Btl::Map<U32, U32> m_IndexMap;	//!< Map of split plane indices in the RegionBSP to internal array.
		Portal::C_Builder& m_Builder;	//!< Reference to the builder object.
		U32 m_iSize;					//!< Size of elements array.
		U32 m_iNext;					//!< Next index (remove).
		U32 m_iNextRegion;				//!< Next region (remove.
		Node* m_pElements;				//!< Pointer to data for the nodes.
	};

	mutable Portal::C_Builder		m_PortalBuilder;			//!< Portal builder object.
};


#endif // APP_PORTAL_DATA_H
