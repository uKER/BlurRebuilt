using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;

namespace ProfilingApplication
{
	public partial class MemoryNodeView : ToolWindow
	{
		Dictionary<string,AugmentedMemoryNode> m_StringMap;
		MemoryFragmentationView m_Parent;
		MemoryTrackerData m_Capture;
		UserSettings m_Settings;
		MainApp m_MainApp;
		List<AugmentedMemoryNode> m_Nodes;
		int m_iFilterTag;
		int m_iSelectedNode;

		public MemoryNodeView()
		{
			m_Nodes = new List<AugmentedMemoryNode>();
			m_iSelectedNode = -1;

			InitializeComponent();

			InitialiseOnce();
		}

		void AddHeader(string s, int width, System.Windows.Forms.HorizontalAlignment align)
		{
			SynapticEffect.Forms.ToggleColumnHeader header = new SynapticEffect.Forms.ToggleColumnHeader();

			header.Hovered = false;
			header.Image = null;
			header.Index = 0;
			header.Pressed = false;
			header.ScaleStyle = SynapticEffect.Forms.ColumnScaleStyle.Slide;
			header.Selected = false;
			header.Text = s;
			header.TextAlign = align;
			header.Visible = true;
			header.Width = width;

			treeList.Columns.Add(header);
		}

		public void SetUserSettings(UserSettings settings)
		{
			m_Settings = settings;

			treeList.Columns[0].Width = m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Name];
			treeList.Columns[1].Width = m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Allocs];
			treeList.Columns[2].Width = m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Memory];
			treeList.Columns[3].Width = m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Wasted];
			treeList.Columns[4].Width = m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Percent];			
		}

		public void SaveSettings()
		{
			// save settings
			m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Name]    = treeList.Columns[0].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Allocs]  = treeList.Columns[1].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Memory]  = treeList.Columns[2].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Wasted]  = treeList.Columns[4].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.MemNode_Percent] = treeList.Columns[5].Width;
		}

		public void SetParameters(MainApp mainApp, MemoryFragmentationView parent, MemoryTrackerData capture, UserSettings settings, int iFilterTag)
		{
			m_Capture = capture;
			m_MainApp = mainApp;
			m_Parent = parent;
			m_Settings = settings;
			m_iFilterTag = iFilterTag;

			InitialiseWithCaptureData();
			Invalidate(true);
		}

		void InitialiseOnce()
		{
			treeList.ColumnSortColor = System.Drawing.Color.Gainsboro;
			treeList.ColumnTrackColor = System.Drawing.Color.WhiteSmoke;
			treeList.ColumnTracking = true;
			treeList.Dock = System.Windows.Forms.DockStyle.Fill;
			treeList.GridLineColor = System.Drawing.Color.WhiteSmoke;
			treeList.HeaderMenu = null;
			treeList.ItemHeight = 20;
			treeList.LabelEdit = false;
			treeList.Name = "treeList";
			treeList.BackColor = System.Drawing.SystemColors.Window;

			AddHeader("Name", 250, System.Windows.Forms.HorizontalAlignment.Left);
			AddHeader("Allocs", 80, System.Windows.Forms.HorizontalAlignment.Right);
			AddHeader("Memory", 80, System.Windows.Forms.HorizontalAlignment.Right);
			AddHeader("Wasted", 80, System.Windows.Forms.HorizontalAlignment.Right);
			AddHeader("Percent", 80, System.Windows.Forms.HorizontalAlignment.Right);
		}

		private string GetParentNodeName(string s)
		{
			int x = s.LastIndexOf('.');
			if (x == -1)
				return "[$root]";
			else
				return s.Substring(0, x);
		}

		private void UpdateChildrenAllocations(AugmentedMemoryNode node, int iAllocations, int iMemory, int iWasted)
		{
			while(true)
			{
				node.iAllocations += iAllocations;
				node.iMemory += iMemory;
				node.iWasted += iWasted;

				if (node.sName == "[$root]")
					return;

				string sParentName = GetParentNodeName(node.sName);

				node = m_StringMap[sParentName];
			}
		}

		private void LinkNode(AugmentedMemoryNode node)
		{
			if (node.sName == "[$root]")
				return;

			AugmentedMemoryNode parentNode;
			string sParentName = GetParentNodeName(node.sName);

			if (m_StringMap.ContainsKey(sParentName))
			{
				parentNode = m_StringMap[sParentName];
				parentNode.Expand();
				parentNode.Nodes.Add(node);

				UpdateChildrenAllocations(parentNode, node.iAllocations, node.iMemory, node.iWasted);
				return;
			}

			parentNode = new AugmentedMemoryNode();
			parentNode.iAllocations = node.iAllocations;
			parentNode.iMemory = node.iMemory;
			parentNode.iWasted = node.iWasted;
			parentNode.sName = sParentName;
			parentNode.iNode = -1;
			parentNode.Nodes.Add(node);
			parentNode.Expand();
			m_Nodes.Add(parentNode);
			m_StringMap[parentNode.sName] = parentNode;
			if (sParentName == "[$root]")
				treeList.Nodes.Add(parentNode);
			else
				LinkNode(parentNode);
		}

		static int AlphabeticalSort(AugmentedMemoryNode a, AugmentedMemoryNode b)
		{
			return a.sName.CompareTo(b.sName);
		}

		static public string IntToMemoryString(int dwSize)
		{
			if (dwSize > 236*1024)
			{
				return string.Format("{0:F1}M", ((float)dwSize)/(1024*1024));
			}
			else if (dwSize > 128)
			{
				return string.Format("{0:F1}K", ((float)dwSize)/1024);
			}
			else
			{
				return dwSize.ToString() + "b";
			}
		}

		void InitialiseWithCaptureData()
		{
			m_Nodes.Clear();
			treeList.Nodes.Clear();

			if (m_Capture == null)
			{
				treeList.Nodes.Clear();
				return;
			}

			m_StringMap = new Dictionary<string,AugmentedMemoryNode>();

			// create a listnode for each memory tracker id
			AugmentedMemoryNode total = new AugmentedMemoryNode();
			total.iAllocations = 0;
			total.iMemory = 0;
			total.iWasted = 0;
			for (int i = 0; i < m_Capture.m_Nodes.Count; i++)
			{
				AugmentedMemoryNode empty = new AugmentedMemoryNode();
				empty.iAllocations = total.iMemory = total.iWasted = 0;
				empty.sName = m_Capture.m_Nodes[i].sName;
				empty.iNode = i;
				m_Nodes.Add(empty);
			}

			// add each allocation into the respective tracker id
			for (int i=0; i<m_Capture.m_Allocations.Count; i++)
			{
				AllocationInfo info = m_Capture.m_Allocations[i];
				if (m_iFilterTag != -1 && info.iTag != m_iFilterTag)
					continue;
				m_Nodes[info.iContextId].iAllocations++;
				total.iAllocations++;
				m_Nodes[info.iContextId].iMemory += (int)info.cSize;
				total.iMemory += (int)info.cSize;
				m_Nodes[info.iContextId].iWasted += info.iWaste;
				total.iWasted += info.iWaste;
			}

			// remove empty nodes, add all others into the stringmap
			for (int i = 0; i < m_Nodes.Count; i++)
			{
				if (m_Nodes[i].iAllocations == 0)
				{
					m_Nodes.RemoveAt(i);
					i--;
				}
				else
					m_StringMap[m_Nodes[i].sName] = m_Nodes[i];
			}

			// sort the leftover nodes
			m_Nodes.Sort(AlphabeticalSort);

			// add in extra parent nodes
			int nNodes = m_Nodes.Count;
			for (int i = 0; i < nNodes; i++)
				LinkNode(m_Nodes[i]);

			// display collected hit info in hierarchy view
			for (int i = 0; i < m_Nodes.Count; i++)
			{
				AugmentedMemoryNode node = m_Nodes[i];
				node.Text = m_Nodes[i].sName;
				node.SubItems.Add(m_Nodes[i].iAllocations.ToString());
				node.SubItems.Add(IntToMemoryString(m_Nodes[i].iMemory));
				node.SubItems.Add(IntToMemoryString(m_Nodes[i].iWasted));
				float fRatio = (float)m_Nodes[i].iMemory / (float)total.iMemory;
				node.SubItems.Add(fRatio.ToString("P0"));
			}

			if (treeList.Nodes.Count > 0)
				treeList.Nodes[0].Text = "root";

		}

		private void treeList_OnSelectionChangedEvent(object sender, SynapticEffect.Forms.TreeListNode node)
		{
			int iNewNode = (((AugmentedMemoryNode)node).iNode);
			if (m_iSelectedNode != iNewNode)
			{
				m_Parent.SetSelectedTrackerId(iNewNode);
			}
		}

		private void treeList_MouseDoubleClick(object sender, MouseEventArgs e)
		{
			m_Parent.DisplaySelectedTrackerIdAllocsInAllocView();
		}
	}

	public class AugmentedMemoryNode : SynapticEffect.Forms.TreeListNode
	{
		public int iAllocations;
		public int iMemory;
		public int iWasted;
		public int iNode;
		public string sName;
	};

}
