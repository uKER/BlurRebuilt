using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using System.Collections.Specialized;
using WeifenLuo.WinFormsUI.Docking;
using SynapticEffect.Forms;

namespace ProfilingApplication
{
	public partial class HierarchyView : ToolWindow
	{
		public HierarchyView()
		{
			InitializeComponent();

			m_Nodes = new List<AugmentedNode>();

			InitialiseOnce();
		}

		public void SetUserSettings(UserSettings settings)
		{
			m_Settings = settings;

			panel2.Columns[0].Width = m_Settings.m_iDimensions[(int)DimensionValues.Hierarchy_Name] ;
			panel2.Columns[1].Width = m_Settings.m_iDimensions[(int)DimensionValues.Hierarchy_Hits] ;
			panel2.Columns[2].Width = m_Settings.m_iDimensions[(int)DimensionValues.Hierarchy_Time] ;
			panel2.Columns[3].Width = m_Settings.m_iDimensions[(int)DimensionValues.Hierarchy_Ratio];
		}

		public void SetParameters(MainApp mainApp, CaptureView parent, CaptureData capture, UserSettings settings)
		{
			m_Capture = capture;
			m_MainApp = mainApp;
			m_Parent = parent;
			m_Settings = settings;
			if (m_Parent != null)
				m_iCurrentCore = m_Parent.GetSelectedCore();
			else
				m_iCurrentCore = 0;

			InitialiseWithCaptureData();
		}

		public void SelectString(UInt32 iString)
		{
			panel2.UnselectAllNodes();

			if (iString != 0xffffffff && iString != 0xfffffffe)
			{
				TreeListNode t = m_StringMap[m_Capture.m_StringList[(int)iString]];
				t.Selected = true;
				t.Focused = true;
				TreeListNode iT = t;
				while (t != null)
				{
					t.Expand();
					t = (TreeListNode)t.ParentNode();
				}
				panel2.AdjustScrollbars();
				panel2.CenterViewOnItem(iT);
			}
            panel2.Invalidate();
		}

		void AddTimingViewHeader(string s, int width, System.Windows.Forms.HorizontalAlignment align)
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

			panel2.Columns.Add(header);
		}

		void InitialiseOnce()
		{
			// colour icon, name, hits, time, percentage, (total hits, total time, total percentage?)

			panel2.ColumnSortColor = System.Drawing.Color.Gainsboro;
			panel2.ColumnTrackColor = System.Drawing.Color.WhiteSmoke;
			panel2.ColumnTracking = true;
			panel2.Dock = System.Windows.Forms.DockStyle.Fill;
			panel2.GridLineColor = System.Drawing.Color.WhiteSmoke;
			panel2.HeaderMenu = null;
			panel2.ItemHeight = 20;
			panel2.LabelEdit = false;
			panel2.Name = "panel2";
			panel2.BackColor = System.Drawing.SystemColors.Window;

			AddTimingViewHeader("Name", 10, System.Windows.Forms.HorizontalAlignment.Left);
			AddTimingViewHeader("Hits", 10, System.Windows.Forms.HorizontalAlignment.Right);
			AddTimingViewHeader("Time", 10, System.Windows.Forms.HorizontalAlignment.Right);
			AddTimingViewHeader("Ratio", 10, System.Windows.Forms.HorizontalAlignment.Right);

			InitialiseImageList();
			panel2.SmallImageList = m_ImageList;
		}

		private static int NodeComparisonFunction(AugmentedNode a, AugmentedNode b)
		{
			return string.Compare(a.sName, b.sName);
		}

		private string GetParentNodeName(string s)
		{
			int x = s.LastIndexOf('.');
			if (x == -1)
				return "[$root]";
			else
				return s.Substring(0, x);
		}

		private void LinkNode(AugmentedNode node)
		{
			if (node.sName == "[$root]")
				return;

			AugmentedNode parentNode;
			string sParentName = GetParentNodeName(node.sName);

			if (m_StringMap.ContainsKey(sParentName))
			{
				parentNode = m_StringMap[sParentName];
				parentNode.Expand();
				parentNode.Nodes.Add(node);
				parentNode.iHits += node.iHits;
				parentNode.iTime += node.iTime;
				return;
			}

			parentNode = new AugmentedNode();
			parentNode.iTime = node.iTime;
			parentNode.iHits = node.iHits;
			parentNode.iColour = -1;
			parentNode.sName = sParentName;
			parentNode.Nodes.Add(node);
			parentNode.Expand();
			m_Nodes.Add(parentNode);
			m_StringMap[parentNode.sName] = parentNode;
			if (sParentName == "[$root]")
				panel2.Nodes.Add(parentNode);
			else
				LinkNode(parentNode);
		}

		int RecursiveGetNodeColour(AugmentedNode node, ref bool bInherited )
		{
			string t = node.sName;
			if (m_Settings.m_ColourSettings.ContainsKey(t))
				return m_Settings.m_ColourSettings[t];

			if (node.sName == "[$root]")
				return 0;


			string sParentNodeName = GetParentNodeName(node.sName);

			if (!m_StringMap.ContainsKey(sParentNodeName))
			{
				return -1; // shouldn't happen
			}

			bInherited = true;
			return RecursiveGetNodeColour(m_StringMap[sParentNodeName], ref bInherited);
		}

		static int AlphabeticalSort(AugmentedNode a, AugmentedNode b)
		{
			return a.sName.CompareTo(b.sName);
		}

		void InitialiseWithCaptureData()
		{
			m_Nodes.Clear();
			panel2.Nodes.Clear();

			if (m_Capture == null)
				return;

			m_StringMap = new Dictionary<string,AugmentedNode>();

			AugmentedNode total = new AugmentedNode();
			total.iTime = 0;
			total.iHits = 0;
			total.iColour = -1;
			int iTotalTime = (int)m_Capture.m_iTotalFrameTime;
			for (int i = 0; i < m_Capture.m_StringList.Count; i++)
			{
				AugmentedNode empty = new AugmentedNode();
				empty.iHits = 0;
				empty.iTime = 0;
				empty.iColour = 1;
				empty.sName = m_Capture.m_StringList[i];
				m_Nodes.Add(empty);
			}

			foreach(List<Event> list in m_Capture.m_ThreadEventList)
			{
				UInt64 iStartTime = 0;
				UInt32 iStartString = 0xffffffff;
				foreach(Event ev in list)
				{
					// deal with thread start/stop
					if (ev.iString == 0xffffffff && ev.iEnd == 0)
					{
						iStartTime = ev.iTime;
						continue;
					}

					// count time
					if (iStartString != 0xffffffff && iStartString != 0xfffffffe)
					{
						int interval = (int)(ev.iTime - iStartTime);
						m_Nodes[(int)iStartString].iTime += interval;
					}

					if (ev.iString == 0xffffffff)
						continue;

					// count hits
					if (ev.iEnd == 0)
					{
						m_Nodes[(int)ev.iString].iHits++;
						total.iHits++;
					}

					iStartString = ev.iString;
					iStartTime = ev.iTime;
				}
			}

			// remove empty nodes
			for (int i = 0; i < m_Nodes.Count; i++)
			{
				if (m_Nodes[i].iHits == 0 && m_Nodes[i].iTime == 0)
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
				AugmentedNode node = m_Nodes[i];
				node.iColour = 0;
				node.Text = m_Nodes[i].sName;
				node.SubItems.Add(m_Nodes[i].iHits.ToString());   // hits
				node.SubItems.Add(m_Nodes[i].iTime.ToString("N0"));   // time
				float fRatio = (float)m_Nodes[i].iTime / (float)iTotalTime;
				node.SubItems.Add(fRatio.ToString("P0"));   // ratio
			}

			if (panel2.Nodes.Count > 0)
				panel2.Nodes[0].Text = "Root";

			RecalculateNodeImages();
		}

		void InitialiseImageList()
		{
			string[] resources = Assembly.GetExecutingAssembly().GetManifestResourceNames();

			System.Windows.Forms.ImageList il = new System.Windows.Forms.ImageList();
			m_ImageList = il;

			List<string> iconNames = new List<string>();
			iconNames.Add("black");
			iconNames.Add("black_tick");
			iconNames.Add("blue");
			iconNames.Add("blue_tick");
			iconNames.Add("green");
			iconNames.Add("green_tick");
			iconNames.Add("orange");
			iconNames.Add("orange_tick");
			iconNames.Add("red");
			iconNames.Add("red_tick");
			iconNames.Add("silver");
			iconNames.Add("silver_tick");

			foreach (string x in iconNames)
			{
				System.IO.Stream strm = Assembly.GetExecutingAssembly().GetManifestResourceStream("ProfilingApplication.Resources." + x + ".PNG");
				il.Images.Add((Image)new Bitmap(strm));
			}
		}

		void RecalculateNodeImages()
		{
			int[] lookuptable = { 1, 3, 5, 7, 9, 11 };

			for (int i = 0; i < m_Nodes.Count; i++)
			{
				bool bInherited = false;
				int iCol = RecursiveGetNodeColour(m_Nodes[i], ref bInherited);
				if (iCol == -1)
					break; // shouldn't happen
				m_Nodes[i].ImageIndex = lookuptable[iCol] - (bInherited?1:0);
				m_Nodes[i].iInheritedColour = iCol;
			}
		}

		public List<AugmentedNode> m_Nodes;
		private System.Windows.Forms.ImageList m_ImageList;
		private CaptureView m_Parent;
		private CaptureData m_Capture;
		private int m_iCurrentCore;
		private UserSettings m_Settings;
		private MainApp m_MainApp;
		private Dictionary<string, AugmentedNode> m_StringMap;

		private void SetColourToSelectedItems(int iColour)
		{
			foreach (SynapticEffect.Forms.TreeListNode x in panel2.SelectedNodes)
			{
				AugmentedNode node = (AugmentedNode)x;
				if (node.sName == null)
					return;
				if (iColour != -1)
					m_Settings.m_ColourSettings[node.sName] = iColour;
				else
					m_Settings.m_ColourSettings.Remove(node.sName);

				node.iColour = iColour;
			}
			RecalculateNodeImages();
			panel2.Invalidate(true);
			m_MainApp.UpdateBarColoursOnAllCaptureView();
		}

		private void blackToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetColourToSelectedItems(0);
		}

		private void blueToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetColourToSelectedItems(1);
		}

		private void orangeToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetColourToSelectedItems(3);
		}

		private void greenToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetColourToSelectedItems(2);
		}

		private void redToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetColourToSelectedItems(4);
		}

		private void silverToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetColourToSelectedItems(5);
		}

		private void notSetToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetColourToSelectedItems(-1);
		}

		private void HierarchyView_FormClosing(object sender, FormClosingEventArgs e)
		{
		}

		public void SaveSettings()
		{
			// save settings
			m_Settings.m_iDimensions[(int)DimensionValues.Hierarchy_Name]  = panel2.Columns[0].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Hierarchy_Hits]  = panel2.Columns[1].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Hierarchy_Time]  = panel2.Columns[2].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Hierarchy_Ratio] = panel2.Columns[3].Width;
		}
	}

	public class AugmentedNode : SynapticEffect.Forms.TreeListNode
	{
		public int iHits;
		public int iTime;
		public int iColour;
		public int iInheritedColour;
		public string sName;
	};

}
