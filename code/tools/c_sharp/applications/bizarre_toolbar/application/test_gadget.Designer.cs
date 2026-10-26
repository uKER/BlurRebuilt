namespace application
{
	partial class TestGadget
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
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TestGadget));
			this.panel1 = new System.Windows.Forms.Panel();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.label1 = new System.Windows.Forms.Label();
			this.buttonHello = new System.Windows.Forms.Button();
			this.timerTest = new System.Windows.Forms.Timer(this.components);
			this.panel1.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.Color.SteelBlue;
			this.panel1.Controls.Add(this.groupBox1);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(145, 96);
			this.panel1.TabIndex = 0;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.buttonHello);
			this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox1.Location = new System.Drawing.Point(0, 0);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(145, 96);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "groupBox1";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(7, 49);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(35, 13);
			this.label1.TabIndex = 1;
			this.label1.Text = "label1";
			// 
			// buttonHello
			// 
			this.buttonHello.Location = new System.Drawing.Point(6, 19);
			this.buttonHello.Name = "buttonHello";
			this.buttonHello.Size = new System.Drawing.Size(75, 23);
			this.buttonHello.TabIndex = 0;
			this.buttonHello.Text = "Hello?";
			this.buttonHello.UseVisualStyleBackColor = true;
			this.buttonHello.Click += new System.EventHandler(this.buttonHello_Click);
			// 
			// timerTest
			// 
			this.timerTest.Enabled = true;
			this.timerTest.Interval = 1000;
			this.timerTest.Tick += new System.EventHandler(this.timerTest_Tick);
			// 
			// TestGadget
			// 
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Inherit;
			this.BackColor = System.Drawing.Color.Gold;
			this.BoundsF = ((System.Drawing.RectangleF)(resources.GetObject("$this.BoundsF")));
			this.ClientSize = new System.Drawing.Size(145, 96);
			this.Controls.Add(this.panel1);
			this.Name = "TestGadget";
			this.panel1.ResumeLayout(false);
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Button buttonHello;
		private System.Windows.Forms.Timer timerTest;
		private System.Windows.Forms.Label label1;
	}
}
