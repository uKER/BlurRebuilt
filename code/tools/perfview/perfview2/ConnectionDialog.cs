using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;

namespace ProfilingApplication
{
    public partial class ConnectionDialog : Form
    {

        public ConnectionDialog(CustomLastConnectionInfo customLastConnectionInfo)
        {
            InitializeComponent();

            m_iMode = ConnectionModes.e_Local;
            radioButton1.Checked = true;
            comboBox2.Text = "Xbox";

            UpdateDialogStuff();

            m_CustomLastConnectionInfo = customLastConnectionInfo;
            if (m_CustomLastConnectionInfo != null)
            {
                textBox1.Text = m_CustomLastConnectionInfo.m_IpAddress;
                textBox2.Text = m_CustomLastConnectionInfo.m_Port;
            }

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

        private void button1_Click(object sender, EventArgs e)
        {
            m_bConnect = false;
            
            Close();
        }

        private void Connect_Click(object sender, EventArgs e)
        {
            m_bConnect = true;
            m_ConnectInfo = new ConnectionMethodInfo();
            m_ConnectInfo.m_bConnectToXbox = false;
            m_ConnectInfo.m_iPortNumber = 6317;

            switch (m_iMode)
            {
            case ConnectionModes.e_Custom:
                m_ConnectInfo.m_iPortNumber = uint.Parse(textBox2.Text);
                IPAddress addr = System.Net.IPAddress.Parse(textBox1.Text);
                byte[] bytes = addr.GetAddressBytes();
                m_ConnectInfo.m_iIPAddress = ((uint)bytes[0] << 24) + ((uint)bytes[1] << 16) + ((uint)bytes[2] << 8) + ((uint)bytes[3]);

                m_CustomLastConnectionInfo.m_IpAddress = textBox1.Text;
                m_CustomLastConnectionInfo.m_Port = textBox2.Text;

                break;
            case ConnectionModes.e_Host:
                break;
            case ConnectionModes.e_Local:
                if (comboBox2.Text == "Pc")
                    m_ConnectInfo.m_iIPAddress = 0x7f000001;
                else
                    m_ConnectInfo.m_bConnectToXbox = true;
                break;
        }

            Close();
        }

        public bool m_bConnect;
        public ConnectionMethodInfo m_ConnectInfo;
        public CustomLastConnectionInfo m_CustomLastConnectionInfo;

    }

    public class ConnectionMethodInfo
    {
        public ConnectionMethodInfo()
        {
            m_bConnectToXbox = false;
            m_iIPAddress = (uint)0xc0a800e6;
            m_iPortNumber = 6317;
        }

        public uint m_iIPAddress;
        public uint m_iPortNumber;
        public bool m_bConnectToXbox;
    };

    public class CustomLastConnectionInfo
    {
        public CustomLastConnectionInfo()
        {
            m_IpAddress = "127.0.0.1";
            m_Port = "6317";
        }
        public string m_IpAddress;
        public string m_Port;
    };


}