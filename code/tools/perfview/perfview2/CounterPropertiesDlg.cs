using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using System.Xml.Serialization;

namespace ProfilingApplication
{
	public partial class CounterPropertiesDlg : Form
	{
		List<CounterConfig> m_Counters;
		List<CounterConfig> m_ActiveCounters;
		int m_iCurrentColour;
		CounterPropertiesConfig m_Config;
		List<string> m_OriginalCounterList;
		Dictionary<string, InactiveCounterNode> m_StringMap;

		public CounterPropertiesDlg()
		{
			InitializeComponent();

			m_Counters = new List<CounterConfig>();
			m_ActiveCounters = new List<CounterConfig>();
			m_iCurrentColour = 0;
		}

		Color[] m_ColourArray = { System.Drawing.Color.AliceBlue, System.Drawing.Color.AntiqueWhite, System.Drawing.Color.Azure, 
		System.Drawing.Color.Beige, System.Drawing.Color.Brown, System.Drawing.Color.Crimson, System.Drawing.Color.DarkGray, 
		System.Drawing.Color.DarkSeaGreen, System.Drawing.Color.Firebrick, System.Drawing.Color.Green, System.Drawing.Color.Olive, 
		System.Drawing.Color.Pink, System.Drawing.Color.Silver, System.Drawing.Color.White, System.Drawing.Color.Yellow};

		internal void SetConfigData(CounterPropertiesConfig currentConfig)
		{
			m_ActiveCounters.Clear();

			// find counters in the current config and copy over the current parameters
			for (int i=0; i<currentConfig.m_ActiveCounters.Count; i++)
			{
				int idx = FindCounterByName(currentConfig.m_ActiveCounters[i].sName, true);

				m_Counters[idx] = currentConfig.m_ActiveCounters[i];
				m_ActiveCounters.Add(m_Counters[idx]);
			}

			m_Config = currentConfig.Copy();

			UpdateListViews();
		}

		// add a counter to the list of this dialog
		void AddCounterToCounterList(string name)
		{
			CounterConfig tmp;
			tmp.sName = name;
			tmp.bActive = false;
			tmp.fScale = 1;
			tmp.r = m_ColourArray[m_iCurrentColour].R;
			tmp.g = m_ColourArray[m_iCurrentColour].G;
			tmp.b = m_ColourArray[m_iCurrentColour].B;
			m_iCurrentColour++;
			if (m_iCurrentColour >= m_ColourArray.Length)
				m_iCurrentColour = 0;
			m_Counters.Add(tmp);			
		}

		// set available counter list and current counter config
		internal void SetCounterData(List<string> list, CounterPropertiesConfig config)
		{
			m_OriginalCounterList = list;
			m_Counters.Clear();
			m_ActiveCounters.Clear();

			// add all existing counters into the list
			for (int i=0; i<list.Count; i++)
				AddCounterToCounterList(list[i]);

			// add all existing custom counters into the list
			for (int i=0; i<config.m_CustomCounters.Count; i++)
				AddCounterToCounterList(config.m_CustomCounters[i].sName);

			SetConfigData(config);
		}

		// get new edited counter config
		internal CounterPropertiesConfig GetCounterData()
		{
			CounterPropertiesConfig config = m_Config;

			// add all active counters into the config
			config.m_ActiveCounters.Clear();
			for (int i=0; i<m_ActiveCounters.Count; i++)
			{
				config.m_ActiveCounters.Add(m_ActiveCounters[i]);
			}

			return config;
		}

		private string GetParentNodeName(string s)
		{
			int x = s.LastIndexOf('.');
			if (x == -1)
				return "[$root]";
			else
				return s.Substring(0, x);
		}

		private void LinkNode(InactiveCounterNode node)
		{
			if (node.sName == "[$root]")
				return;
			m_StringMap[node.sName] = node;

			InactiveCounterNode parentNode;
			string sParentName = GetParentNodeName(node.sName);
			if (sParentName == "[$root]")
			{
				inactiveCounterView.Nodes.Add(node);
				return;
			}

			if (m_StringMap.ContainsKey(sParentName))
			{
				parentNode = m_StringMap[sParentName];
				parentNode.Nodes.Add(node);
				return;
			}

			parentNode = new InactiveCounterNode(sParentName);
			parentNode.bHasCounter = false;
			parentNode.Nodes.Add(node);
			m_StringMap[parentNode.sName] = parentNode;
			if (sParentName == "[$root]")
				inactiveCounterView.Nodes.Add(parentNode);
			else
				LinkNode(parentNode);
		}

		// update list views after the counter data has been changed
		internal void UpdateListViews()
		{
			activeCounterView.Clear();
			inactiveCounterView.Nodes.Clear();
			m_StringMap = new Dictionary<string,InactiveCounterNode>();

			List<string> counters = new List<string>();
			for (int i=0; i<m_Counters.Count; i++)
				if (!m_Counters[i].bActive)
					counters.Add(m_Counters[i].sName);
			counters.Sort();

			// add all inactive counters to the inactive list
			for (int i=0; i<counters.Count; i++)
				LinkNode(new InactiveCounterNode(counters[i]));

			// add all active counters to the active list
			for (int i=0; i<m_ActiveCounters.Count; i++)
			{
				activeCounterView.Items.Add(new ListViewItem(m_ActiveCounters[i].sName));
			}

			colorButton.Enabled = false;
			scaleTextBox.Enabled = false;
			m_StringMap = null;
		}

		// return the index of the counter with the given name, or adds a new counter if it doesn't currently exist
		private int FindCounterByName(string name, bool bCreate)
		{
			for (int i=0; i<m_Counters.Count; i++)
			{
				if (m_Counters[i].sName == name)
					return i;
			}
			if (!bCreate)
				return -1;

			CounterConfig tmp = new CounterConfig();
			tmp.sName = name;
			m_Counters.Add(tmp);
			return m_Counters.Count-1;
		}

		// recursively add counters to active list
		private void AddNodesToActiveRecursive(TreeNode node)
		{
			if (!((InactiveCounterNode)node).bHasCounter)
				return;
			int idx = FindCounterByName(((InactiveCounterNode)node).sName, false);
			if (idx != -1)
			{
				CounterConfig tmp = m_Counters[idx];
				tmp.bActive = true;
				m_Counters[idx] = tmp;
				m_ActiveCounters.Add(tmp);
			}

			//for (TreeNode x = node.FirstNode; x != null; x = x.NextNode)
			//    AddNodesToActiveRecursive(x);								
		}

		// move a counter from inactive to active
		private void addButton_Click(object sender, EventArgs e)
		{
			if (inactiveCounterView.SelectedNode == null)
				return;

			// move all subnodes if a rootnode is active
			AddNodesToActiveRecursive(inactiveCounterView.SelectedNode);

			UpdateListViews();
		}

		// move a counter from active to inactive
		private void removeButton_Click(object sender, EventArgs e)
		{
			if (activeCounterView.SelectedItems.Count == 0)
				return;
			int idxOffset = 0;
			for (int i=0; i<activeCounterView.SelectedItems.Count; i++)
			{
				int idx = FindCounterByName(activeCounterView.SelectedItems[i].Text, false);
				if (idx != -1)
				{
					int idxA = activeCounterView.SelectedIndices[i] - idxOffset;
					CounterConfig tmp = m_ActiveCounters[idxA];
					tmp.bActive = false;
					m_Counters[idx] = tmp;
					m_ActiveCounters.RemoveAt(idxA);
					idxOffset++;
				}
			}

			UpdateListViews();
		}

		// change the colour of the selected counters
		private void colorButton_Click(object sender, EventArgs e)
		{
			if (activeCounterView.SelectedItems.Count == 0)
				return;

    		ColorButton callingButton = (ColorButton)sender;

    		Point p = new Point(groupBox2.Left + callingButton.Left, groupBox2.Top + callingButton.Top + callingButton.Height);
    		p = PointToScreen( p);
    		    		
    		ColorPaletteDlg clDlg = new ColorPaletteDlg(p.X, p.Y);

    		clDlg.ShowDialog();    	 
    		
    		if(clDlg.DialogResult == DialogResult.OK)
			{
				for (int i=0; i<activeCounterView.SelectedIndices.Count; i++)
				{
					int idx = activeCounterView.SelectedIndices[i];
					CounterConfig tmp = m_ActiveCounters[idx];
					tmp.r = clDlg.Color.R;
					tmp.g = clDlg.Color.G;
					tmp.b = clDlg.Color.B;
					m_ActiveCounters[idx] = tmp;
				}
				colorButton.CenterColor = clDlg.Color;
				colorButton.Invalidate();
			}
    		
    		Invalidate();
    		
    		clDlg.Dispose();
		}

		private void activeCounterView_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (activeCounterView.SelectedItems.Count == 0)
			{
				colorButton.Enabled = false;
				scaleTextBox.Enabled = false;
				return;
			}

			colorButton.Enabled = true;
			scaleTextBox.Enabled = true;

			// update the counter's colour on the colour button
			int idx = activeCounterView.SelectedIndices[0];
    		colorButton.CenterColor = System.Drawing.Color.FromArgb(m_ActiveCounters[idx].r, m_ActiveCounters[idx].g, m_ActiveCounters[idx].b);
			colorButton.Invalidate();

			// update the counter's scale
			scaleTextBox.Value = (decimal)m_ActiveCounters[idx].fScale;
		}

		private void SwapItems(int iFirst)
		{
			if (iFirst<0)
				return;
			if (iFirst+1 >= activeCounterView.Items.Count)
				return;
			{
			CounterConfig tmp = m_ActiveCounters[iFirst+1];
			m_ActiveCounters[iFirst+1] = m_ActiveCounters[iFirst];
			m_ActiveCounters[iFirst] = tmp;
			}

			{
			ListViewItem tmp = (ListViewItem)activeCounterView.Items[iFirst+1].Clone();
			activeCounterView.Items[iFirst+1] = (ListViewItem)activeCounterView.Items[iFirst].Clone();
			activeCounterView.Items[iFirst] = tmp;
			}
		}

		private void moveUpButton_Click(object sender, EventArgs e)
		{
			List<int> selectedItems = new List<int>();
			for (int i=0; i<activeCounterView.SelectedItems.Count; i++)
			{
				int idx = activeCounterView.SelectedIndices[i];
				selectedItems.Add(idx);
				SwapItems(idx-1);
			}
			for (int i=0; i<selectedItems.Count; i++)
			{
				int x = selectedItems[i]-1;
				if (x<0) x=0;
				activeCounterView.SelectedIndices.Add(x);
			}
		}

		private void moveDownButton_Click(object sender, EventArgs e)
		{
			List<int> selectedItems = new List<int>();
			for (int i=0; i<activeCounterView.SelectedItems.Count; i++)
			{
				int idx = activeCounterView.SelectedIndices[i];
				selectedItems.Add(idx);
				SwapItems(idx);
			}
			for (int i=0; i<selectedItems.Count; i++)
			{
				int x = selectedItems[i]+1;
				if (x>=activeCounterView.Items.Count)
					x = activeCounterView.Items.Count-1;
				activeCounterView.SelectedIndices.Add(x);
			}
		}

		private void scaleTextBox_ValueChanged(object sender, EventArgs e)
		{
			for (int i=0; i<activeCounterView.SelectedItems.Count; i++)
			{
				int idx = activeCounterView.SelectedIndices[i];
				CounterConfig info = m_ActiveCounters[idx];
				info.fScale = (float)scaleTextBox.Value;
				m_ActiveCounters[idx] = info;
			}
		}

		// Displays a an open file dialog so the user can open a counter config file
		private void loadButton_Click(object sender, EventArgs e)
		{
            OpenFileDialog openFileDialog1 = new OpenFileDialog();
            openFileDialog1 .Filter = "Counter Config (*.countercfg)|*.countercfg";
            openFileDialog1 .Title = "Open a Capture";
            openFileDialog1 .ShowDialog();

            // If the file name is not an empty string open it for saving.
            if(openFileDialog1.FileName != "")
            {
				CounterPropertiesConfig config;
				Stream stream = openFileDialog1.OpenFile();
				XmlSerializer xs = new XmlSerializer(typeof(CounterPropertiesConfig));

				// load data
				using (StreamReader sr = new StreamReader(stream))
				{
					config = (CounterPropertiesConfig)xs.Deserialize(sr);
					SetCounterData(m_OriginalCounterList, config);
				}
				stream.Close();
            }

		}

		// display an save as file dialog so the user can save a counter config file
		private void saveButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Counter Config (*.countercfg)|*.countercfg";
            saveFileDialog1.Title = "Save a Capture";
            saveFileDialog1.ShowDialog();

            // If the file name is not an empty string open it for saving.
            if(saveFileDialog1.FileName != "")
            {
                //serialize the object in binary format.
                Stream stream = saveFileDialog1.OpenFile();
				// save data
				CounterPropertiesConfig cfg = GetCounterData();
				XmlSerializer xs = new XmlSerializer(typeof(CounterPropertiesConfig));
				using (StreamWriter sw = new StreamWriter(stream))
				{
					xs.Serialize(sw, cfg);
				}

				stream.Close();
            }
		}

		// bring up the custom counter dialog box to let the user create a new custom counter
		private void customCounterButton_Click(object sender, EventArgs e)
		{
			CustomCounterDlg dlg = new CustomCounterDlg();
			if (dlg.ShowDialog() == DialogResult.OK)
			{
				CustomCounter c = dlg.GetCustomCounter();
				m_Config.m_CustomCounters.Add(c);
				AddCounterToCounterList(c.sName);
				UpdateListViews();
			}
		}

		private void deleteCustomCounterButton_Click(object sender, EventArgs e)
		{
			if (inactiveCounterView.SelectedNode == null)
				return;
			string name = inactiveCounterView.SelectedNode.Text;
			// find the custom counter and delete it from the config
			for (int i=0; i<m_Config.m_CustomCounters.Count; i++)
			{
				if (m_Config.m_CustomCounters[i].sName == name)
				{
					m_Config.m_CustomCounters.RemoveAt(i);
					SetCounterData(m_OriginalCounterList, GetCounterData());
					return;
				}
			}
		}
	}

	public class InactiveCounterNode : TreeNode
	{
		public InactiveCounterNode(string name) :
			base(name)
		{
			sName = name;
			int idx = name.LastIndexOf('.');
			string shortName;
			if (idx != -1)
				shortName = name.Substring(idx+1,name.Length-idx-1);
			else
				shortName = name;
			this.Text = shortName;
			this.bHasCounter = true;
		}

		public string sName;
		public bool bHasCounter;
	};

}

