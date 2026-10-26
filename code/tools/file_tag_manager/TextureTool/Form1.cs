using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using FreeImageAPI;
using System.Runtime.InteropServices;
using folderSelect;
using System.IO;
using CustomFolderBrowser;
//using CustomListView;
using System.Diagnostics;
using XmlHelperFunctions;
using System.Xml;
using RegexpHelpers;

namespace TextureTool
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.ImageList imageList1;
		private CustomFolderBrowser.CustomFolderBrowser customFolderBrowser1;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem setRootFolder;
		private CustomListView customListView1;

		Bitmap m_bitmap;
		private ActorSystem.ActorSystemForm actorSystemForm1;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Panel leftPanel;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Button applyButton;
		private System.Windows.Forms.Splitter splitter3;
		private System.Windows.Forms.Panel panelListview;
		private System.Windows.Forms.Splitter splitter2;
		private System.Windows.Forms.Panel panelFilter;
		private System.Windows.Forms.GroupBox groupBoxFilter;
		private System.Windows.Forms.ComboBox comboBox1;
		Hashtable m_imageTypeLookup;

		[DllImport("Gdi32.dll")]
		private static extern int SetStretchBltMode(IntPtr HDC, int iStretchMode);

		[DllImport("Gdi32.dll")]
		private static extern int StretchDIBits(
			IntPtr HDC,
			int XDest,                    // x-coord of destination upper-left corner
			int YDest,                    // y-coord of destination upper-left corner
			int nDestWidth,               // width of destination rectangle
			int nDestHeight,              // height of destination rectangle
			int XSrc,                     // x-coord of source upper-left corner
			int YSrc,                     // y-coord of source upper-left corner
			int nSrcWidth,                // width of source rectangle
			int nSrcHeight,               // height of source rectangle
			IntPtr lpBits,								// bitmap bits
			BITMAPINFO lpBitsInfo,				// bitmap data
			uint iUsage,                  // usage options
			int dwRop											// raster operation code
			);


		[DllImport("gdi32.dll")]
		public static extern bool DeleteObject(IntPtr hObject);

		public Form1()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			customFolderBrowser1.TreeAfterSelect += new CustomFolderBrowser.CustomFolderBrowser.TreeAfterSelectDelegate(customFolderBrowser1_TreeAfterSelect);
			customListView1.ListViewItemActivate += new CustomListView.ListViewItemActivateDelegate(customListView1_ListViewItemActivate);
			customListView1.ListViewItemClick += new CustomListView.ListViewItemClickDelegate(customListView1_ListViewItemClick);
			customListView1.ListViewSelectedIndexChanged +=new CustomListView.ListViewSelectedIndexChangedDelegate(customListView1_ListViewSelectedIndexChanged);


			actorSystemForm1.ActorNodeChanged += new ActorSystem.ActorSystemForm.DelegateActorNodeChanged(actorSystemForm1_ActorNodeChanged);

			m_imageTypeLookup = new Hashtable();

			m_imageTypeLookup.Add(".tga",	FIF.FIF_TARGA);
			m_imageTypeLookup.Add(".tiff",	FIF.FIF_TIFF);
			m_imageTypeLookup.Add(".bmp",	FIF.FIF_BMP);
			m_imageTypeLookup.Add(".gif",	FIF.FIF_GIF);
			m_imageTypeLookup.Add(".jpg",	FIF.FIF_JPEG);
			m_imageTypeLookup.Add(".jpeg",	FIF.FIF_JPEG);
			m_imageTypeLookup.Add(".psd",	FIF.FIF_PSD);
			m_imageTypeLookup.Add(".ico",	FIF.FIF_ICO);

			actorSystemForm1.Initialise(@"FileTagManagerConfig.xml");
			actorSystemForm1.SetString("");
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.pictureBox1 = new System.Windows.Forms.PictureBox();
			this.imageList1 = new System.Windows.Forms.ImageList(this.components);
			this.customFolderBrowser1 = new CustomFolderBrowser.CustomFolderBrowser();
			this.splitter1 = new System.Windows.Forms.Splitter();
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.setRootFolder = new System.Windows.Forms.MenuItem();
			this.customListView1 = new TextureTool.CustomListView();
			this.actorSystemForm1 = new ActorSystem.ActorSystemForm();
			this.panel1 = new System.Windows.Forms.Panel();
			this.panel2 = new System.Windows.Forms.Panel();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.applyButton = new System.Windows.Forms.Button();
			this.leftPanel = new System.Windows.Forms.Panel();
			this.splitter3 = new System.Windows.Forms.Splitter();
			this.panelListview = new System.Windows.Forms.Panel();
			this.panelFilter = new System.Windows.Forms.Panel();
			this.groupBoxFilter = new System.Windows.Forms.GroupBox();
			this.comboBox1 = new System.Windows.Forms.ComboBox();
			this.splitter2 = new System.Windows.Forms.Splitter();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.leftPanel.SuspendLayout();
			this.panelListview.SuspendLayout();
			this.panelFilter.SuspendLayout();
			this.groupBoxFilter.SuspendLayout();
			this.SuspendLayout();
			// 
			// pictureBox1
			// 
			this.pictureBox1.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
			this.pictureBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pictureBox1.Location = new System.Drawing.Point(0, 416);
			this.pictureBox1.Name = "pictureBox1";
			this.pictureBox1.Size = new System.Drawing.Size(256, 257);
			this.pictureBox1.TabIndex = 0;
			this.pictureBox1.TabStop = false;
			// 
			// imageList1
			// 
			this.imageList1.ImageSize = new System.Drawing.Size(8, 8);
			this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// customFolderBrowser1
			// 
			this.customFolderBrowser1.Dock = System.Windows.Forms.DockStyle.Top;
			this.customFolderBrowser1.Location = new System.Drawing.Point(0, 0);
			this.customFolderBrowser1.Name = "customFolderBrowser1";
			this.customFolderBrowser1.Size = new System.Drawing.Size(256, 416);
			this.customFolderBrowser1.TabIndex = 1;
			// 
			// splitter1
			// 
			this.splitter1.Location = new System.Drawing.Point(256, 0);
			this.splitter1.Name = "splitter1";
			this.splitter1.Size = new System.Drawing.Size(3, 673);
			this.splitter1.TabIndex = 2;
			this.splitter1.TabStop = false;
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem1});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.setRootFolder});
			this.menuItem1.Text = "File";
			// 
			// setRootFolder
			// 
			this.setRootFolder.Index = 0;
			this.setRootFolder.Text = "Set root folder";
			this.setRootFolder.Click += new System.EventHandler(this.setRootFolder_Click);
			// 
			// customListView1
			// 
			this.customListView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.customListView1.Location = new System.Drawing.Point(0, 48);
			this.customListView1.Name = "customListView1";
			this.customListView1.Size = new System.Drawing.Size(200, 616);
			this.customListView1.TabIndex = 4;
			// 
			// actorSystemForm1
			// 
			this.actorSystemForm1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.actorSystemForm1.Location = new System.Drawing.Point(8, 56);
			this.actorSystemForm1.Name = "actorSystemForm1";
			this.actorSystemForm1.Size = new System.Drawing.Size(261, 609);
			this.actorSystemForm1.TabIndex = 7;
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.SystemColors.Control;
			this.panel1.Controls.Add(this.actorSystemForm1);
			this.panel1.Controls.Add(this.panel2);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(462, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(274, 673);
			this.panel1.TabIndex = 8;
			// 
			// panel2
			// 
			this.panel2.Controls.Add(this.groupBox1);
			this.panel2.Location = new System.Drawing.Point(0, 0);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(88, 40);
			this.panel2.TabIndex = 8;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.applyButton);
			this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox1.Location = new System.Drawing.Point(0, 0);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(88, 40);
			this.groupBox1.TabIndex = 1;
			this.groupBox1.TabStop = false;
			// 
			// applyButton
			// 
			this.applyButton.Location = new System.Drawing.Point(8, 8);
			this.applyButton.Name = "applyButton";
			this.applyButton.TabIndex = 0;
			this.applyButton.Text = "APPLY";
			this.applyButton.Click += new System.EventHandler(this.button1_Click);
			// 
			// leftPanel
			// 
			this.leftPanel.Controls.Add(this.splitter3);
			this.leftPanel.Controls.Add(this.pictureBox1);
			this.leftPanel.Controls.Add(this.customFolderBrowser1);
			this.leftPanel.Dock = System.Windows.Forms.DockStyle.Left;
			this.leftPanel.Location = new System.Drawing.Point(0, 0);
			this.leftPanel.Name = "leftPanel";
			this.leftPanel.Size = new System.Drawing.Size(256, 673);
			this.leftPanel.TabIndex = 8;
			// 
			// splitter3
			// 
			this.splitter3.Dock = System.Windows.Forms.DockStyle.Top;
			this.splitter3.Location = new System.Drawing.Point(0, 416);
			this.splitter3.Name = "splitter3";
			this.splitter3.Size = new System.Drawing.Size(256, 3);
			this.splitter3.TabIndex = 2;
			this.splitter3.TabStop = false;
			// 
			// panelListview
			// 
			this.panelListview.Controls.Add(this.panelFilter);
			this.panelListview.Controls.Add(this.customListView1);
			this.panelListview.Dock = System.Windows.Forms.DockStyle.Left;
			this.panelListview.Location = new System.Drawing.Point(259, 0);
			this.panelListview.Name = "panelListview";
			this.panelListview.Size = new System.Drawing.Size(200, 673);
			this.panelListview.TabIndex = 9;
			// 
			// panelFilter
			// 
			this.panelFilter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.panelFilter.Controls.Add(this.groupBoxFilter);
			this.panelFilter.Location = new System.Drawing.Point(0, 0);
			this.panelFilter.Name = "panelFilter";
			this.panelFilter.Size = new System.Drawing.Size(200, 48);
			this.panelFilter.TabIndex = 5;
			// 
			// groupBoxFilter
			// 
			this.groupBoxFilter.Controls.Add(this.comboBox1);
			this.groupBoxFilter.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBoxFilter.Location = new System.Drawing.Point(0, 0);
			this.groupBoxFilter.Name = "groupBoxFilter";
			this.groupBoxFilter.Size = new System.Drawing.Size(200, 48);
			this.groupBoxFilter.TabIndex = 0;
			this.groupBoxFilter.TabStop = false;
			this.groupBoxFilter.Text = "Filter";
			// 
			// comboBox1
			// 
			this.comboBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.comboBox1.Location = new System.Drawing.Point(8, 16);
			this.comboBox1.Name = "comboBox1";
			this.comboBox1.Size = new System.Drawing.Size(184, 21);
			this.comboBox1.TabIndex = 0;
			this.comboBox1.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.comboBox1_KeyPress);
			// 
			// splitter2
			// 
			this.splitter2.Location = new System.Drawing.Point(459, 0);
			this.splitter2.Name = "splitter2";
			this.splitter2.Size = new System.Drawing.Size(3, 673);
			this.splitter2.TabIndex = 10;
			this.splitter2.TabStop = false;
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(736, 673);
			this.Controls.Add(this.panel1);
			this.Controls.Add(this.splitter2);
			this.Controls.Add(this.panelListview);
			this.Controls.Add(this.splitter1);
			this.Controls.Add(this.leftPanel);
			this.Menu = this.mainMenu1;
			this.Name = "Form1";
			this.Text = "File Tag Manager";
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.groupBox1.ResumeLayout(false);
			this.leftPanel.ResumeLayout(false);
			this.panelListview.ResumeLayout(false);
			this.panelFilter.ResumeLayout(false);
			this.groupBoxFilter.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}

		private void setRootFolder_Click(object sender, System.EventArgs e)
		{
			try 
			{
				FolderSelect dlg = new FolderSelect();

				if ( dlg.ShowDialog() == DialogResult.OK)
				{
					DirectoryInfo info = dlg.info;
					this.customFolderBrowser1.SetRootFolder(dlg.fullPath);
				}
			}
			catch( Exception err)
			{
				Console.WriteLine(err.Message);
			}
		}


		XmlManager m_XmlFolderData;

		private void customFolderBrowser1_TreeAfterSelect(object sender, TreeViewEventArgs e)
		{
			RefreshListView();
		}

		void RefreshListView()
		{
//			CustomFolderBrowser.CustomFolderBrowser.TreeTagInfo tti = e.Node.Tag as CustomFolderBrowser.CustomFolderBrowser.TreeTagInfo;
			CustomFolderBrowser.CustomFolderBrowser.TreeTagInfo tti = this.customFolderBrowser1.CurrentSelectedItem.Tag as CustomFolderBrowser.CustomFolderBrowser.TreeTagInfo;
			this.customListView1.PopulateListView(tti.strPath, this.actorSystemForm1.Filemask, m_localFileMask);
			m_XmlFolderData = new XmlManager(tti.strPath + @"\TextureData.xml");
			ListView.ListViewItemCollection lvic = customListView1.listView1.Items;
			XmlNodeList textureNodes = m_XmlFolderData.xmlDoc.SelectNodes("/root/textures/texture");
			this.actorSystemForm1.SetString("");								// Clear the current actor panel.
			XmlNode defaultNode = this.actorSystemForm1.GetActorNode();			// Get the default actor.

			foreach(ListViewItem lvi in lvic)
			{
				if(textureNodes != null)
				{
					CustomListView.ListTagInfo lti = lvi.Tag as CustomListView.ListTagInfo;
					XmlNode texture = XmlHelper.FindNodeByKeyValue(textureNodes, "name", lti.m_fsi.Name);

					if(texture != null)
					{
						string strRefcount = XmlHelper.GetKeyValue(texture, "refcount");
						int refCount;

						if(strRefcount != null)
							refCount = Convert.ToInt32(strRefcount);
						else
							refCount = 0;

						refCount++;
						XmlHelper.AddAttribute(texture, "refcount", refCount.ToString());
					}
					else
					{
						texture = m_XmlFolderData.xmlDoc.CreateElement("texture");
						XmlHelper.AddAttribute(texture, "name", lti.m_fsi.Name);
						XmlHelper.AddAttribute(texture, "refcount", "0");
						XmlNode textureData = m_XmlFolderData.xmlDoc.ImportNode(defaultNode, true);
						texture.AppendChild(textureData.Clone());
					}
					lti.textureData = texture;
				}
			}

			XmlNode textures = m_XmlFolderData.xmlDoc.SelectSingleNode("/root/textures");
			textures.RemoveAll();

			foreach(ListViewItem lvi in lvic)
			{
				CustomListView.ListTagInfo lti = lvi.Tag as CustomListView.ListTagInfo;
				textures.AppendChild(lti.textureData.Clone());
			}
			if(m_XmlFolderData.dataIsWritable == true)
				m_XmlFolderData.SaveDataFile();

			Debug.WriteLine("Folder " + tti.strPath + "\n");
		}

		private void customListView1_ListViewItemActivate(object sender, EventArgs e)
		{
			Debug.WriteLine("Item activate\n");
		}


		Timer listViewTimer;
		ListViewItem listViewTimer_FocusedItem;

		private void customListView1_ListViewSelectedIndexChanged(object sender, EventArgs e)
		{
			ListView lv = sender as ListView;

			if(lv.FocusedItem == null)
			{
				Debug.WriteLine("Selected index changed. NO FOCUS\n");
				return;
			}
			else
				Debug.WriteLine("Selected index changed. FOCUS\n");

			if(listViewTimer == null)
			{
				listViewTimer = new Timer();
				listViewTimer.Tick += new EventHandler(ListViewTimerTick);
			}
			listViewTimer.Interval = 300;
			listViewTimer_FocusedItem = lv.FocusedItem;
			listViewTimer.Enabled = true;
			return;
		}

		private void ListViewTimerTick(object o, System.EventArgs e)
		{
			Debug.WriteLine("Tick");
			ListViewItem lvi = listViewTimer_FocusedItem;
			CustomListView.ListTagInfo lti = lvi.Tag as CustomListView.ListTagInfo;

			if(lti != null)
			{
				m_bitmap = GetImage(lti);

				this.actorSystemForm1.SetString(lti.textureData.InnerXml);

				SetCurrentImage(m_bitmap);

				if(m_bitmap != null)
				{
					if(lvi.ListView.LargeImageList.Images.Count <= lvi.Index)
						lvi.ListView.LargeImageList.Images.Add(m_bitmap);
					else
						lvi.ListView.LargeImageList.Images[lvi.Index] = m_bitmap;

					if(lvi.ListView.SmallImageList.Images.Count <= lvi.Index)
						lvi.ListView.SmallImageList.Images.Add(m_bitmap);
					else
						lvi.ListView.SmallImageList.Images[lvi.Index] = m_bitmap;

					lvi.ImageIndex = 0;
				}
			}
			Timer t = o as Timer;
			t.Stop();
			this.Invalidate(true);
		}


		private bool actorSystemForm1_ActorNodeChanged(XmlNode newActorNode)
		{
			Debug.WriteLine("Actor node has changed.");

			ListView.ListViewItemCollection lvic = customListView1.listView1.Items;

			if(m_XmlFolderData != null)
			{
				XmlNode textures = m_XmlFolderData.xmlDoc.SelectSingleNode("/root/textures");

				foreach(ListViewItem lvi in lvic)
				{
					if(lvi.Selected == true)
					{
						CustomListView.ListTagInfo lti = lvi.Tag as CustomListView.ListTagInfo;
						// First remove the texture we are replacing.
						XmlNode thisTexture = XmlHelper.FindNodeByKeyValue(textures.ChildNodes, "name", lti.m_fsi.Name);
						//					thisTexture.RemoveAll();
						thisTexture.RemoveChild(thisTexture.FirstChild);
						XmlNode textureData = m_XmlFolderData.xmlDoc.ImportNode(newActorNode, true);
						thisTexture.AppendChild(textureData.Clone());
						lti.textureData = thisTexture.Clone();
						//					lti.textureData = textureData.Clone();
					}
				}

				m_XmlFolderData.SaveDataFile();

				return true;
			}
			else
				return false;
		}

		private void customListView1_ListViewItemClick(object sender, EventArgs e)
		{
		}

		Bitmap GetImage(CustomListView.ListTagInfo lti)
		{
			string strExtension = lti.m_fsi.Extension.ToLower();
			if(m_imageTypeLookup.Contains(strExtension))
			{
				FIF imageType = (FIF)m_imageTypeLookup[strExtension];
				uint image = FreeImage.Load(imageType, lti.m_fsi.FullName, 0);

				if(image != 0)
				{
					try
					{
						BITMAPINFOHEADER header = FreeImage.GetInfoHeader(image);
						Bitmap bitmap = new Bitmap(header.width, header.height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
						Graphics g = System.Drawing.Graphics.FromImage(bitmap);
						IntPtr hdc = g.GetHdc();
						int r = SetStretchBltMode(hdc, 3 /* COLORONCOLOR */);
//						r = StretchDIBits(hdc, 0, 0, bitmap.Width, bitmap.Height, 0, 0, (int)FreeImage.GetWidth(image), (int)FreeImage.GetHeight(image), FreeImage.GetBits(image), FreeImage.GetInfo(image), 0 /* DIB_RGB_COLORS */, 0x00CC0020 /* SRCCOPY */);
						r = StretchDIBits(hdc, 0, 0, 256, 256, 0, 0, (int)FreeImage.GetWidth(image), (int)FreeImage.GetHeight(image), FreeImage.GetBits(image), FreeImage.GetInfo(image), 0 /* DIB_RGB_COLORS */, 0x00CC0020 /* SRCCOPY */);
						g.ReleaseHdc(hdc);
						FreeImage.Unload(image);
						return bitmap;
					}
					catch(Exception ex)
					{
						Debug.WriteLine(ex.ToString());
					}
				}
			}
			return null;
		}


		bool SetCurrentImage(Bitmap bitmap)
		{
			if(bitmap != null)
			{
				pictureBox1.Image = m_bitmap;
				return true;
			}
			else
			{
				pictureBox1.Image = null;
				return false;
			}
		}

		private void button1_Click(object sender, System.EventArgs e)
		{
			actorSystemForm1.ShowActorString();
		}

		string m_localFileMask = "";

		private void comboBox1_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r')
			{
				e.Handled = true;
				comboBox1.Items.Insert(0, comboBox1.Text);
				m_localFileMask = FilemaskHandler.ToRegexFromList(comboBox1.Text);
				RefreshListView();

				comboBox1.SelectionStart = 0;
				comboBox1.SelectionLength = comboBox1.Text.Length;

				if (comboBox1.Items.Count > 10)
					comboBox1.Items.RemoveAt(comboBox1.Items.Count-1);
			}
		}






	}



}
