namespace MenuSystemLibrary
{
	partial class Container_Bool
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
			this.checkBox = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
			// 
			// checkBox
			// 
			this.checkBox.AutoSize = true;
			this.checkBox.Location = new System.Drawing.Point(3, 3);
			this.checkBox.Name = "checkBox";
			this.checkBox.Size = new System.Drawing.Size(80, 17);
			this.checkBox.TabIndex = 0;
			this.checkBox.Text = "checkBox1";
			this.checkBox.UseVisualStyleBackColor = true;
			this.checkBox.CheckStateChanged += new System.EventHandler(this.checkBox_CheckStateChanged);
			// 
			// Container_Bool
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.BackColor = System.Drawing.SystemColors.Control;
			this.Controls.Add(this.checkBox);
			this.Name = "Container_Bool";
			this.Size = new System.Drawing.Size(195, 24);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.CheckBox checkBox;
	}
}
