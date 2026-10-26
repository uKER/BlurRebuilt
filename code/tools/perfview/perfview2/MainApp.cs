using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using WeifenLuo.WinFormsUI.Docking;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using JWC;
using Microsoft.Win32;

namespace ProfilingApplication
{
    public delegate void NetworkMessageObjectCallback(object message);
    public delegate void UpdateConnectionStatus(string connected);

    public partial class MainApp : Form
    {
		public MruStripMenuInline mruMenu;
		static string		mruRegKey = "SOFTWARE\\BizarreCreations\\PerfView";
        GraphView           m_GraphView;
        List<CaptureView>   m_CaptureViews = new List<CaptureView>();
		List<MemoryFragmentationView> m_MemoryFragmentationViews = new List<MemoryFragmentationView>();
        HierarchyView       m_HierarchyView = new HierarchyView();
		MemoryNodeView		m_MemoryNodeView = new MemoryNodeView();
        UserSettings        m_Settings;
		ReceivingDataDlg	m_RcvDlg = new ReceivingDataDlg();

        private DeserializeDockContent m_deserializeDockContent;

        public MainApp()
        {
            m_TestCaptureData = new CaptureData();
            m_NetworkCommunication = new NetworkCommunication(this);
            m_deserializeDockContent = new DeserializeDockContent(GetContentFromPersistString);

			InitializeComponent();

			mruMenu = new MruStripMenuInline(fileToolStripMenuItem, menuRecentFile, new MruStripMenu.ClickedHandler(OnMruFile), mruRegKey + "\\MRU");
        }

        private IDockContent GetContentFromPersistString(string persistString)
        {
            if (persistString == typeof(HierarchyView).ToString())
                return m_HierarchyView;
			else if (persistString == typeof(MemoryNodeView).ToString())
				return m_MemoryNodeView;
            else
                return null;
        }

        CaptureData m_TestCaptureData;
        NetworkCommunication m_NetworkCommunication;

        // do a capture
        private void toolStripButton1_Click(object sender, EventArgs e)
        {
            if (m_NetworkCommunication.SendMessage("EventList"))
				m_RcvDlg.ShowDialog();
        }

		private void toolStripButton3_Click_1(object sender, EventArgs e)
		{
			if (m_NetworkCommunication.SendMessage("Allocations"))
				m_RcvDlg.ShowDialog();
		}

        public void MessageCallback(object message)
        {
			if (this.IsDisposed)
				return;
            Invoke(new NetworkMessageObjectCallback(ReceiveNetworkMessageObject), message);
        }

        public void UpdateConnectionStatus(string bConnected)
        {
            Invoke(new UpdateConnectionStatus(UpdateConnectionStatusInternal), bConnected);
        }

        protected void UpdateConnectionStatusInternal(string connected)
        {
			m_RcvDlg.Hide();
            connectedStatusLabel1.Text = connected;
			bool bConnected = connected.Contains("Connected to");
			graphViewToolStripMenuItem1.Enabled = 
			toolStripButton5.Enabled = 
			timingCaptureToolStripMenuItem.Enabled = 
			toolStripButton1.Enabled =
			memoryCaptureToolStripMenuItem.Enabled = 
			toolStripButton3.Enabled = bConnected;
        }

        public void ReceiveNetworkMessageObject(object message)
        {
            if (message.GetType() == typeof(CaptureData))
            {
                CaptureView cv = new CaptureView((CaptureData)message, this, m_Settings, m_HierarchyView);
                cv.MdiParent = this;
                m_CaptureViews.Add(cv);
                cv.Show();
				m_RcvDlg.Hide();
            }
			else if (message.GetType() == typeof(MemoryTrackerData))
			{
				MemoryFragmentationView cv = new MemoryFragmentationView((MemoryTrackerData)message, this, m_Settings, m_MemoryNodeView);
				cv.MdiParent = this;
				m_MemoryFragmentationViews.Add(cv);
				cv.Show();
				m_RcvDlg.Hide();
			}
			else if (message.GetType() == typeof(CounterValues))
			{
				if (m_GraphView != null)
					m_GraphView.SetReceivedCounterList((CounterValues)message);
			}
			else if (message.GetType() == typeof(List<string>))
			{
				CounterPropertiesDlg dlg = new CounterPropertiesDlg();

				dlg.SetCounterData((List<string>)message, m_Settings.m_CounterPropertiesConfig);

				DialogResult r = dlg.ShowDialog();
				if (r == DialogResult.OK)
				{
					m_Settings.m_CounterPropertiesConfig = dlg.GetCounterData();

					m_GraphView.UpdateCounterPropertiesConfig(m_Settings.m_CounterPropertiesConfig);
				}
			}
			else if (message.GetType() == typeof(string))
			{
				// display the network string in a dialog box
				m_RcvDlg.Hide();
				MessageBox.Show((string)message, "Network Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
        }

        private void toolStripButton2_Click(object sender, EventArgs e)
        {
            m_HierarchyView.Show(m_DockPanel, DockState.DockRight);
        }

		private void toolStripButton4_Click(object sender, EventArgs e)
		{
			m_MemoryNodeView.Show(m_DockPanel, DockState.DockRight);
		}

        private void MainApp_Load(object sender, EventArgs e)
        {
            m_HierarchyView.MdiParent = MdiParent;
			m_MemoryNodeView.MdiParent = MdiParent;
            try
            {
                m_Settings = UserSettings.Load(Application.UserAppDataPath + "\\perfview_settings.xml");

                this.WindowState = m_Settings.m_MainAppState;
                if (WindowState == FormWindowState.Normal)
                {
                    this.Location = m_Settings.m_MainAppLocation;
                    this.Size = m_Settings.m_MainAppSize;
                }
            }
            catch(System.IO.IOException)
            {
                m_Settings = new UserSettings();
            }
            string configFile = Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), "DockPanel.config");

            if (File.Exists(configFile))
                m_DockPanel.LoadFromXml(configFile, m_deserializeDockContent);

            m_DockPanel.DocumentStyle = DocumentStyle.SystemMdi;
			m_HierarchyView.SetUserSettings(m_Settings);
			m_MemoryNodeView.SetUserSettings(m_Settings);

            m_NetworkCommunication.Connect(m_Settings.m_ConnectInfo);
        }

        private void MainApp_FormClosing(object sender, FormClosingEventArgs e)
        {
            m_Settings.m_MainAppState = this.WindowState;
            m_Settings.m_MainAppLocation = this.Location;
            m_Settings.m_MainAppSize = this.Size;
			m_HierarchyView.SaveSettings();
			m_MemoryNodeView.SaveSettings();

            string configFile = Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), "DockPanel.config");
            m_DockPanel.SaveAsXml(configFile);

            m_NetworkCommunication.Shutdown();

            UserSettings.Save(Application.UserAppDataPath + "\\perfview_settings.xml", m_Settings);

			mruMenu.SaveToRegistry();
        }

        public void CaptureViewClosing(CaptureView view)
        {
            m_CaptureViews.Remove(view);
        }

		public void MemoryViewClosing(MemoryFragmentationView view)
		{
			m_MemoryFragmentationViews.Remove(view);
		}

        public void UpdateBarColoursOnAllCaptureView()
        {
            foreach (CaptureView view in m_CaptureViews)
                view.UpdateBarColours(m_Settings);
        }

		private void OnMruFile(int number, String filename)
		{
			Stream stream = System.IO.File.OpenRead(filename);
			if (OpenFile(stream, filename))
			{
				mruMenu.SetFirstFile(number);
			}
			else
			{
				mruMenu.RemoveFile(number);
			}
		}

		private bool OpenFile(Stream stream, string sOriginalFilename)
		{
            //serialize the object in binary format.
			if (stream != null)
			{
                  //Check if it is a .perf file
                if (Path.GetExtension(sOriginalFilename) == ".perf")
                {
                    CaptureData capture = new CaptureData();
                    capture.Read(stream);

                    ReceiveNetworkMessageObject(capture);
                }
                else
                {
                    PerfViewFile capture = null;
                    BinaryFormatter formatter = new BinaryFormatter();
                    try
                    {
                        capture = (PerfViewFile)formatter.Deserialize(stream);
                        if (sOriginalFilename != null)
                        {
                            capture.m_sOriginalFilename = sOriginalFilename;
                            capture.m_bSaved = true;
                        }
                    }
                    catch (Exception)
                    {
                        stream.Close();
                        return false;
                    }
                    stream.Close();

                    capture.InitialiseUnserialisables();
                    ReceiveNetworkMessageObject(capture);
                }
				return true;
			}
			return false;
		}

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Displays a an open file dialog so the user can open a capture
            OpenFileDialog openFileDialog1 = new OpenFileDialog();
            openFileDialog1 .Filter = "All Captures(*.pvcap;*.mcap;*.perf)|*.pvcap;*.mcap;*.perf|Timing Capture (*.pvcap)|*.pvcap|Memory Capture (*.mcap)|*.mcap|Raw Perf Capture (*.perf)|*.perf";
            openFileDialog1 .Title = "Open a Capture";
            openFileDialog1 .ShowDialog();

            // If the file name is not an empty string open it for saving.
            if(openFileDialog1.FileName != "")
            {
				mruMenu.AddFile(openFileDialog1.FileName);

				Stream stream = openFileDialog1.OpenFile();

				OpenFile(stream, openFileDialog1.FileName);
            }
        }

		private void timingCaptureToolStripMenuItem_Click(object sender, EventArgs e)
		{
			toolStripButton1_Click(sender, e);
		}

		private void memoryCaptureToolStripMenuItem_Click(object sender, EventArgs e)
		{
			toolStripButton3_Click_1(sender, e);
		}

		private void connectToToolStripMenuItem_Click_1(object sender, EventArgs e)
		{
            ConnectionDialog dlg = new ConnectionDialog(m_Settings.m_CustomConnectionConfig);

            dlg.ShowDialog();

            if (dlg.m_bConnect)
            {
                m_Settings.m_ConnectInfo = dlg.m_ConnectInfo;
                m_NetworkCommunication.Connect(dlg.m_ConnectInfo);
                UpdateConnectionStatusInternal("Offline");
            }
		}

		private void graphViewToolStripMenuItem1_Click(object sender, EventArgs e)
		{
			toolStripButton5_Click(sender, e);
		}

		private void toolStripButton5_Click(object sender, EventArgs e)
		{
			if (m_GraphView == null)
			{
				m_GraphView = new GraphView(this, m_Settings, m_NetworkCommunication);
				m_GraphView.MdiParent = this;
			}
			m_GraphView.Show();
		}

		internal void NotifyGraphViewClosing()
		{
			m_GraphView = null;			
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}
	}
}
