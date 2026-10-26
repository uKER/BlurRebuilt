using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Reflection;
using System.Collections;

namespace ProfilingApplication
{
	public partial class MemoryFragmentationView : Form
	{
		MemoryTrackerData m_MemoryDump;
		MemoryNodeView m_NodeView;
		UserSettings m_Settings;
		Symbols m_Symbols;
		MainApp m_Parent;
		int m_iFilterTag;
		int m_iAllocation;
		int m_iTrackingId;
		ListViewItemComparer m_lvwItemComparer;

		public MemoryFragmentationView(MemoryTrackerData memoryDump, MainApp parent, UserSettings settings, MemoryNodeView nodeView)
		{
			InitializeComponent();

			blockSizeListBox.DropDownStyle = ComboBoxStyle.DropDownList;

			m_MemoryDump = memoryDump;
			m_NodeView = nodeView;
			m_Settings = settings;
			m_Parent = parent;

			panel1.SetMemoryTrackerData(memoryDump);
			panel1.SetParent(this);
			panel1.SetBlockSize(1024*256);
			blockSizeListBox.Text = "256K";

			m_Symbols = new Symbols();
			m_Symbols.Initialise(memoryDump.m_iPlatform, memoryDump.m_Modules);

			m_iFilterTag = -1;
			m_iAllocation = -1;

			UpdateNodeView();

			// The ListViewItemSorter property allows you to specify the
			// object that performs the sorting of items in the ListView.
			// You can use the ListViewItemSorter property in combination
			// with the Sort method to perform custom sorting.
			listView1.ListViewItemSorter = m_lvwItemComparer = new ListViewItemComparer();

			InitialiseFragmentationViewOnce();
		}

		void InitialiseFragmentationViewOnce()
		{
			listView1.Columns[0].Width = m_Settings.m_iDimensions[(int)DimensionValues.Alloc_Name];
			listView1.Columns[1].Width = m_Settings.m_iDimensions[(int)DimensionValues.Alloc_Start];
			listView1.Columns[2].Width = m_Settings.m_iDimensions[(int)DimensionValues.Alloc_End];
			listView1.Columns[3].Width = m_Settings.m_iDimensions[(int)DimensionValues.Alloc_Size];
			listView1.Columns[3].Width = m_Settings.m_iDimensions[(int)DimensionValues.Alloc_Tag];

			listView2.Columns[0].Width = m_Settings.m_iDimensions[(int)DimensionValues.Callstack_Name];
			listView2.Columns[1].Width = m_Settings.m_iDimensions[(int)DimensionValues.Callstack_File];
			listView2.Columns[2].Width = m_Settings.m_iDimensions[(int)DimensionValues.Callstack_Line];

			int x = this.Height - m_Settings.m_iDimensions[(int)DimensionValues.FragView_VSplit];
			if (x<2) x = 2;
			splitContainer1.SplitterDistance = x;
			x = this.Width - m_Settings.m_iDimensions[(int)DimensionValues.FragView_HSplit];
			if (x<2) x = 2;
			splitContainer2.SplitterDistance = x;
		}

		void UpdateNodeView()
		{
			m_NodeView.SetParameters(m_Parent, this, m_MemoryDump, m_Settings, m_iFilterTag);
		}

		// display all allocations within a selected address range in the bottom-left list view
		public void SelectBlock(UInt32 iBlockStart, UInt32 iSize, bool bFilterTrackingId)
		{
			// find all the allocations in this block and add them to the list view
			listView1.Items.Clear();

			UInt32 iBlockEnd = iBlockStart + iSize;
            List<ListViewItem> items = new List<ListViewItem>();

			UInt32 iUsage = 0;

			// find allocations that are within this block
			for (int i=0; i<m_MemoryDump.m_Allocations.Count; i++)
			{
				AllocationInfo item = m_MemoryDump.m_Allocations[i];
				if (m_iFilterTag != -1 && item.iTag != m_iFilterTag)
					continue;
				if (bFilterTrackingId && item.iContextId != m_iTrackingId)
					continue;

				UInt32 iAddrStart  = item.iAddress;
				UInt32 iAddrEnd    = item.iAddress + item.cSize;

				if (iAddrEnd >= iBlockStart && iAddrStart < iBlockEnd)
				{
					string[] row = { m_MemoryDump.m_Nodes[item.iContextId].sName, iAddrStart.ToString("X"), iAddrEnd.ToString("X"), MemoryNodeView.IntToMemoryString((int)item.cSize), item.iTag.ToString() };

					items.Add(new AugmentedListViewItem(row, i, (int)item.cSize));

					iUsage += ((iBlockEnd>iAddrEnd)?iAddrEnd:iBlockEnd) - ((iBlockStart>iAddrStart)?iBlockStart:iAddrStart);
				}
            }

            listView1.Items.AddRange(items.ToArray());

			float fUsage = (float)iUsage / (float)iSize;

			blockLabel.Text = iBlockStart.ToString("X") + " to " + iBlockEnd.ToString("X") + ": " + fUsage.ToString("P0") + " used";
		}

		string StripPath(string path)
		{
			int idx = path.LastIndexOf('\\');
			int idx2 = path.LastIndexOf('/');
			if (idx2 > idx)
				idx = idx2;
			if (idx > 0 && idx<path.Length-1)
				return path.Substring(idx+1);
			else
				return path;
		}

		// display the call-stack for a particular allocation in the bottom-right list view
		public void SelectAllocation(int iAllocation)
		{
			m_iAllocation = iAllocation;

			listView2.Items.Clear();

			// find allocations that are within this block
			AllocationInfo item = m_MemoryDump.m_Allocations[iAllocation];
            List<ListViewItem> items = new List<ListViewItem>(item.callstack.Count);
			for (int i=0; i<item.callstack.Count; i++)
			{
				SymbolInfo info = m_Symbols.LookupSymbol(item.callstack[i]);
				if (info.m_sSymbolName != "??")
				{
					string[] row = { info.m_sSymbolName, StripPath(info.m_sFilename), info.m_iLineNumber.ToString() };
					items.Add(new ListViewItem(row));
				}
				else
				{
					string[] row = { "0x" + item.callstack[i].ToString("X"), "-", "-" };
					items.Add(new ListViewItem(row));
				}

				if (info.m_sSymbolName == "._start")
					break;
			}

            listView2.Items.AddRange(items.ToArray());
		}

		private void listView1_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (listView1.SelectedIndices.Count > 0)
			{
				SelectAllocation(((AugmentedListViewItem)listView1.SelectedItems[0]).iAllocation);
			}
		}

		private void blockSizeListBox_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (blockSizeListBox.Text == "1MB")
				panel1.SetBlockSize(1024*1024);
			else if (blockSizeListBox.Text == "512K")
				panel1.SetBlockSize(1024*512);
			else if (blockSizeListBox.Text == "256K")
				panel1.SetBlockSize(1024*256);
			else if (blockSizeListBox.Text == "128K")
				panel1.SetBlockSize(1024*128);
			else if (blockSizeListBox.Text == "64K")
				panel1.SetBlockSize(1024*64);
			else if (blockSizeListBox.Text == "32K")
				panel1.SetBlockSize(1024*32);
			else if (blockSizeListBox.Text == "16K")
				panel1.SetBlockSize(1024*16);
			else if (blockSizeListBox.Text == "8K")
				panel1.SetBlockSize(1024*8);
			else if (blockSizeListBox.Text == "4K")
				panel1.SetBlockSize(1024*4);
			else if (blockSizeListBox.Text == "2K")
				panel1.SetBlockSize(1024*2);
			else
				panel1.SetBlockSize(1024*1);
		}

		private void filterTagButton_Click(object sender, EventArgs e)
		{
			this.filterTagLabel.Enabled = this.filterTagButton.Checked;
			this.filterTagNo.Enabled = this.filterTagButton.Checked;
			if (this.filterTagButton.Checked)
			{
				try
				{
					int iTag = int.Parse(filterTagNo.Text);
					panel1.SetFilterTag(iTag);
					m_iFilterTag = iTag;
					UpdateNodeView();
				}
				catch (Exception)
				{
					panel1.SetFilterTag(0);
					this.filterTagNo.Text = "0";
					m_iFilterTag = 0;
					UpdateNodeView();
				}
			}
			else
			{
				panel1.SetFilterTag(-1);
				m_iFilterTag = -1;
				UpdateNodeView();
			}
		}

		private void filterTagNo_Validated(object sender, EventArgs e)
		{
			filterTagButton_Click(sender,e);
		}

		private string StripArguments(string s)
		{
			int idx = s.LastIndexOf('(');
			if (idx <= 0)
				return s;
			else
				return s.Substring(0,idx);
		}

		private void SpawnVisualStudioWithSourceFile(string filename, int line)
		{
            try
            {
                Object obj = System.Runtime.InteropServices.Marshal.GetActiveObject("VisualStudio.DTE.8.0");
                if (obj != null)
                {
                    if (File.Exists(filename) == false)
                    {
                        MessageBox.Show("File " + filename + " does not exist");
                        return;
                    }

                    obj.GetType().InvokeMember("ExecuteCommand", BindingFlags.InvokeMethod, null, obj,
                        new object[]
                    {
                        "File.OpenFile",
                        "\"" + filename + "\""
                    });
                    obj.GetType().InvokeMember("ExecuteCommand", BindingFlags.InvokeMethod, null, obj,
                        new object[]
                    {
                        "Edit.GoTo",
                        line
                    });

                    Object window = obj.GetType().InvokeMember("MainWindow", BindingFlags.GetProperty,
                        null, obj, new object[] { });
                    window.GetType().InvokeMember("Activate", BindingFlags.InvokeMethod, null, window, new object[] { });
                }
                else
                {
                    MessageBox.Show("No instance of VS.NET 2003 found");
                    return;
                }
            }
            catch (Exception)
            {
                MessageBox.Show("No instance of VS.NET 2003 found");
                return;
            }
		}

		// copy callstack into clipboard
		private void button1_Click(object sender, EventArgs e)
		{
            string outputBuffer = 
				"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2057{\\fonttbl{\\f0\\fmodern\\fprq1\\fcharset0 Courier New;}{\\f1\\fswiss\\fcharset0 Arial;}}\n" +
				"\\viewkind4\\uc1\\pard\\f0\\fs18 ";
            int longestString = 0;

            foreach (ListViewItem lvi in listView2.Items)
            {
				string func = StripArguments(lvi.Text);
                if (func.Length > longestString)
                    longestString = func.Length;
            }

            foreach (ListViewItem lvi in listView2.Items)
            {
				string func = StripArguments(lvi.Text);
                outputBuffer += func;

                int padBy = longestString - func.Length;

                for (int iSpace = 0; iSpace < padBy + 5; iSpace++)
                    outputBuffer += " ";

                outputBuffer += "[" + lvi.SubItems[1].Text + " @ " + lvi.SubItems[2].Text + "]";
                                
                outputBuffer += "\\par\n";
            }

            Clipboard.SetText(outputBuffer + "\\f1\\fs20\\par\n}", TextDataFormat.Rtf);
		}

		private void listView2_MouseDoubleClick(object sender, MouseEventArgs e)
		{
            if (e.Button == MouseButtons.Left)
            {
                ListView lv = sender as ListView;
                ListView.SelectedListViewItemCollection selectedItemList = lv.SelectedItems;

				if (m_iAllocation == -1)
					return;

                foreach (ListViewItem item in selectedItemList)
                {
					SymbolInfo info = m_Symbols.LookupSymbol(m_MemoryDump.m_Allocations[m_iAllocation].callstack[item.Index]);
					SpawnVisualStudioWithSourceFile(info.m_sFilename, info.m_iLineNumber);
					return;
				}
			}
		}

		internal void SetSelectedTrackerId(int iNewNode)
		{
			m_iTrackingId = iNewNode;
			this.panel1.SetSelectedTrackerId(iNewNode);
		}

		private void closeToolStripMenuItem_Click(object sender, EventArgs e)
		{
            Close();
		}

		private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Displays a SaveFileDialog so the user can save the capture
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Memory Capture (*.mcap)|*.mcap";
            saveFileDialog1.Title = "Save a Capture";
            saveFileDialog1.ShowDialog();

            // If the file name is not an empty string open it for saving.
            if(saveFileDialog1.FileName != "")
            {
                //serialize the object in binary format.
                Stream stream = saveFileDialog1.OpenFile();
				m_MemoryDump.SaveToFile(stream, saveFileDialog1.FileName);
				m_Parent.mruMenu.AddFile(saveFileDialog1.FileName);
            }
        }

		private void MemoryFragmentationView_FormClosing(object sender, FormClosingEventArgs e)
		{
			if (!m_MemoryDump.m_bSaved)
			{
				DialogResult result = MessageBox.Show("The memory capture has not been saved\nDo you want to save this memory capture?", "Save Timing Capture", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation);
				if (result == DialogResult.Yes)
				{
					if (m_MemoryDump.m_sOriginalFilename != null && m_MemoryDump.m_sOriginalFilename != "")
					{
						Stream stream = System.IO.File.OpenWrite(m_MemoryDump.m_sOriginalFilename);
						if (stream != null)
						{
							m_MemoryDump.SaveToFile(stream, m_MemoryDump.m_sOriginalFilename);
							m_Parent.MemoryViewClosing(this);            
							return;
						}
					}

					saveToolStripMenuItem_Click(null,null);
				}
			}

			m_Settings.m_iDimensions[(int)DimensionValues.Alloc_Name] = listView1.Columns[0].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Alloc_Start] = listView1.Columns[1].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Alloc_End] = listView1.Columns[2].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Alloc_Size] = listView1.Columns[3].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Alloc_Tag] = listView1.Columns[4].Width;

			m_Settings.m_iDimensions[(int)DimensionValues.Callstack_Name] = listView2.Columns[0].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Callstack_File] = listView2.Columns[1].Width;
			m_Settings.m_iDimensions[(int)DimensionValues.Callstack_Line] = listView2.Columns[2].Width;

			m_Settings.m_iDimensions[(int)DimensionValues.FragView_VSplit] = this.Height - splitContainer1.SplitterDistance;
			m_Settings.m_iDimensions[(int)DimensionValues.FragView_HSplit] = this.Width - splitContainer2.SplitterDistance;

			m_NodeView.SetParameters(m_Parent, null, null, m_Settings, -1);

            m_Parent.MemoryViewClosing(this);
		}

		internal void DisplaySelectedTrackerIdAllocsInAllocView()
		{
			SelectBlock(0, 0xffffffff, true);						
		}

		// Perform Sorting on Column Headers
		private void listView1_ColumnClick(object sender, System.Windows.Forms.ColumnClickEventArgs e)
		{
			// Determine if clicked column is already the column that is being sorted.
			if (e.Column == m_lvwItemComparer.SortColumn)
			{
				// Reverse the current sort direction for this column.
				if (m_lvwItemComparer.Order == SortOrder.Ascending)
				{
					m_lvwItemComparer.Order = SortOrder.Descending;
				}
				else
				{
					m_lvwItemComparer.Order = SortOrder.Ascending;
				}
			}
			else
			{
				// Set the column number that is to be sorted; default to ascending.
				m_lvwItemComparer.SortColumn = e.Column;
				m_lvwItemComparer.Order = SortOrder.Ascending;
			}

			// Perform the sort with these new sort options.
			this.listView1.Sort();
		}

		private void MemoryFragmentationView_Activated(object sender, EventArgs e)
		{
			UpdateNodeView();
		}
	}

	public class AugmentedListViewItem : ListViewItem
	{
		public AugmentedListViewItem(string[] items, int _iAllocation, int _iSize) :
			base(items)
		{
			iAllocation = _iAllocation;
			iSize = _iSize;
		}

		public int iAllocation;
		public int iSize;
	}

	// This class is an implementation of the 'IComparer' interface.
	public class ListViewItemComparer : IComparer
	{
		// Specifies the column to be sorted
		private int ColumnToSort;

		// Specifies the order in which to sort (i.e. 'Ascending').
		private SortOrder OrderOfSort;

		// Case insensitive comparer object
		private CaseInsensitiveComparer ObjectCompare;

		// Class constructor, initializes various elements
		public ListViewItemComparer()
		{
			// Initialize the column to '0'
			ColumnToSort = 0;

			// Initialize the sort order to 'none'
			OrderOfSort = SortOrder.None;

			// Initialize the CaseInsensitiveComparer object
			ObjectCompare = new CaseInsensitiveComparer();
		}

		// This method is inherited from the IComparer interface.
		public int Compare(object x, object y)
		{
			int compareResult;
			ListViewItem listviewX, listviewY;

			// Cast the objects to be compared to ListViewItem objects
			listviewX = (ListViewItem)x;
			listviewY = (ListViewItem)y;

			if (ColumnToSort != 3)
			{
				// Case insensitive Compare
				compareResult = ObjectCompare.Compare (
					listviewX.SubItems[ColumnToSort].Text,
					listviewY.SubItems[ColumnToSort].Text
				);
			}
			else
			{
				compareResult = ((AugmentedListViewItem)listviewX).iSize - ((AugmentedListViewItem)listviewY).iSize;
			}

			// Calculate correct return value based on object comparison
			if (OrderOfSort == SortOrder.Ascending)
			{
				// Ascending sort is selected, return normal result of compare operation
				return compareResult;
			}
			else if (OrderOfSort == SortOrder.Descending)
			{
				// Descending sort is selected, return negative result of compare operation
				return (-compareResult);
			}
			else
			{
				// Return '0' to indicate they are equal
				return 0;
			}
		}

		// Gets or sets the number of the column to which to
		// apply the sorting operation (Defaults to '0').
		public int SortColumn
		{
			set
			{
				ColumnToSort = value;
			}
			get
			{
				return ColumnToSort;
			}
		}

		// Gets or sets the order of sorting to apply
		// (for example, 'Ascending' or 'Descending').
		public SortOrder Order
		{
			set
			{
				OrderOfSort = value;
			}
			get
			{
				return OrderOfSort;
			}
		}
	}
}
