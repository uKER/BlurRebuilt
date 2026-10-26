namespace MenuSystemLibrary
{
	partial class Container_Dropdown
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
			this.comboBox = new System.Windows.Forms.ComboBox();
			this.groupChildren = new System.Windows.Forms.GroupBox();
			this.groupBox = new System.Windows.Forms.GroupBox();
			this.groupBox.SuspendLayout();
			this.SuspendLayout();
			// 
			// comboBox
			// 
			this.comboBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.comboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBox.FormattingEnabled = true;
			this.comboBox.Location = new System.Drawing.Point(6, 19);
			this.comboBox.Name = "comboBox";
			this.comboBox.Size = new System.Drawing.Size(426, 21);
			this.comboBox.TabIndex = 0;
			this.comboBox.SelectedIndexChanged += new System.EventHandler(this.comboBox_SelectedIndexChanged);
			// 
			// groupChildren
			// 
			this.groupChildren.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.groupChildren.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.groupChildren.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.groupChildren.ForeColor = System.Drawing.SystemColors.ControlText;
			this.groupChildren.Location = new System.Drawing.Point(6, 46);
			this.groupChildren.MinimumSize = new System.Drawing.Size(10, 50);
			this.groupChildren.Name = "groupChildren";
			this.groupChildren.Size = new System.Drawing.Size(426, 208);
			this.groupChildren.TabIndex = 1;
			this.groupChildren.TabStop = false;
			this.groupChildren.Text = "Children";
			// 
			// groupBox
			// 
			this.groupBox.Controls.Add(this.groupChildren);
			this.groupBox.Controls.Add(this.comboBox);
			this.groupBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox.Location = new System.Drawing.Point(0, 0);
			this.groupBox.Name = "groupBox";
			this.groupBox.Size = new System.Drawing.Size(438, 260);
			this.groupBox.TabIndex = 1;
			this.groupBox.TabStop = false;
			// 
			// Container_Dropdown
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.groupBox);
			this.Name = "Container_Dropdown";
			this.Size = new System.Drawing.Size(438, 260);
			this.groupBox.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.ComboBox comboBox;
		private System.Windows.Forms.GroupBox groupChildren;
		private System.Windows.Forms.GroupBox groupBox;

	}
}
