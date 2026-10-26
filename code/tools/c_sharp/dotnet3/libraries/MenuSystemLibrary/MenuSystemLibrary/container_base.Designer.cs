namespace MenuSystemLibrary
{
	partial class Container_Base
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.toolTip = new System.Windows.Forms.ToolTip(this.components);
			this.SuspendLayout();
			// 
			// toolTip
			// 
			this.toolTip.AutomaticDelay = 50;
			this.toolTip.AutoPopDelay = 0;
			this.toolTip.InitialDelay = 50;
			this.toolTip.ReshowDelay = 10;
			this.toolTip.ShowAlways = true;
			// 
			// Container_Base
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Name = "Container_Base";
			this.Size = new System.Drawing.Size(558, 303);
			this.Enter += new System.EventHandler(this.Container_Base_Enter);
			this.Load += new System.EventHandler(this.Container_Base_Load);
			this.ClientSizeChanged += new System.EventHandler(this.Container_Base_ClientSizeChanged);
			this.ResumeLayout(false);

		}

		#endregion

		public System.Windows.Forms.ToolTip toolTip;















	}
}
