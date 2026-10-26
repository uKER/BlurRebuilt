namespace application
{
	partial class Toolbar
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
			this.SuspendLayout();
			// 
			// panelClientArea
			// 
			this.panelClientArea.BackColor = System.Drawing.Color.Aquamarine;
			this.panelClientArea.Visible = false;
			// 
			// Toolbar
			// 
			this.BackColor = System.Drawing.Color.White;
			this.ClientSize = new System.Drawing.Size(335, 251);
			this.DoubleBuffered = true;
			this.Location = new System.Drawing.Point(0, 0);
			this.Name = "Toolbar";
			this.Load += new System.EventHandler(this.BizarreToolbar_Load);
			this.FrameFormEndDrag += new DragEventHandler(this.Toolbar_FrameFormEndDrag);
			this.FrameFormBeginDrag += new DragEventHandler(this.Toolbar_FrameFormBeginDrag);
			this.ResumeLayout(false);

		}

		#endregion
	}
}
