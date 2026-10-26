namespace MenuSystemLibrary
{
	partial class Container_Filename
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
			this.label = new System.Windows.Forms.Label();
			this.textBox = new System.Windows.Forms.TextBox();
			this.buttonBrowse = new System.Windows.Forms.Button();
			this.labelPrefix = new System.Windows.Forms.Label();
			this.buttonOptions = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label
			// 
			this.label.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.label.AutoSize = true;
			this.label.Location = new System.Drawing.Point(3, 3);
			this.label.Name = "label";
			this.label.Size = new System.Drawing.Size(29, 13);
			this.label.TabIndex = 0;
			this.label.Text = "label";
			// 
			// textBox
			// 
			this.textBox.AllowDrop = true;
			this.textBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.textBox.Cursor = System.Windows.Forms.Cursors.IBeam;
			this.textBox.HideSelection = false;
			this.textBox.Location = new System.Drawing.Point(37, 20);
			this.textBox.Name = "textBox";
			this.textBox.Size = new System.Drawing.Size(221, 20);
			this.textBox.TabIndex = 1;
			this.textBox.TextChanged += new System.EventHandler(this.textBox_TextChanged);
			this.textBox.DragDrop += new System.Windows.Forms.DragEventHandler(this.textBox_DragDrop);
			this.textBox.DragEnter += new System.Windows.Forms.DragEventHandler(this.textBox_DragEnter);
			// 
			// buttonBrowse
			// 
			this.buttonBrowse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonBrowse.Location = new System.Drawing.Point(259, 20);
			this.buttonBrowse.Name = "buttonBrowse";
			this.buttonBrowse.Size = new System.Drawing.Size(24, 23);
			this.buttonBrowse.TabIndex = 2;
			this.buttonBrowse.Text = "...";
			this.buttonBrowse.UseVisualStyleBackColor = true;
			this.buttonBrowse.Click += new System.EventHandler(this.button_Click);
			// 
			// labelPrefix
			// 
			this.labelPrefix.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.labelPrefix.AutoSize = true;
			this.labelPrefix.Location = new System.Drawing.Point(3, 23);
			this.labelPrefix.Name = "labelPrefix";
			this.labelPrefix.Size = new System.Drawing.Size(33, 13);
			this.labelPrefix.TabIndex = 4;
			this.labelPrefix.Text = "Prefix";
			// 
			// buttonOptions
			// 
			this.buttonOptions.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonOptions.Image = global::MenuSystemLibrary.Properties.Resources.ExecuterToolButton;
			this.buttonOptions.Location = new System.Drawing.Point(283, 20);
			this.buttonOptions.Name = "buttonOptions";
			this.buttonOptions.Size = new System.Drawing.Size(23, 23);
			this.buttonOptions.TabIndex = 8;
			this.buttonOptions.UseVisualStyleBackColor = true;
			this.buttonOptions.Click += new System.EventHandler(this.buttonOptions_Click);
			// 
			// Container_Filename
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.buttonOptions);
			this.Controls.Add(this.textBox);
			this.Controls.Add(this.labelPrefix);
			this.Controls.Add(this.buttonBrowse);
			this.Controls.Add(this.label);
			this.Name = "Container_Filename";
			this.Size = new System.Drawing.Size(306, 45);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label;
		private System.Windows.Forms.TextBox textBox;
		private System.Windows.Forms.Button buttonBrowse;
		private System.Windows.Forms.Label labelPrefix;
		private System.Windows.Forms.Button buttonOptions;
	}
}
