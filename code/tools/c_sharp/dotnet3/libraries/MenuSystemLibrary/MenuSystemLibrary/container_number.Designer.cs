namespace MenuSystemLibrary
{
	partial class Container_Number
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
			this.textBox = new System.Windows.Forms.TextBox();
			this.label = new System.Windows.Forms.Label();
			this.trackBar = new System.Windows.Forms.TrackBar();
			this.buttonExpand = new System.Windows.Forms.Button();
			((System.ComponentModel.ISupportInitialize)(this.trackBar)).BeginInit();
			this.SuspendLayout();
			// 
			// textBox
			// 
			this.textBox.Location = new System.Drawing.Point(3, 16);
			this.textBox.Name = "textBox";
			this.textBox.Size = new System.Drawing.Size(100, 20);
			this.textBox.TabIndex = 1;
			this.textBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.textBox.TextChanged += new System.EventHandler(this.textBox_TextChanged);
			// 
			// label
			// 
			this.label.AutoEllipsis = true;
			this.label.AutoSize = true;
			this.label.ImageAlign = System.Drawing.ContentAlignment.TopLeft;
			this.label.Location = new System.Drawing.Point(3, 0);
			this.label.Name = "label";
			this.label.Size = new System.Drawing.Size(35, 13);
			this.label.TabIndex = 2;
			this.label.Text = "label1";
			// 
			// trackBar
			// 
			this.trackBar.Location = new System.Drawing.Point(136, 16);
			this.trackBar.Name = "trackBar";
			this.trackBar.Size = new System.Drawing.Size(180, 45);
			this.trackBar.TabIndex = 3;
			this.trackBar.TickStyle = System.Windows.Forms.TickStyle.None;
			this.trackBar.Scroll += new System.EventHandler(this.trackBar_Scroll);
			// 
			// buttonExpand
			// 
			this.buttonExpand.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.buttonExpand.Location = new System.Drawing.Point(109, 15);
			this.buttonExpand.Name = "buttonExpand";
			this.buttonExpand.Size = new System.Drawing.Size(21, 20);
			this.buttonExpand.TabIndex = 4;
			this.buttonExpand.Text = ">";
			this.buttonExpand.UseVisualStyleBackColor = true;
			this.buttonExpand.Click += new System.EventHandler(this.buttonExpand_Click);
			// 
			// Container_Number
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.textBox);
			this.Controls.Add(this.buttonExpand);
			this.Controls.Add(this.label);
			this.Controls.Add(this.trackBar);
			this.Name = "Container_Number";
			this.Size = new System.Drawing.Size(312, 42);
			this.Load += new System.EventHandler(this.Container_Number_Load);
			((System.ComponentModel.ISupportInitialize)(this.trackBar)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TextBox textBox;
		private System.Windows.Forms.Label label;
		private System.Windows.Forms.TrackBar trackBar;
		private System.Windows.Forms.Button buttonExpand;
	}
}
