using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;
using System.Xml;
using System.Text.RegularExpressions;

namespace TextureTool
{
	/// <summary>
	/// Summary description for UserControl1.
	/// </summary>
	public class CustomListView : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.Panel panel1;
		internal System.Windows.Forms.ListView listView1;
		private System.Windows.Forms.ImageList imageList1;
		private System.ComponentModel.IContainer components;

		public CustomListView()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitComponent call

		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if( components != null )
					components.Dispose();
			}
			base.Dispose( disposing );
		}

		public class ListTagInfo
		{
			public ListTagInfo(FileSystemInfo fsi)
			{
				m_fsi = fsi;
			}
			public FileSystemInfo m_fsi;
			public XmlNode textureData;
		}

		public bool PopulateListView(string strPath, string strFilemask1, string strFilemask2)
		{
			try
			{
				DirectoryInfo di = new DirectoryInfo(strPath);

				this.listView1.Items.Clear();

				FileInfo[] files = di.GetFiles();

				foreach (FileInfo fiNext in files)
				{
					// TODO: Check it against filemask.

					bool allowFile = true;

					if(strFilemask1 != null)
					{
						Regex r = new Regex(strFilemask1);

						Match m = r.Match(fiNext.Name.ToLower(), 0);

						if(m.Success == true)
						{
							if(m.Index == 0 && m.Length == fiNext.Name.Length)
								allowFile = m.Success;
							else
								allowFile = false;
						}
						else
							allowFile = false;
					}

					if(allowFile == true)
					{
						if(strFilemask2 != null && strFilemask2 != "")
						{
							Regex r = new Regex(strFilemask2);

							Match m = r.Match(fiNext.Name.ToLower(), 0);

							if(m.Success == true)
							{
								if(m.Index == 0 && m.Length == fiNext.Name.Length)
									allowFile = m.Success;
								else
									allowFile = false;
							}
							else
								allowFile = false;
						}
					}

					if(allowFile == true)
					{
						if((fiNext.Attributes & FileAttributes.Hidden) != FileAttributes.Hidden)
						{
							ListViewItem lvi = new ListViewItem(fiNext.Name);
							ListTagInfo lti = new ListTagInfo(fiNext);
							lvi.Tag = lti;
							this.listView1.Items.Add(lvi);

						}
					}
				}
				return true;
			}
			catch
			{
				return false;
			}
		}

		#region Component Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.panel1 = new System.Windows.Forms.Panel();
			this.listView1 = new System.Windows.Forms.ListView();
			this.imageList1 = new System.Windows.Forms.ImageList(this.components);
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.listView1);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(232, 256);
			this.panel1.TabIndex = 0;
			// 
			// listView1
			// 
			this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listView1.GridLines = true;
			this.listView1.HideSelection = false;
			this.listView1.LargeImageList = this.imageList1;
			this.listView1.Location = new System.Drawing.Point(0, 0);
			this.listView1.Name = "listView1";
			this.listView1.Size = new System.Drawing.Size(232, 256);
			this.listView1.SmallImageList = this.imageList1;
			this.listView1.TabIndex = 0;
			this.listView1.View = System.Windows.Forms.View.List;
			this.listView1.Click += new System.EventHandler(this.listView1_Click);
			this.listView1.ItemActivate += new System.EventHandler(this.listView1_ItemActivate);
			this.listView1.Enter += new System.EventHandler(this.listView1_Enter);
			this.listView1.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
			// 
			// imageList1
			// 
			this.imageList1.ColorDepth = System.Windows.Forms.ColorDepth.Depth32Bit;
			this.imageList1.ImageSize = new System.Drawing.Size(8, 8);
			this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// CustomListView
			// 
			this.Controls.Add(this.panel1);
			this.Name = "CustomListView";
			this.Size = new System.Drawing.Size(232, 256);
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		public delegate void ListViewItemActivateDelegate(object sender, System.EventArgs e);
		[RefreshProperties(RefreshProperties.Repaint), Description("After Select Description here."), Category("Keefs Events"), Browsable(true)]
		public event ListViewItemActivateDelegate ListViewItemActivate;
		private void listView1_ItemActivate(object sender, System.EventArgs e)
		{
			if(ListViewItemActivate != null)
			{
				ListViewItemActivate(sender, e);
			}
		}

		public delegate void ListViewItemClickDelegate(object sender, System.EventArgs e);
		[RefreshProperties(RefreshProperties.Repaint), Description("After Select Description here."), Category("Keefs Events"), Browsable(true)]
		public event ListViewItemClickDelegate ListViewItemClick;
		private void listView1_Click(object sender, System.EventArgs e)
		{
			if(ListViewItemClick != null)
				ListViewItemClick(sender, e);		
		}

		public delegate void ListViewSelectedIndexChangedDelegate(object sender, System.EventArgs e);
		[RefreshProperties(RefreshProperties.Repaint), Description("After SelectedIndexChanged Description here."), Category("Keefs Events"), Browsable(true)]
		public event ListViewSelectedIndexChangedDelegate ListViewSelectedIndexChanged;
		private void listView1_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if(ListViewSelectedIndexChanged != null)
				ListViewSelectedIndexChanged(sender, e);
		}

		private void listView1_Enter(object sender, System.EventArgs e)
		{
			Debug.WriteLine("Enter.\n");
		}
	}
}

