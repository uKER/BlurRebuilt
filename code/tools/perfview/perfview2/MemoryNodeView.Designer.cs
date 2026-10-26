namespace ProfilingApplication
{
	partial class MemoryNodeView
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
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MemoryNodeView));
			this.treeList = new SynapticEffect.Forms.TreeListView();
			this.SuspendLayout();
			// 
			// treeList
			// 
			this.treeList.BackColor = System.Drawing.SystemColors.Window;
			this.treeList.ColumnSortColor = System.Drawing.Color.Gainsboro;
			this.treeList.ColumnTrackColor = System.Drawing.Color.WhiteSmoke;
			this.treeList.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treeList.GridLineColor = System.Drawing.Color.WhiteSmoke;
			this.treeList.HeaderMenu = null;
			this.treeList.ItemHeight = 20;
			this.treeList.ItemMenu = null;
			this.treeList.LabelEdit = false;
			this.treeList.Location = new System.Drawing.Point(0, 0);
			this.treeList.Name = "treeList";
			this.treeList.RowSelectColor = System.Drawing.SystemColors.Highlight;
			this.treeList.RowTrackColor = System.Drawing.Color.WhiteSmoke;
			this.treeList.Size = new System.Drawing.Size(436, 545);
			this.treeList.SmallImageList = null;
			this.treeList.StateImageList = null;
			this.treeList.TabIndex = 0;
			this.treeList.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.treeList_MouseDoubleClick);
			this.treeList.OnSelectionChangedEvent += new SynapticEffect.Forms.OnSelectionChanged(this.treeList_OnSelectionChangedEvent);
			// 
			// MemoryNodeView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(436, 545);
			this.Controls.Add(this.treeList);
			this.HideOnClose = true;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "MemoryNodeView";
			this.TabText = "MemoryNodeView";
			this.Text = "MemoryNodeView";
			this.ResumeLayout(false);

		}

		#endregion

		private SynapticEffect.Forms.TreeListView treeList;
	}
}