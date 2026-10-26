using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using SynapticEffect.Forms;
using WeifenLuo.WinFormsUI.Docking;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;

namespace ProfilingApplication
{
    public partial class CaptureView : DockContent
	{
		public CaptureView(CaptureData capture, MainApp parent, UserSettings settings, HierarchyView hierarchyView)
		{
			m_Capture = capture;
			m_UserSettings = settings;
			m_iThreadStartEvent = new List<int>();
			m_iThreadEndEvent = new List<int>();
            m_Parent = parent;

			InitializeComponent();

			int x = m_UserSettings.m_iDimensions[(int)DimensionValues.Capture_HSplit];
			if (x < 2) x = 2;
			splitContainer1.SplitterDistance = x;
			x = m_UserSettings.m_iDimensions[(int)DimensionValues.Capture_VSplit];
			if (x < 2) x = 2;
			splitContainer2.SplitterDistance = x;

			m_iCurrentCore = 0;
            m_iCurrentTime = 0;
            m_iSelectedBar = -1;
			m_bSelectedArea = false;
			InitialiseThreadViewOnce();
			InitialiseThreadView();

			m_iCurrentThreadIndex = 0;
            m_HierarchyView = hierarchyView;
            m_HierarchyView.SetParameters(parent, this, m_Capture, settings);
            InitialiseTimingViewOnce();
            InitialiseTimingView();

            panel2.SetCaptureData(capture);
            panel2.UpdateBarColours(settings);
            panel2.SetParent(this);

            if (m_Capture.m_sOriginalFilename == null)
            {
                string currentTime = "Capture__" + DateTime.Now.ToString("dd_MM_yyyy__hh_mm_ss"); 

                this.Name = currentTime;
                this.TabText = currentTime;
                this.Text = currentTime;

                m_Capture.m_sOriginalFilename = currentTime;
            }
            else
            {
                string name = Path.GetFileNameWithoutExtension(m_Capture.m_sOriginalFilename);
                this.Name = name;
                this.TabText = name;
                this.Text = name;
            }
		}

		void InitialiseThreadViewOnce()
		{
			listView2.Columns[0].Width = m_UserSettings.m_iDimensions[(int)DimensionValues.Thread_Name];
			listView2.Columns[1].Width = m_UserSettings.m_iDimensions[(int)DimensionValues.Thread_Start];
			listView2.Columns[2].Width = m_UserSettings.m_iDimensions[(int)DimensionValues.Thread_End];
			listView2.Columns[3].Width = m_UserSettings.m_iDimensions[(int)DimensionValues.Thread_Duration];
		}

		/// <summary>
		/// Read information from capturedata and display into thread-view
		/// </summary>
		void InitialiseThreadView()
		{
            List<ListViewItem> items = new List<ListViewItem>();

            listView2.Items.Clear();

            List<CoreThreadInfo> events = m_Capture.m_CoreEventList[m_iCurrentCore];
            for (int iEvent = 0; iEvent < events.Count; iEvent++)
            {
                CoreThreadInfo ev = events[iEvent];

                UInt64 duration = ev.iEnd - ev.iStart;
                string[] row = { m_Capture.m_ThreadList[ev.iThread].sName, ev.iStart.ToString(), ev.iEnd.ToString(), duration.ToString() };

                items.Add(new ListViewItem(row));
            }

            listView2.Items.AddRange(items.ToArray());
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

            treeListView1.Columns.Add(header);
        }

        void InitialiseTimingViewOnce()
        {
            treeListView1.ColumnSortColor = System.Drawing.Color.Gainsboro;
            treeListView1.ColumnTrackColor = System.Drawing.Color.WhiteSmoke;
            treeListView1.ColumnTracking = true;
            treeListView1.Dock = System.Windows.Forms.DockStyle.Fill;
            treeListView1.GridLineColor = System.Drawing.Color.WhiteSmoke;
            treeListView1.HeaderMenu = null;
            treeListView1.ItemHeight = 20;
            treeListView1.ItemMenu = null;
            treeListView1.LabelEdit = false;
            treeListView1.Name = "treeListView1";
            treeListView1.BackColor = System.Drawing.SystemColors.Window;

            AddTimingViewHeader("Name", m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_Name], System.Windows.Forms.HorizontalAlignment.Left);
            AddTimingViewHeader("Start", m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_Start], System.Windows.Forms.HorizontalAlignment.Right);
            AddTimingViewHeader("End", m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_End], System.Windows.Forms.HorizontalAlignment.Right);
            AddTimingViewHeader("Duration", m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_Duration], System.Windows.Forms.HorizontalAlignment.Right);
            AddTimingViewHeader("DMA Stall", m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_CacheUsed], System.Windows.Forms.HorizontalAlignment.Right);
            AddTimingViewHeader("Cache Used", m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_CacheWasted], System.Windows.Forms.HorizontalAlignment.Right);
        }

		/// <summary>
		/// Initialise the hierarchical timing view using the information from a specific thread
		/// </summary>
		void InitialiseTimingView()
		{
            List<TimingViewNodeX> nodeStack = new List<TimingViewNodeX>();
            List<TimingViewNodeX> nodes = new List<TimingViewNodeX>();
            List<Event> eventList = m_Capture.m_ThreadEventList[m_iCurrentThreadIndex];

            m_iEventIndexToNodeMap = new List<TreeListNode>();
            if (eventList.Count == 0)
            {
                treeListView1.Nodes.Clear();
                treeListView1.Invalidate();
                return;
            }

			for (int iEvent = 0; iEvent < eventList.Count; iEvent++)
			{
                Event ev = eventList[iEvent];
                if (ev.iString == 0xffffffff)
                {
                    m_iEventIndexToNodeMap.Add(null);
                    continue;
                }

                if (ev.iEnd == 0)
                {
                    // open a new node
                    TimingViewNodeX node = new TimingViewNodeX();
                    m_iEventIndexToNodeMap.Add(node);

                    if (nodeStack.Count == 0)
                        nodes.Add(node);
                    else
                        nodeStack[nodeStack.Count - 1].Nodes.Add(node);

                    nodeStack.Add(node);
                    node.Selected = false;
                    node.StateImageIndex = 0;
                    node.Tag = null;
                    node.Text = m_Capture.m_StringList[(int)ev.iString];
                    node.SubItems.Add(ev.iTime.ToString());   // start
                    node.SubItems.Add("-");   // end
                    node.SubItems.Add("?");   // duration
                    node.SubItems.Add(ev.iExtraInfo[0].iData1.ToString());    // cache usage
                    node.SubItems.Add(ev.iExtraInfo[0].iData2.ToString());      // cache wasted
                    node.iEvent = iEvent;
                }
                else
                {
                    if (nodeStack.Count > 0)
                    {
                        // close a node
                        TimingViewNodeX node = nodeStack[nodeStack.Count - 1];
                        m_iEventIndexToNodeMap.Add(node);
                        node.SubItems[1].Text = ev.iTime.ToString("N0");
                        // time
                        UInt64 start = UInt64.Parse(node.SubItems[0].Text);
                        UInt64 duration = ev.iTime - start;
                        node.SubItems[2].Text = duration.ToString("N0");
                        node.SubItems[0].Text = start.ToString("N0");
                        // cache usage
                        start = UInt64.Parse(node.SubItems[3].Text);
                        duration = ev.iExtraInfo[0].iData1 - start;
                        node.SubItems[3].Text = duration.ToString("N0");
                        // cache wasted
                        start = UInt64.Parse(node.SubItems[4].Text);
                        duration = ev.iExtraInfo[0].iData2 - start;
                        node.SubItems[4].Text = duration.ToString("N0");
                        //
                        nodeStack.RemoveAt(nodeStack.Count - 1);
                    }
                    else
                    {
                        //nodes.Clear();
                        m_iEventIndexToNodeMap.Add(null);
                    }
                }
			}

            treeListView1.Nodes.Clear();
            treeListView1.Nodes.AddRange(nodes.ToArray());
            treeListView1.Invalidate(true);
		}

        // --- functions to keep track of selection

        public int GetSelectedCore()
        {
            return m_iCurrentCore;
        }

        public int GetSelectedEvent()
        {
            return m_iSelectedEvent;
        }

        public int GetSelectedThread()
        {
            return m_iSelectedThread;
        }

        public int GetCurrentTime()
        {
            return (int)m_iCurrentTime;
        }

        public int GetSelectedBar()
        {
            return m_iSelectedBar;
        }

		public bool GetSelectedArea()
		{
			return m_bSelectedArea;
		}

		public int GetSelectedMin()
		{
			return m_iSelectedMin;
		}

		public int GetSelectedMax()
		{
			return m_iSelectedMax;
		}

        public void SetSelectedCoreAndTime(int iCore, UInt32 iTime)
        {
            if (m_iCurrentCore != iCore)
            {
                if (iCore < 0)
                    iCore = 0;
                if (iCore >= m_Capture.m_CoreList.Count)
                    iCore = m_Capture.m_CoreList.Count-1;
                m_iCurrentCore = iCore;
                InitialiseThreadView();
                InitialiseTimingView();
            }
            m_iCurrentTime = iTime;
			m_bSelectedArea = false;
            panel2.m_iFocusCore = m_iCurrentCore;
            CalculateSelectedInfo();
        }

        public void SetSelectedTime(UInt32 iTime)
        {
            m_iCurrentTime = iTime;
            CalculateSelectedInfo();
        }

		int CalculateSelectedBarFromCurrentTime()
		{
            // find selected bar
            for (int iBar=0; iBar<m_Capture.m_CoreBarsList[m_iCurrentCore].Count; iBar++)
            {
                CoreBarsInfo bar = m_Capture.m_CoreBarsList[m_iCurrentCore][iBar];

                if (bar.iStart <= m_iCurrentTime && bar.iEnd >= m_iCurrentTime)
                    return iBar;
            }

			return -1;
		}

		void UpdateSelectedBarFromSelectedEvent()
		{
			int iBar = CalculateSelectedBarFromCurrentTime();

			if (iBar != m_iSelectedBar)
			{
				m_iSelectedBar = iBar;
				panel2.Invalidate();	// TODO: optimise this by only invalidating the area of the selected bar
			}
			m_bSelectedArea = false;
		}

		void SetSelectedArea(int iMin, int iMax)
		{
			m_iSelectedMin = iMin;
			m_iSelectedMax = iMax;
			m_bSelectedArea = true;
		}

        void CalculateSelectedInfo()
        {
            bool bUpdateThreadView = false;
            bool bUpdateTimingView = false;
			bool bUpdateBarView = false;
            m_iSelectedThread = -1;

            // find selected thread
            int iInfo = 0;
            foreach(CoreThreadInfo x in m_Capture.m_CoreEventList[m_iCurrentCore])
            {
                if (x.iEnd >= m_iCurrentTime)
                {
                    if (m_iSelectedThread != x.iThread || m_iCoreThreadInfo != x.iThread)
                    {
                        m_iSelectedThread = x.iThread;
                        m_iCoreThreadInfo = iInfo;
                        bUpdateThreadView = true;
                        bUpdateTimingView = true;
                        bUpdateBarView = true;
                    }
                    break;
                }
                iInfo++;
            }

            if (m_iSelectedThread != -1)
            {
                // find selected event
                int iEvent = 0;
                for (iEvent=0; iEvent<m_Capture.m_ThreadEventList[m_iSelectedThread].Count; iEvent++)
                {
                    Event ev = m_Capture.m_ThreadEventList[m_iSelectedThread][iEvent];

                    if (ev.iTime>m_iCurrentTime)
                    {
                        if (iEvent>0)
                            iEvent--;
                        break;
                    }
                    if (ev.iTime == m_iCurrentTime)
                        break;
                }
                if (m_iSelectedEvent != iEvent)
                {
                    m_iSelectedEvent = iEvent;
                    bUpdateTimingView = true;
                }

				bUpdateBarView = true;
            }
			else
			{
				m_iSelectedEvent = -1;
				m_iSelectedBar = -1;
			}

            if (bUpdateThreadView)
                UpdateThreadViewSelection();
            if (bUpdateTimingView)
                UpdateTimingViewSelection();
			if (bUpdateBarView)
				UpdateSelectedBarFromSelectedEvent();
        }

        // find the item in the thread view for the currently selected time and select it
        void UpdateThreadViewSelection()
        {
            listView2.SelectedItems.Clear();
            if (m_iCoreThreadInfo >= 0 && m_iCoreThreadInfo < listView2.Items.Count)
                listView2.Items[m_iCoreThreadInfo].Selected = true;
            listView2.EnsureVisible(m_iCoreThreadInfo);
        }

        // keep track of the node stack and work out the correct node for the given time point on this thread
        private int FindEventIndex(int iThread, int iTime)
        {
            List<int> nodeStack = new List<int>();
            List<Event> eventList = m_Capture.m_ThreadEventList[iThread];

            if (eventList.Count == 0)
                return -1;

			for (int iEvent = 0; iEvent < eventList.Count; iEvent++)
			{
                Event ev = eventList[iEvent];
                if (ev.iString == 0xfffffffe || ev.iString == 0xffffffff)
                    continue; // skip thread start/stop events

                if (ev.iTime > iTime)
                {
                    if (nodeStack.Count>0)
                        return nodeStack[nodeStack.Count-1];
                    else
                        return -1;
                }

                if (ev.iEnd == 0)
                {
                    // open a new node
                    nodeStack.Add(iEvent);
                }
                else
                {
                    if (nodeStack.Count > 0)
                    {
                        nodeStack.RemoveAt(nodeStack.Count - 1);
                    }
                }
			}

            if (nodeStack.Count == 0)
                return -1;
            else
                return nodeStack[nodeStack.Count-1];
        }

		void UpdateHierarchyViewSelection()
		{
			m_HierarchyView.SetParameters(m_Parent, this, m_Capture, m_UserSettings);
			m_HierarchyView.SelectString(m_iSelectedString);
		}

        void UpdateTimingViewSelection()
        {
            listView1.SelectedItems.Clear();
            if (m_Capture.m_CoreEventList[m_iCurrentCore].Count == 0)
                return;
            int iEvent = m_Capture.m_CoreEventList[m_iCurrentCore][m_iCoreThreadInfo].iStartEvent;
            int iThread = m_Capture.m_CoreEventList[m_iCurrentCore][m_iCoreThreadInfo].iThread;
            int iLastGoodEvent = -1;
            for (int i=0; i<iEvent; i++)
            {
                Event ev = m_Capture.m_ThreadEventList[iThread][i];
                if (ev.iString != 0xfffffffe && ev.iString != 0xffffffff)
                {
                    iLastGoodEvent = i;
                }
            }

            iLastGoodEvent = FindEventIndex(iThread, (int)m_iCurrentTime);

            if (iLastGoodEvent != -1)
            {
				m_iSelectedString = m_Capture.m_ThreadEventList[iThread][iLastGoodEvent].iString;

                treeListView1.UnselectAllNodes();
                TreeListNode t = m_iEventIndexToNodeMap[iLastGoodEvent];
                t.Selected = true;
                t.Focused = true;
                TreeListNode iT = t;
                while (t != null)
                {
                    t.Expand();
                    t = (TreeListNode)t.ParentNode();
                }
                treeListView1.AdjustScrollbars();
                treeListView1.CenterViewOnItem(iT);
                treeListView1.Invalidate();
            }
			else
				m_iSelectedString = 0xffffffff;

			UpdateHierarchyViewSelection();
        }

        // ---

		public CaptureData m_Capture;
        MainApp m_Parent;
		int m_iCurrentCore;              // number of the current core
        UInt32 m_iCurrentTime;                  // current focus time
        int m_iSelectedThread;
        int m_iSelectedEvent;
        int m_iSelectedBar;
		bool m_bSelectedArea;
		int m_iSelectedMin;
		int m_iSelectedMax;
		UInt32 m_iSelectedString;
        int m_iCoreThreadInfo;

		int m_iCurrentThreadIndex;      // index of the current thread int the thread-view
		List<int> m_iThreadStartEvent;  // index of the start event of each thread in the thread-view
		List<int> m_iThreadEndEvent;
        List<TreeListNode> m_iEventIndexToNodeMap;
        HierarchyView m_HierarchyView;
		UserSettings m_UserSettings;

        private void toolStripButton1_Click(object sender, EventArgs e)
        {
            //m_HierarchyView.MdiParent = MdiParent;
            //m_HierarchyView.Show();
        }

        private void CaptureView_FormClosing(object sender, FormClosingEventArgs e)
        {
			if (!m_Capture.m_bSaved)
			{
				DialogResult result = MessageBox.Show("The timing capture has not been saved\nDo you want to save this timing capture?", "Save Timing Capture", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation);
				if (result == DialogResult.Yes)
				{
					if (m_Capture.m_sOriginalFilename != null && m_Capture.m_sOriginalFilename != "")
					{
						Stream stream = System.IO.File.OpenWrite(m_Capture.m_sOriginalFilename);
						if (stream != null)
						{
							m_Capture.SaveToFile(stream, m_Capture.m_sOriginalFilename);
							m_Parent.CaptureViewClosing(this);            
							return;
						}
					}

					saveToolStripMenuItem_Click(null,null);
				}
			}

            m_HierarchyView.SetParameters(m_Parent, null, null, m_UserSettings);

            m_Parent.CaptureViewClosing(this);

			m_UserSettings.m_iDimensions[(int)DimensionValues.Thread_Name] = listView2.Columns[0].Width;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Thread_Start] = listView2.Columns[1].Width;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Thread_End] = listView2.Columns[2].Width;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Thread_Duration] = listView2.Columns[3].Width;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Capture_HSplit] = splitContainer1.SplitterDistance;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Capture_VSplit] = splitContainer2.SplitterDistance;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_Name] = treeListView1.Columns[0].Width;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_Start] = treeListView1.Columns[1].Width;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_End] = treeListView1.Columns[2].Width;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_Duration] = treeListView1.Columns[3].Width;
			m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_CacheUsed] = treeListView1.Columns[4].Width;
            m_UserSettings.m_iDimensions[(int)DimensionValues.Timing_CacheWasted] = treeListView1.Columns[5].Width;
        }

        private void toolStripButton2_Click(object sender, EventArgs e)
        {
            panel2.ZoomOut();
            CenterBarView();
        }

        private void CenterBarView()
        {
            int v = panel2.TimeToWidth((int)m_iCurrentTime) - (splitContainer1.Panel1.Width/2);
            if (v<0)
                v = 0;
			if (v>=splitContainer1.Panel1.HorizontalScroll.Maximum)
				v = splitContainer1.Panel1.HorizontalScroll.Maximum=1;
            splitContainer1.Panel1.HorizontalScroll.Value = v;
        }

        private void toolStripButton3_Click(object sender, EventArgs e)
        {
            panel2.ZoomIn();
            CenterBarView();
        }

        private void panel2_MouseClick(object sender, MouseEventArgs e)
        {
            panel2.ProcessMouseClick(sender, e);
        }

        public void UpdateBarColours(UserSettings settings)
        {
            panel2.UpdateBarColours(settings);
        }

        // change the thread
        private void listView2_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listView2.SelectedIndices.Count > 0)
            {
                int iThread = m_Capture.m_CoreEventList[m_iCurrentCore][listView2.SelectedIndices[0]].iThread;
                if (iThread != m_iCurrentThreadIndex)
                {
                    m_iCurrentThreadIndex = iThread;
                    InitialiseTimingView();
                }
                return;
            }
        }

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Displays a SaveFileDialog so the user can save the capture
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Timing Capture (*.pvcap)|*.pvcap";
            saveFileDialog1.Title = "Save a Capture";
            saveFileDialog1.FileName = Directory.GetCurrentDirectory() + "\\" + Name + ".pvcap";
            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                //serialize the object in binary format.
                Stream stream = saveFileDialog1.OpenFile();
				m_Capture.SaveToFile(stream, saveFileDialog1.FileName);
				m_Parent.mruMenu.AddFile(saveFileDialog1.FileName);

                //Update Name
                string name = Path.GetFileNameWithoutExtension(m_Capture.m_sOriginalFilename);
                this.Name = name;
                this.TabText = name;
                this.Text = name;

            }
        }

		private void treeListView1_OnSelectionChangedEvent(object sender, TreeListNode node)
		{
			int iNewEvent = (((TimingViewNodeX)node).iEvent);
			if (m_iSelectedEvent != iNewEvent)
			{
				panel2.Invalidate();

				if (m_iSelectedEvent != iNewEvent)
				{
					if (m_Capture.m_ThreadEventList[m_iSelectedThread].Count<=iNewEvent)
						iNewEvent = 0;

					m_iSelectedEvent = iNewEvent;
					Event ev = m_Capture.m_ThreadEventList[m_iSelectedThread][m_iSelectedEvent];
					UInt32 iNewString = ev.iString;
					if (m_iSelectedString != iNewString)
					{
						m_iSelectedString = iNewString;
						UpdateHierarchyViewSelection();														
					}

					SetSelectedArea(((TimingViewNodeX)node).iStart, ((TimingViewNodeX)node).iEnd);
//					m_iCurrentTime = ev.iTime + 1;

//					UpdateSelectedBarFromSelectedEvent();
				}
			}
		}


    }

	public class TimingViewNodeX : TreeListNode
	{
		public int iEvent, iStart, iEnd;
	};

}
