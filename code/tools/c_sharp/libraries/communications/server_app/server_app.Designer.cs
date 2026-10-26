namespace server_app
{
	partial class ServerApp
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
			this.buttonCreateLocalClient = new System.Windows.Forms.Button();
			this.buttonClear = new System.Windows.Forms.Button();
			this.buttonCreateRemoteClient = new System.Windows.Forms.Button();
			this.tabControl = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.groupBox_Broadcast = new System.Windows.Forms.GroupBox();
			this.textBox_UdpBroadcast = new System.Windows.Forms.TextBox();
			this.button_Broadcast = new System.Windows.Forms.Button();
			this.richTextBox_UdpStrings = new System.Windows.Forms.RichTextBox();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.tabControl.SuspendLayout();
			this.groupBox_Broadcast.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.SuspendLayout();
			// 
			// buttonCreateLocalClient
			// 
			this.buttonCreateLocalClient.AutoSize = true;
			this.buttonCreateLocalClient.Location = new System.Drawing.Point(12, 12);
			this.buttonCreateLocalClient.Name = "buttonCreateLocalClient";
			this.buttonCreateLocalClient.Size = new System.Drawing.Size(101, 23);
			this.buttonCreateLocalClient.TabIndex = 0;
			this.buttonCreateLocalClient.Text = "Create local client";
			this.buttonCreateLocalClient.UseVisualStyleBackColor = true;
			this.buttonCreateLocalClient.Click += new System.EventHandler(this.buttonCreateLocalClient_Click);
			// 
			// buttonClear
			// 
			this.buttonClear.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonClear.Location = new System.Drawing.Point(617, 422);
			this.buttonClear.Name = "buttonClear";
			this.buttonClear.Size = new System.Drawing.Size(75, 23);
			this.buttonClear.TabIndex = 2;
			this.buttonClear.Text = "Clear";
			this.buttonClear.UseVisualStyleBackColor = true;
			this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
			// 
			// buttonCreateRemoteClient
			// 
			this.buttonCreateRemoteClient.AutoSize = true;
			this.buttonCreateRemoteClient.Location = new System.Drawing.Point(119, 12);
			this.buttonCreateRemoteClient.Name = "buttonCreateRemoteClient";
			this.buttonCreateRemoteClient.Size = new System.Drawing.Size(117, 23);
			this.buttonCreateRemoteClient.TabIndex = 3;
			this.buttonCreateRemoteClient.Text = "Create Remote Client";
			this.buttonCreateRemoteClient.UseVisualStyleBackColor = true;
			this.buttonCreateRemoteClient.Click += new System.EventHandler(this.buttonCreateRemoteClient_Click);
			// 
			// tabControl
			// 
			this.tabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.tabControl.Controls.Add(this.tabPage1);
			this.tabControl.Controls.Add(this.tabPage2);
			this.tabControl.Location = new System.Drawing.Point(12, 272);
			this.tabControl.Name = "tabControl";
			this.tabControl.SelectedIndex = 0;
			this.tabControl.Size = new System.Drawing.Size(680, 144);
			this.tabControl.TabIndex = 4;
			// 
			// tabPage1
			// 
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage1.Size = new System.Drawing.Size(672, 118);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "tabPage1";
			this.tabPage1.UseVisualStyleBackColor = true;
			// 
			// tabPage2
			// 
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage2.Size = new System.Drawing.Size(542, 248);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "tabPage2";
			this.tabPage2.UseVisualStyleBackColor = true;
			// 
			// groupBox_Broadcast
			// 
			this.groupBox_Broadcast.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox_Broadcast.Controls.Add(this.groupBox1);
			this.groupBox_Broadcast.Controls.Add(this.button_Broadcast);
			this.groupBox_Broadcast.Controls.Add(this.textBox_UdpBroadcast);
			this.groupBox_Broadcast.Location = new System.Drawing.Point(12, 41);
			this.groupBox_Broadcast.Name = "groupBox_Broadcast";
			this.groupBox_Broadcast.Size = new System.Drawing.Size(676, 225);
			this.groupBox_Broadcast.TabIndex = 5;
			this.groupBox_Broadcast.TabStop = false;
			this.groupBox_Broadcast.Text = "Broadcast";
			// 
			// textBox_UdpBroadcast
			// 
			this.textBox_UdpBroadcast.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.textBox_UdpBroadcast.Location = new System.Drawing.Point(6, 19);
			this.textBox_UdpBroadcast.Name = "textBox_UdpBroadcast";
			this.textBox_UdpBroadcast.Size = new System.Drawing.Size(581, 20);
			this.textBox_UdpBroadcast.TabIndex = 0;
			// 
			// button_Broadcast
			// 
			this.button_Broadcast.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button_Broadcast.Location = new System.Drawing.Point(593, 19);
			this.button_Broadcast.Name = "button_Broadcast";
			this.button_Broadcast.Size = new System.Drawing.Size(75, 23);
			this.button_Broadcast.TabIndex = 1;
			this.button_Broadcast.Text = "Broadcast";
			this.button_Broadcast.UseVisualStyleBackColor = true;
			this.button_Broadcast.Click += new System.EventHandler(this.button_Broadcast_Click);
			// 
			// richTextBox_UdpStrings
			// 
			this.richTextBox_UdpStrings.Dock = System.Windows.Forms.DockStyle.Fill;
			this.richTextBox_UdpStrings.Location = new System.Drawing.Point(3, 16);
			this.richTextBox_UdpStrings.Name = "richTextBox_UdpStrings";
			this.richTextBox_UdpStrings.Size = new System.Drawing.Size(656, 155);
			this.richTextBox_UdpStrings.TabIndex = 2;
			this.richTextBox_UdpStrings.Text = "";
			// 
			// groupBox1
			// 
			this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox1.Controls.Add(this.richTextBox_UdpStrings);
			this.groupBox1.Location = new System.Drawing.Point(6, 45);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(662, 174);
			this.groupBox1.TabIndex = 3;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Incoming UDP messages";
			// 
			// ServerApp
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(704, 454);
			this.Controls.Add(this.groupBox_Broadcast);
			this.Controls.Add(this.tabControl);
			this.Controls.Add(this.buttonCreateRemoteClient);
			this.Controls.Add(this.buttonClear);
			this.Controls.Add(this.buttonCreateLocalClient);
			this.MinimumSize = new System.Drawing.Size(712, 488);
			this.Name = "ServerApp";
			this.Text = "ServerApp";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
			this.tabControl.ResumeLayout(false);
			this.groupBox_Broadcast.ResumeLayout(false);
			this.groupBox_Broadcast.PerformLayout();
			this.groupBox1.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button buttonCreateLocalClient;
		private System.Windows.Forms.Button buttonClear;
		private System.Windows.Forms.Button buttonCreateRemoteClient;
		private System.Windows.Forms.TabControl tabControl;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.GroupBox groupBox_Broadcast;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.RichTextBox richTextBox_UdpStrings;
		private System.Windows.Forms.Button button_Broadcast;
		private System.Windows.Forms.TextBox textBox_UdpBroadcast;
	}
}

