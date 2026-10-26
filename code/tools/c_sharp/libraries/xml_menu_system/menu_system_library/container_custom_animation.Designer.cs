namespace MenuSystemLibrary
{
	partial class Container_CustomAnimation
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
			this.checkBoxDirectoryFromNamingConvention = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
			// 
			// checkBoxDirectoryFromNamingConvention
			// 
			this.checkBoxDirectoryFromNamingConvention.AutoSize = true;
			this.checkBoxDirectoryFromNamingConvention.Location = new System.Drawing.Point(3, 4);
			this.checkBoxDirectoryFromNamingConvention.Name = "checkBoxDirectoryFromNamingConvention";
			this.checkBoxDirectoryFromNamingConvention.Size = new System.Drawing.Size(243, 17);
			this.checkBoxDirectoryFromNamingConvention.TabIndex = 2;
			this.checkBoxDirectoryFromNamingConvention.Text = "Directory from naming convention in config file";
			this.checkBoxDirectoryFromNamingConvention.UseVisualStyleBackColor = true;
			this.checkBoxDirectoryFromNamingConvention.CheckStateChanged += new System.EventHandler(this.checkBoxDirectoryFromNamingConvention_CheckStateChanged);
			// 
			// Container_CustomAnimation
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.checkBoxDirectoryFromNamingConvention);
			this.Name = "Container_CustomAnimation";
			this.Size = new System.Drawing.Size(248, 25);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.CheckBox checkBoxDirectoryFromNamingConvention;
	}
}
