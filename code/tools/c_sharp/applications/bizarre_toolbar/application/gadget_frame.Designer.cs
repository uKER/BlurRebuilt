namespace application
{
	partial class GadgetFrame
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
			if (disposing && (components != null))
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(GadgetFrame));
			this.SuspendLayout();
			// 
			// GadgetFrame
			// 
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
			this.BoundsF = ((System.Drawing.RectangleF)(resources.GetObject("$this.BoundsF")));
			this.ClientSize = new System.Drawing.Size(335, 251);
			this.Name = "GadgetFrame";
			this.Move += new System.EventHandler(this.GadgetFrame_Move);
			this.FrameFormEndDrag += new DragEventHandler(this.GadgetFrame_FrameFormEndDrag);
			this.Resize += new System.EventHandler(this.GadgetFrame_Resize);
			this.FrameFormBeginDrag += new DragEventHandler(this.GadgetFrame_FrameFormBeginDrag);
			this.ResumeLayout(false);

		}

		#endregion

	}
}
