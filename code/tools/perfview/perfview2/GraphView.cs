using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;
using System.Text.RegularExpressions;

namespace ProfilingApplication
{
    public partial class GraphView : DockContent
    {
        Timer m_Timer;
		NetworkCommunication m_NetworkCommunication;
		UserSettings m_Settings;
		CounterPropertiesConfig m_CounterPropertiesConfig;
		MainApp m_Parent;
		bool m_bReceivedPreviousCounterList;
		List<int> m_ActiveCustomCounters = new List<int>();
		List<string> m_CustomCounterDependencies = new List<string>();
		List<GraphChart> m_GraphCharts;
		int m_nCurrentX, m_nCurrentY;
		bool m_bPaused;

		List<CounterGraphInfo> m_Counters = new List<CounterGraphInfo>();

		CounterGraphInfo m_FrameIndex = new CounterGraphInfo();

        public GraphView(MainApp parent, UserSettings settings, NetworkCommunication networkCommunication)
        {
            InitializeComponent();
			counterListView.FullRowSelect = true;

			m_Parent = parent;
			m_Settings = settings;
			m_NetworkCommunication = networkCommunication;
			UpdateCounterPropertiesConfig(settings.m_CounterPropertiesConfig);

            m_Timer = new Timer();
            m_Timer.Interval = 200;
            m_Timer.Tick += new EventHandler(m_Timer_Tick);
            m_Timer.Start();

			m_bReceivedPreviousCounterList = true;

			CreateGraphCharts();
        }

		// create a single GraphChart given a location
		GraphChart CreateGraphChart(int x, int y, int w, int h)
		{
			GraphChart rtn = new GraphChart(this);

			rtn.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			rtn.BackColor = System.Drawing.SystemColors.AppWorkspace;
			rtn.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			rtn.Name = "graphChart";
			rtn.Location = new System.Drawing.Point(x, y);
			rtn.Size = new System.Drawing.Size(w, h);
			this.splitContainer1.Panel1.Controls.Add(rtn);
			rtn.SetSettings(m_Settings.m_GraphViewConfig);
			return rtn;
		}

		// clear all graphcharts from our form
		void ClearGraphCharts()
		{
			for (int i=0; i<m_GraphCharts.Count; i++)
				this.splitContainer1.Panel1.Controls.Remove(m_GraphCharts[i]);
			m_GraphCharts = null;
		}

		// create a new set of graph charts for our form
		void CreateGraphCharts()
		{
			if (m_GraphCharts != null)
				ClearGraphCharts();

			m_GraphCharts = new List<GraphChart>();
			int iWidth = this.splitContainer1.Panel1.Width - 3;
			int iHeight = this.splitContainer1.Panel1.Height - 3;

			if (!m_Settings.m_GraphViewConfig.bMultipleGraphs)
			{
				m_GraphCharts.Add( CreateGraphChart(3, 3, iWidth, iHeight));
				m_GraphCharts[0].SetCounters(m_Counters);
			}
			else
			{
				// work out number of graphs in X and Y
				// keep dividing the used space in the largest axis until we have enough boxes
				int nNeeded = m_Counters.Count;
				int nX = 1;
				int nY = 1;
				while (nX * nY < nNeeded)
				{
					if ((iWidth/nX)>(iHeight/nY))
						nX++;
					else
						nY++;
				}
				m_nCurrentX = nX;
				m_nCurrentY = nY;

				// work out graph sizes
				int cX = iWidth/nX;
				int cY = iHeight/nY;

				// create the graphs
				int iCounter = 0;
				for (int y=0; y<nY; y++)
				{
					for (int x=0; x<nX; x++)
					{
						if (iCounter<nNeeded)
						{
							m_GraphCharts.Add( CreateGraphChart(x*cX+3, y*cY+3, cX,cY) );
							List<CounterGraphInfo> counter = new List<CounterGraphInfo>();
							counter.Add(m_Counters[iCounter]);
							m_GraphCharts[m_GraphCharts.Count-1].SetCounters(counter);
							iCounter++;
						}
					}
				}				
			}
		}

        void m_Timer_Tick(object sender, EventArgs e)
        {
			if (m_bPaused)
				return;

			if (!m_bReceivedPreviousCounterList || counterListView.Items.Count == 0)
				return;

			// read the counter names from the list view and send them to the app to retrieve their current values
			List<string> counterList = new List<string>();
			for (int i=0; i<m_Counters.Count; i++)
			{
				if (m_Counters[i].iCustomCounter == -1)
					counterList.Add(m_Counters[i].sName);
			}
			for (int i=0; i<m_CustomCounterDependencies.Count; i++)
			{
				counterList.Add(m_CustomCounterDependencies[i]);
			}
			if (counterList.Count == 0)
				return;

			m_NetworkCommunication.SendCounterListMessage(counterList);
			m_bReceivedPreviousCounterList = false;
        }

		// return the index of the counter with the given name
		private int FindCounterByName(string name)
		{
			for (int i=0; i<m_Counters.Count; i++)
			{
				if (m_Counters[i].sName == name)
					return i;
			}
			return -1;
		}

		static CounterValues g_CounterInfoList;

		static string CustomCounterMatchEvaluator(Match match)
		{
			string s = match.Value.Substring(2, match.Value.Length - 3);
			for (int i=0; i<g_CounterInfoList.counters.Count; i++)
			{
				if (g_CounterInfoList.counters[i].sName == s)
					return g_CounterInfoList.counters[i].fValue.ToString();
			}

			return "0";
		}

		public void SetReceivedCounterList(CounterValues list)
		{
			m_bReceivedPreviousCounterList = true;
			g_CounterInfoList = list;

			// update the received counter values in the list-view
			for (int i=0; i<list.counters.Count; i++)
			{
				int idx = FindCounterByName(list.counters[i].sName);
				if (idx == -1)
					continue;
				float fValue = list.counters[i].fValue;
				m_Counters[idx].AddValue(fValue);
				counterListView.Items[idx].SubItems[2].Text = fValue.ToString();
				counterListView.Items[idx].SubItems[3].Text = m_Counters[idx].fMin.ToString();
				counterListView.Items[idx].SubItems[4].Text = m_Counters[idx].fMax.ToString();
			}

			// evaluate custom counters
			for (int i=0; i<m_Counters.Count; i++)
			{
				int idx = m_Counters[i].iCustomCounter;
				if (idx == -1)
					continue;

				// find macros and replace them with the current counter values
				Regex macroRegex = new Regex(@"\$\([^\)]+\)");
				string s = macroRegex.Replace(m_CounterPropertiesConfig.m_CustomCounters[idx].sExpression, CustomCounterMatchEvaluator);

				double f = ExpressionEvaluator.Calculate(s);

				m_Counters[i].AddValue((float)f);
				counterListView.Items[i].SubItems[2].Text = f.ToString();
				counterListView.Items[i].SubItems[3].Text = m_Counters[i].fMin.ToString();
				counterListView.Items[i].SubItems[4].Text = m_Counters[i].fMax.ToString();
			}

			// store frame index
			CounterInfo info = list.counters[list.counters.Count-1];
			if (info.sName == "$_frameidx")
				m_FrameIndex.AddValue(info.fValue);

			for (int i=0; i<m_GraphCharts.Count; i++)
			{
				m_GraphCharts[i].MoveGridOffset(1);
				m_GraphCharts[i].Invalidate();
			}
		}

		private void addCounterButton_Click(object sender, EventArgs e)
		{
			m_NetworkCommunication.SendMessage("ListCounters");
		}

		int FindCustomCounterIndex(string sName)
		{
			for (int i=0; i<m_CounterPropertiesConfig.m_CustomCounters.Count; i++)
			{
				if (m_CounterPropertiesConfig.m_CustomCounters[i].sName == sName)
					return i;
			}
			return -1;
		}

		void AddCustomCounterDependency(string name)
		{
			// check whether the dependency already exists in the normal counters list
			string s = name;
			s = s.Substring(2, s.Length-3);
			for (int i=0; i<m_Counters.Count; i++)
			{
				if (m_Counters[i].sName == s)
					return;
			}
			// check whether the dependency already exists in the dependency list
			for (int i=0; i<m_CustomCounterDependencies.Count; i++)
			{
				if (m_CustomCounterDependencies[i] == s)
					return;
			}
			m_CustomCounterDependencies.Add(s);
		}

		// add a custom counter to the list of active custom counters, and evaluate it's dependencies
		void AddCustomCounter(int x)
		{
			m_ActiveCustomCounters.Add(x);

			string sExpression = m_CounterPropertiesConfig.m_CustomCounters[x].sExpression;
			Regex macroRegex = new Regex(@"\$\([^\)]+\)");
			MatchCollection matches = macroRegex.Matches(sExpression);
			for (int i=0; i<matches.Count; i++)
			{
				AddCustomCounterDependency(matches[i].Value);
			}
		}

		internal void UpdateCounterPropertiesConfig(CounterPropertiesConfig cfg)
		{
			m_CounterPropertiesConfig = cfg;
			counterListView.Items.Clear();
			m_Counters.Clear();
			m_ActiveCustomCounters.Clear();

			// update the list of active counters from the counter properties config settings class
			for (int i=0; i<cfg.m_ActiveCounters.Count; i++)
			{
				CounterConfig ccfg = cfg.m_ActiveCounters[i];
				string sName = ccfg.sName;
				float fScale = ccfg.fScale;
				string[] row = { "", sName, "", "", "", fScale.ToString() };

				int iCustomCounter = FindCustomCounterIndex(sName);
				if (iCustomCounter != -1)
					AddCustomCounter(iCustomCounter);

				ListViewItem item = new ListViewItem(row);
				item.UseItemStyleForSubItems = false;
				item.BackColor = System.Drawing.Color.FromArgb(ccfg.r, ccfg.g, ccfg.b);
				counterListView.Items.Add(item);
				CounterGraphInfo info = new CounterGraphInfo();
				info.nValues = 0;
				info.iLastValue = 0;
				info.sName = sName;
				info.pen = new Pen(System.Drawing.Color.FromArgb(cfg.m_ActiveCounters[i].r, cfg.m_ActiveCounters[i].g, cfg.m_ActiveCounters[i].b));
				info.fScale = fScale;
				info.iCustomCounter = iCustomCounter;
				info.bMinMaxInitialised = false;
				info.fMin = 0;
				info.fMax = 0;
				m_Counters.Add(info);
			}

			counterListView.Invalidate();
			CreateGraphCharts();
		}

		private void GraphView_FormClosing(object sender, FormClosingEventArgs e)
		{
			m_Parent.NotifyGraphViewClosing();
		}

		private void graphViewConfigButton_Click(object sender, EventArgs e)
		{
			GraphViewConfigDlg dlg = new GraphViewConfigDlg(m_Settings.m_GraphViewConfig);
			if (dlg.ShowDialog() == DialogResult.OK)
			{
				m_Settings.m_GraphViewConfig = dlg.GetSettings();
				CreateGraphCharts();
	            m_Timer.Interval = 1000/m_Settings.m_GraphViewConfig.iUpdatesPerSec;
			}
		}

		private void GraphView_SizeChanged(object sender, EventArgs e)
		{
			int iWidth = this.splitContainer1.Panel1.Width - 3;
			int iHeight = this.splitContainer1.Panel1.Height - 3;

			// work out number of graphs in X and Y
			// keep dividing the used space in the largest axis until we have enough boxes
			int nNeeded = m_Counters.Count;
			int nX = 1;
			int nY = 1;
			while (nX * nY < nNeeded)
			{
				if ((iWidth/nX)>(iHeight/nY))
					nX++;
				else
					nY++;
			}
			// reposition the existing graphs
			// work out graph sizes
			int cX = iWidth/nX;
			int cY = iHeight/nY;

			// create the graphs
			int iCounter = 0;
			for (int y=0; y<nY; y++)
			{
				for (int x=0; x<nX; x++)
				{
					if (iCounter<nNeeded)
					{
						m_GraphCharts[iCounter].Location = new Point(x*cX+3, y*cY+3);
						m_GraphCharts[iCounter].Size = new Size(cX,cY);
						iCounter++;
					}
				}
			}				


		}

		private void pauseButton_CheckedChanged(object sender, EventArgs e)
		{
			m_bPaused = ((CheckBox)sender).Checked;
			m_NetworkCommunication.SendMessage(m_bPaused?"Pause":"Unpause");
		}

		private void continuousCaptureCheckbox_CheckedChanged(object sender, EventArgs e)
		{
			m_NetworkCommunication.SendMessage(((CheckBox)sender).Checked?"ContinuousCaptureOn":"ContinuousCaptureOff");
		}

		// when the user clicks on a chart, retrieve the timing capture for that frame if possible
		internal void ChartClicked(int iSelection)
		{
			CounterGraphInfo info = m_FrameIndex;
			if (iSelection > info.nValues)
				return;
			int idx = info.iLastValue - iSelection;
			while (idx < 0)
				idx += CounterGraphInfo.iMaxValues;

			string s = "RetrieveTimingCapture" + ((int)info.fValues[idx]).ToString();
			m_NetworkCommunication.SendMessage(s);
		}
	}

	public class CounterGraphInfo
	{
		public string sName;
		public float[] fValues = new float[iMaxValues];
		public int iLastValue;
		public float fScale;
		public float fMin;
		public float fMax;
		public const int iMaxValues = 250;
		public int nValues;
		public Pen pen;
		public int iCustomCounter;
		public bool bMinMaxInitialised;

		public void AddValue(float fValue)
		{
			if (!bMinMaxInitialised)
			{
				bMinMaxInitialised = true;
				fMin = fValue;
				fMax = fValue;
			}
			else
			{
				if (fValue<fMin)
					fMin = fValue;
				if (fValue>fMax)
					fMax = fValue;
			}

			iLastValue++;
			if (iLastValue >= iMaxValues)
				iLastValue = 0;
			fValues[iLastValue] = fValue;
			if (nValues < iMaxValues)
				nValues++;
		}
	};

	public struct CounterInfo
	{
		public string sName;
		public float fValue;
	};

	public class CounterValues
	{
		public List<CounterInfo> counters;

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
			counters = new List<CounterInfo>();

            // receive events
            int nCounters;
            msgReader.Read(out nCounters);
            counters.Clear();
            for (int i=0; i<nCounters; i++)
            {
                CounterInfo ci = new CounterInfo();
                msgReader.Read(out ci.sName);
                msgReader.Read(out ci.fValue);
                counters.Add(ci);
            }
		}
	};

	public class DoubleBufferedListView : System.Windows.Forms.ListView
	{
		public DoubleBufferedListView()
		{
			this.DoubleBuffered = true;
		}
	};
}
