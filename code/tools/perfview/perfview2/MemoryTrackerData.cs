using System;
using System.Collections.Generic;
using System.Text;

namespace ProfilingApplication
{
    [Serializable]
	public struct AllocationInfo
	{
		public int iContextId;
		public UInt32 iAddress;
		public UInt32 cSize;
		public int iWaste;
		public int iType;
		public int iTag;
		public List<int> callstack;
	};

    [Serializable]
	public struct TrackerNodes
	{
		public string sName;
		public int iParent;
	};

    [Serializable]
	public struct ModuleInfo
	{
		public string sName;
		public int iBaseAddress;
		public int cSize;
		public Byte[] signature;
	};

    [Serializable]
	public class MemoryTrackerData : PerfViewFile
	{
		// data
		public List<AllocationInfo> m_Allocations;
		public List<TrackerNodes> m_Nodes;
		public List<ModuleInfo> m_Modules;

		public UInt32 m_iMemoryStart, m_iMemoryEnd;
		public int m_iPlatform; // 0 = xbox, 1 = ps3, 2 = other

		/// calculate memory start and end
		void CalcMemoryStartAndEnd()
		{
			if (m_Allocations.Count > 0)
			{
				m_iMemoryStart = m_iMemoryEnd = m_Allocations[0].iAddress;
				for (int i=1; i<m_Allocations.Count; i++)
				{
					if (m_Allocations[i].iAddress < m_iMemoryStart)
						m_iMemoryStart = m_Allocations[i].iAddress;
					UInt32 iEnd = m_Allocations[i].iAddress + m_Allocations[i].cSize;
					if (iEnd > m_iMemoryEnd)
						m_iMemoryEnd = iEnd;
				}

				// round to nearest 64K
				m_iMemoryStart = m_iMemoryStart & 0xffff0000;
				m_iMemoryEnd = (m_iMemoryEnd + 0xffff) & 0xffff0000;
			}
			else
			{
				m_iMemoryStart = m_iMemoryEnd = 0;
			}
		}

        public override void InitialiseUnserialisables()
		{
		}

        private static int AllocationSortByAddress(AllocationInfo a, AllocationInfo b)
        {
            return (int)a.iAddress - (int)b.iAddress;
        }

		public void ReadAllocation(Bizarre.Artery.MessageReader msgReader)
		{
			int nAllocations;
			msgReader.Read(out nAllocations);

			// read allocation data
			for (int i=0; i<nAllocations; i++)
			{
				AllocationInfo info;

				if (!msgReader.Read(out info.iContextId))
					break;
				msgReader.Read(out info.cSize);
				msgReader.Read(out info.iAddress);
				msgReader.Read(out info.iWaste);
				msgReader.Read(out info.iType);
				msgReader.Read(out info.iTag);

				int cCallstack;
				msgReader.Read(out cCallstack);
				info.callstack = new List<int>(cCallstack);
				for (int j=0; j<cCallstack; j++)
				{
					int iAddress;
					msgReader.Read(out iAddress);
					info.callstack.Add(iAddress);
				}

				if (info.iAddress > 0)
					m_Allocations.Add(info);
            }
		}

		public void ReadFinish(Bizarre.Artery.MessageReader msgReader)
		{
			m_Allocations.Sort(AllocationSortByAddress);

			CalcMemoryStartAndEnd();

			m_bSaved = false;
		}

        public void ReadHeader(Bizarre.Artery.MessageReader msgReader)
        {
			// read header
			int iVersion;
			msgReader.Read(out iVersion);
			msgReader.Read(out m_iPlatform);

			if (m_iPlatform == 0) // xenon
			{
				int nModules;
				msgReader.Read(out nModules);

				m_Modules = new List<ModuleInfo>(nModules);

				for (int iModule=0; iModule<nModules; iModule++)
				{
					ModuleInfo info;
					msgReader.Read(out info.iBaseAddress);
					msgReader.Read(out info.cSize);
					msgReader.Read(out info.sName);
					int cSignature;
					msgReader.Read(out cSignature);
					info.signature = new Byte[cSignature];
					msgReader.Read(info.signature, cSignature);
					m_Modules.Add(info);
				}
			}

			int nNodes;
			msgReader.Read(out nNodes);

			m_Nodes = new List<TrackerNodes>(nNodes);

			// read context node strings
			for (int i=0; i < nNodes; i++)
			{
				TrackerNodes node;
				msgReader.Read(out node.sName);
				msgReader.Read(out node.iParent);

				m_Nodes.Add(node);
			}

			// read some extra data that we're not interested in
			int dummy;
			for (int i=0; i<nNodes; i++)
			{
				msgReader.Read(out dummy);
			}

			m_Allocations = new List<AllocationInfo>();
		}
	}
}
