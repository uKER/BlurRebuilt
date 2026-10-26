namespace MenuSystemLibrary
{
	partial class Container_GetDefault360
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
			this.textBox_360Name = new System.Windows.Forms.TextBox();
			this.buttonRefresh = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// textBox_360Name
			// 
			this.textBox_360Name.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.textBox_360Name.HideSelection = false;
			this.textBox_360Name.Location = new System.Drawing.Point(0, 2);
			this.textBox_360Name.MaxLength = 50;
			this.textBox_360Name.Name = "textBox_360Name";
			this.textBox_360Name.ReadOnly = true;
			this.textBox_360Name.Size = new System.Drawing.Size(105, 20);
			this.textBox_360Name.TabIndex = 0;
			this.textBox_360Name.TextChanged += new System.EventHandler(this.textBox_360Name_TextChanged);
			// 
			// buttonRefresh
			// 
			this.buttonRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonRefresh.Location = new System.Drawing.Point(110, 1);
			this.buttonRefresh.Name = "buttonRefresh";
			this.buttonRefresh.Size = new System.Drawing.Size(75, 23);
			this.buttonRefresh.TabIndex = 1;
			this.buttonRefresh.Text = "Refresh";
			this.buttonRefresh.UseVisualStyleBackColor = true;
			// 
			// Container_GetDefault360
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.buttonRefresh);
			this.Controls.Add(this.textBox_360Name);
			this.Name = "Container_GetDefault360";
			this.Size = new System.Drawing.Size(189, 25);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TextBox textBox_360Name;
		private System.Windows.Forms.Button buttonRefresh;
	}
}
