using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;

namespace ProfilerTool.Windows
{
    public class ConnectDlg : Form
    {
        public ConnectDlg()
        {
            InitializeComponent();

            m_iMode = ConnectionModes.e_Local;
            radioButton1.Checked = true;
            comboBox2.Text = "Xbox";

            UpdateDialogStuff();
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            m_iMode = ConnectionModes.e_Local;
            UpdateDialogStuff();
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            m_iMode = ConnectionModes.e_Host;
            UpdateDialogStuff();
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            m_iMode = ConnectionModes.e_Custom;
            UpdateDialogStuff();
        }

        enum ConnectionModes
        {
            e_Local,
            e_Host,
            e_Custom
        };

        private void UpdateDialogStuff()
        {
            comboBox2.Enabled = (m_iMode == ConnectionModes.e_Local);
            comboBox1.Enabled = (m_iMode == ConnectionModes.e_Host);
            textBox1.Enabled = (m_iMode == ConnectionModes.e_Custom);
            textBox2.Enabled = (m_iMode == ConnectionModes.e_Custom);
        }

        ConnectionModes m_iMode;

        public bool m_bConnect;
        private ProfilerTool.Controls.Joystick panel1;
        public Classes.Comms.Info m_ConnectInfo;

        private void InitializeComponent()
        {
            this.button1 = new System.Windows.Forms.Button();
            this.Connect = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.comboBox2 = new System.Windows.Forms.ComboBox();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.radioButton3 = new System.Windows.Forms.RadioButton();
            this.radioButton2 = new System.Windows.Forms.RadioButton();
            this.radioButton1 = new System.Windows.Forms.RadioButton();
            this.panel1 = new ProfilerTool.Controls.Joystick();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(233, 216);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 14;
            this.button1.Text = "Cancel";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // Connect
            // 
            this.Connect.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.Connect.Location = new System.Drawing.Point(152, 216);
            this.Connect.Name = "Connect";
            this.Connect.Size = new System.Drawing.Size(75, 23);
            this.Connect.TabIndex = 12;
            this.Connect.Text = "&Connect";
            this.Connect.UseVisualStyleBackColor = true;
            this.Connect.Click += new System.EventHandler(this.Connect_Click_1);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(32, 42);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(46, 13);
            this.label4.TabIndex = 24;
            this.label4.Text = "Method:";
            // 
            // comboBox2
            // 
            this.comboBox2.FormattingEnabled = true;
            this.comboBox2.Items.AddRange(new object[] {
            "Pc",
            "Xbox"});
            this.comboBox2.Location = new System.Drawing.Point(89, 39);
            this.comboBox2.Name = "comboBox2";
            this.comboBox2.Size = new System.Drawing.Size(121, 21);
            this.comboBox2.TabIndex = 23;
            // 
            // comboBox1
            // 
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(89, 98);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(219, 21);
            this.comboBox1.TabIndex = 22;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(35, 101);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(32, 13);
            this.label3.TabIndex = 21;
            this.label3.Text = "Host:";
            // 
            // textBox2
            // 
            this.textBox2.Location = new System.Drawing.Point(244, 164);
            this.textBox2.Name = "textBox2";
            this.textBox2.Size = new System.Drawing.Size(64, 20);
            this.textBox2.TabIndex = 20;
            this.textBox2.Text = "6317";
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(89, 165);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(100, 20);
            this.textBox1.TabIndex = 19;
            this.textBox1.Text = "127.0.0.1";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(209, 168);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(29, 13);
            this.label2.TabIndex = 18;
            this.label2.Text = "Port:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(35, 164);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(48, 13);
            this.label1.TabIndex = 17;
            this.label1.Text = "Address:";
            // 
            // radioButton3
            // 
            this.radioButton3.AutoSize = true;
            this.radioButton3.Location = new System.Drawing.Point(16, 135);
            this.radioButton3.Name = "radioButton3";
            this.radioButton3.Size = new System.Drawing.Size(60, 17);
            this.radioButton3.TabIndex = 16;
            this.radioButton3.TabStop = true;
            this.radioButton3.Text = "Custom";
            this.radioButton3.UseVisualStyleBackColor = true;
            this.radioButton3.CheckedChanged += new System.EventHandler(this.radioButton3_CheckedChanged);
            // 
            // radioButton2
            // 
            this.radioButton2.AutoSize = true;
            this.radioButton2.Location = new System.Drawing.Point(16, 78);
            this.radioButton2.Name = "radioButton2";
            this.radioButton2.Size = new System.Drawing.Size(62, 17);
            this.radioButton2.TabIndex = 15;
            this.radioButton2.TabStop = true;
            this.radioButton2.Text = "Remote";
            this.radioButton2.UseVisualStyleBackColor = true;
            this.radioButton2.CheckedChanged += new System.EventHandler(this.radioButton2_CheckedChanged);
            // 
            // radioButton1
            // 
            this.radioButton1.AutoSize = true;
            this.radioButton1.Location = new System.Drawing.Point(16, 16);
            this.radioButton1.Name = "radioButton1";
            this.radioButton1.Size = new System.Drawing.Size(51, 17);
            this.radioButton1.TabIndex = 13;
            this.radioButton1.TabStop = true;
            this.radioButton1.Text = "Local";
            this.radioButton1.UseVisualStyleBackColor = true;
            this.radioButton1.CheckedChanged += new System.EventHandler(this.radioButton1_CheckedChanged);
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.SystemColors.Control;
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Location = new System.Drawing.Point(61, 376);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(205, 195);
            this.panel1.TabIndex = 25;
            this.panel1.X = 0F;
            this.panel1.Y = 0F;
            this.panel1.Z = 0F;
            // 
            // ConnectDlg
            // 
            this.ClientSize = new System.Drawing.Size(322, 256);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.Connect);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.comboBox2);
            this.Controls.Add(this.comboBox1);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.textBox2);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.radioButton3);
            this.Controls.Add(this.radioButton2);
            this.Controls.Add(this.radioButton1);
            this.MaximumSize = new System.Drawing.Size(330, 290);
            this.Name = "ConnectDlg";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Connection";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        private Button button1;
        private Button Connect;
        private Label label4;
        private ComboBox comboBox2;
        private ComboBox comboBox1;
        private Label label3;
        private TextBox textBox2;
        private TextBox textBox1;
        private Label label2;
        private Label label1;
        private RadioButton radioButton3;
        private RadioButton radioButton2;
        private RadioButton radioButton1;

        private void Connect_Click_1(object sender, EventArgs e)
        {
            m_bConnect = true;
            m_ConnectInfo = new Classes.Comms.Info();
            m_ConnectInfo._isXbox = false;
            m_ConnectInfo._portNum = 6317;

            switch (m_iMode)
            {
                case ConnectionModes.e_Custom:
                    m_ConnectInfo._portNum = uint.Parse(textBox2.Text);
                    IPAddress addr = System.Net.IPAddress.Parse(textBox1.Text);
                    byte[] bytes = addr.GetAddressBytes();
                    m_ConnectInfo._ipAddress = ((uint)bytes[0] << 24) + ((uint)bytes[1] << 16) + ((uint)bytes[2] << 8) + ((uint)bytes[3]);
                    break;
                case ConnectionModes.e_Host:
                    break;
                case ConnectionModes.e_Local:
                    if (comboBox2.Text == "Pc")
                        m_ConnectInfo._ipAddress = 0x7f000001;
                    else
                        m_ConnectInfo._isXbox = true;
                    break;
            }

            Close();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}