namespace file_tag_manager_2
{
	partial class ListViewManager
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{

			if(disposing && (components != null))
			{
				if (m_populateThread != null)
					m_populateThread.Abort();

				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
			this.toolStripContainer1.SuspendLayout();
			this.SuspendLayout();
			// 
			// toolStripContainer1
			// 
			// 
			// toolStripContainer1.ContentPanel
			// 
			this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(150, 150);
			this.toolStripContainer1.Location = new System.Drawing.Point(0, 0);
			this.toolStripContainer1.Name = "toolStripContainer1";
			this.toolStripContainer1.Size = new System.Drawing.Size(150, 175);
			this.toolStripContainer1.TabIndex = 0;
			this.toolStripContainer1.Text = "toolStripContainer1";
			// 
			// ListViewManager
			// 
			this.AllowColumnReorder = true;
			this.FullRowSelect = true;
			this.Name = "list_view_manager";
			this.Size = new System.Drawing.Size(345, 472);
			this.SelectedIndexChanged += new System.EventHandler(this.ListViewManager_SelectedIndexChanged);
			this.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.ListViewManager_ColumnClick);
			this.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.ListViewManager_ItemSelectionChanged);
			this.toolStripContainer1.ResumeLayout(false);
			this.toolStripContainer1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.ToolStripContainer toolStripContainer1;
	}
}
