using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;

namespace ProfilerTool.Windows
{
    #region UI delegates
    public delegate void ResetDelegate();
    public delegate void UpdateStatusTextDelegate(string connected);
    public delegate void UpdateInitDataDelegate(Packets.InitData initData);
    public delegate void UpdateObjectDataDelegate(Packets.ObjectData objData);
    public delegate void UpdateValueDelegate(Packets.ValueData valData);
    public delegate void UpdateGetParamDelegate(Packets.GetParamData paramData);
    public delegate void UpdateAudioGraphDelegate(Packets.AudioGraphData audioGraphData);
    public delegate void UpdateAudioDelegate(Packets.AudioData audioData);
    public delegate void UpdateConnectionsDelegate(Packets.ConnectionData conData);
    public delegate void UpdateEventsDelegate(Packets.EventData evData);
    public delegate void UpdateErrorsDelegate(Packets.MessageData errData);
    public delegate void UpdateWarningsDelegate(Packets.MessageData warnData);
    #endregion

    public partial class MainWnd : Form
    {
        #region Member vars
        public Classes.Comms _comms;    //  TODO: not public
        private List<InstanceWnd> _lstInstanceWindows;
        private bool sendStats = false;
        public bool GroupComponents = false;
        #endregion

        /// <summary> 
        /// Constructor
        /// </summary>
        public MainWnd()
        {
            InitializeComponent();

            Packets.ObjectData.Init();

            _comms = new Classes.Comms(this);
            _lstInstanceWindows = new List<InstanceWnd>();

            tvObjects.Parent = this;
        }

        private void MainWnd_Load(object sender, EventArgs e)
        {
            TryToConnect(new Classes.Comms.Info());
        }

        private void tbcInstances_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (tbcInstances.SelectedTab != null)
            {
                foreach (InstanceWnd instWnd in _lstInstanceWindows)
                {
                    if (tbcInstances.SelectedTab.Text == instWnd.Text)
                    {
                        instWnd.AlignPages();
                        tvObjects.Select(tbcInstances.SelectedTab.Text);
                    }
                }
            }
        }

        #region UI inputs
        private void MainWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            _comms.Shutdown();
        }

        private void tbbConnect_Click(object sender, EventArgs e)
        {
            ConnectDlg();
        }

        private void connectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ConnectDlg();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        //void tabMain_SelectedIndexChanged(object sender, System.EventArgs e)
        //{
        //    TabControl ctrl = sender as TabControl;

        //    if (ctrl.SelectedIndex == 1)
        //    {
        //        sendStats = true;
        //        _comms.SendStatsRequest();
        //    }
        //    else
        //        sendStats = false;
        //}
        #endregion

        #region UI callbacks
        /// <summary> 
        /// Class to store comms info
        /// </summary>
        public void Reset()
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new ResetDelegate(Reset));
                return;
            }

            tvObjects.Nodes.Clear();

            foreach (InstanceWnd wnd in _lstInstanceWindows)
            {
                wnd.CloseAll();
                wnd.Dispose();
            }

            _lstInstanceWindows.Clear();
        }

        public void UpdateStatusText(string text)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateStatusTextDelegate(UpdateStatusText), new object[] { text });
                return;
            }

            sslblMain.Text = text;
        }

        public void UpdateInitData(Packets.InitData initData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateInitDataDelegate(UpdateInitData), new object[] { initData });
                return;
            }

            try
            {
                tvObjects.Create(initData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateObjectData(Packets.ObjectData objData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateObjectDataDelegate(UpdateObjectData), new object[] { objData });
                return;
            }

            try
            {
                tvObjects.AddObject(objData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateValue(Packets.ValueData valData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateValueDelegate(UpdateValue), new object[] { valData });
                return;
            }

            try
            {
                tvObjects.SetValue(valData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateGetParam(Packets.GetParamData paramData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateGetParamDelegate(UpdateGetParam), new object[] { paramData });
                return;
            }

            try
            {
                tvObjects.SetParamValue(paramData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateAudioGraph(Packets.AudioGraphData audioGraphData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateAudioGraphDelegate(UpdateAudioGraph), new object[] { audioGraphData });
                return;
            }

            try
            {
                tvObjects.SetAudioGraph(audioGraphData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateAudio(Packets.AudioData audioData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateAudioDelegate(UpdateAudio), new object[] { audioData });
                return;
            }

            try
            {
                tvObjects.SetAudio(audioData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateConnections(Packets.ConnectionData conData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateConnectionsDelegate(UpdateConnections), new object[] { conData });
                return;
            }

            try
            {
                tvObjects.SetConnections(conData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateEvents(Packets.EventData evData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateEventsDelegate(UpdateEvents), new object[] { evData });
                return;
            }

            try
            {
                tvObjects.DoMessage(evData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateErrors(Packets.MessageData errData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateErrorsDelegate(UpdateErrors), new object[] { errData });
                return;
            }

            try
            {
                tvObjects.DoError(errData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void UpdateWarnings(Packets.MessageData warnData)
        {
            if (InvokeRequired)
            {
                //  not UI thread, so invoke it
                BeginInvoke(new UpdateWarningsDelegate(UpdateWarnings), new object[] { warnData });
                return;
            }

            try
            {
                tvObjects.DoWarning(warnData);
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }
        #endregion

        public void ConnectDlg()
        {
            Windows.ConnectDlg dlg = new Windows.ConnectDlg();

            dlg.ShowDialog();

            if (dlg.m_bConnect)
                TryToConnect(dlg.m_ConnectInfo);
        }

        public void TryToConnect(Classes.Comms.Info info)
        {
            UpdateStatusText("trying to connect...");
            _comms.Connect(info);
        }

        public InstanceWnd CreateInstanceWindow(string name, UInt32 instHandle, UInt32 objHandle)
        {
            InstanceWnd instWnd = new InstanceWnd(this, name, instHandle, objHandle);

            //  add to list of instance windows
            _lstInstanceWindows.Add(instWnd);

            return instWnd;
        }

        public void SendObjectRequest(Controls.ObjectTreeNode node)
        {
            _comms.SendObjectRequest(node.ObjectHandle);
        }

        public void SendAudioGraphRequest(Controls.ObjectTreeNode node)
        {
            _comms.SendAudioGraphRequest(node.ObjectHandle);
        }

        public void ActivateInstance(Controls.InstanceTreeNode instNode)
        {
            InstanceWnd instWnd = instNode.InstanceWindow;

            tbcInstances.TabPages.Add(instWnd);
            instWnd.Parent = tbcInstances;
            tbcInstances.SelectedTab = instWnd;
        }

        public void DeactivateInstance(Controls.InstanceTreeNode instNode)
        {
            InstanceWnd instWnd = instNode.InstanceWindow;

            tbcInstances.TabPages.Remove(instWnd);
            instWnd.Parent = null;
        }

        public void ActivateComponent(Controls.ComponentTreeNode comNode)
        {
            TreeNode groupNode = comNode.Parent as TreeNode;
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (comNode.Parent.Text == "[Actions]")
                    {
                        instWnd.ActivateControl(InstanceWnd.PageId.Actions, comNode);
                    }
                    else if (comNode.Parent.Text == "[Parameters]")
                    {
                        instWnd.ActivateControl(InstanceWnd.PageId.Parameters, comNode);

                        //  request com data
                        _comms.SendGetParameterRequest(objNode.ObjectHandle, instNode.InstanceHandle, comNode.ComponentHandle);

                        //  set to pending a ParamIn
                        comNode.PendingParamIn = true;
                    }
                    else if (comNode.Parent.Text == "[Vectors]")
                    {
                        instWnd.ActivateControl(InstanceWnd.PageId.Vectors, comNode);

                        //  request com data
                        //_comms.SendGetParameterRequest(objNode.ObjectHandle, instNode.InstanceHandle, comNode.ComponentHandle);

                        //  set to pending a ParamIn
                        //comNode.PendingParamIn = true;
                    }
                    else if (comNode.Parent.Text == "[Graphs]")
                    {
                        instWnd.ActivateControl(InstanceWnd.PageId.Graphs, comNode);

                        //  request com data
                        _comms.SendComValueRequest(objNode.ObjectHandle, instNode.InstanceHandle, comNode.ComponentHandle, comNode.ComponentOutputHandle);

                        //  set to pending
                        comNode.Pending = true;
                    }
                    else if (comNode.Parent.Text == "[Audio]")
                    {
                        instWnd.ActivateControl(InstanceWnd.PageId.Audio, comNode);

                        //  request audio data
                        _comms.SendAudioInfoRequest(objNode.ObjectHandle, instNode.InstanceHandle, comNode.ComponentHandle);

                        //  set to pending
                        comNode.Pending = true;
                    }
                    else if (comNode.Parent.Text == "[Inputs]")
                    {
                        instWnd.ActivateControl(InstanceWnd.PageId.Inputs, comNode);

                        //  request audio data
                        _comms.SendConnectionInfoRequest(objNode.ObjectHandle, instNode.InstanceHandle, comNode.ComponentHandle, false);

                        //  set to pending
                        comNode.Pending = true;
                    }
                    else if (comNode.Parent.Text == "[Outputs]")
                    {
                        instWnd.ActivateControl(InstanceWnd.PageId.Outputs, comNode);

                        //  request audio data
                        _comms.SendConnectionInfoRequest(objNode.ObjectHandle, instNode.InstanceHandle, comNode.ComponentHandle, true);

                        //  set to pending
                        comNode.Pending = true;
                    }
                    else
                        MessageBox.Show("Unknown group in ActivateComponent", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

                    return;
                }
            }

            throw new Exception("Cannot find instance window");
        }

        public void DeactivateComponent(Controls.ComponentTreeNode comNode)
        {
            TreeNode groupNode = comNode.Parent as TreeNode;
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (comNode.Parent.Text == "[Actions]")
                    {
                        instWnd.DeactivateControl(InstanceWnd.PageId.Actions, comNode);
                    }
                    else if (comNode.Parent.Text == "[Parameters]")
                    {
                        instWnd.DeactivateControl(InstanceWnd.PageId.Parameters, comNode);
                    }
                    else if (comNode.Parent.Text == "[Vectors]")
                    {
                        instWnd.DeactivateControl(InstanceWnd.PageId.Vectors, comNode);
                    }
                    else if (comNode.Parent.Text == "[Graphs]")
                    {
                        instWnd.DeactivateControl(InstanceWnd.PageId.Graphs, comNode);
                    }
                    else if (comNode.Parent.Text == "[Audio]")
                    {
                        instWnd.DeactivateControl(InstanceWnd.PageId.Audio, comNode);
                    }
                    else if (comNode.Parent.Text == "[Inputs]")
                    {
                        instWnd.DeactivateControl(InstanceWnd.PageId.Inputs, comNode);
                    }
                    else if (comNode.Parent.Text == "[Outputs]")
                    {
                        instWnd.DeactivateControl(InstanceWnd.PageId.Outputs, comNode);
                    }
                    else
                        MessageBox.Show("Unknown group in DeactivateComponent", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                }
            }
        }

        public void ActivateGroup(TreeNode groupNode)
        {
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (groupNode.Text == "[Actions]")
                    {
                        instWnd.ActivatePage(InstanceWnd.PageId.Actions);
                    }
                    else if (groupNode.Text == "[Parameters]")
                    {
                        instWnd.ActivatePage(InstanceWnd.PageId.Parameters);
                    }
                    else if (groupNode.Text == "[Vectors]")
                    {
                        instWnd.ActivatePage(InstanceWnd.PageId.Vectors);
                    }
                    else if (groupNode.Text == "[Graphs]")
                    {
                        instWnd.ActivatePage(InstanceWnd.PageId.Graphs);
                    }
                    else if (groupNode.Text == "[Audio]")
                    {
                        instWnd.ActivatePage(InstanceWnd.PageId.Audio);
                    }
                    else if (groupNode.Text == "[Inputs]")
                    {
                        instWnd.ActivatePage(InstanceWnd.PageId.Inputs);
                    }
                    else if (groupNode.Text == "[Outputs]")
                    {
                        instWnd.ActivatePage(InstanceWnd.PageId.Outputs);
                    }
                    if (groupNode.Text == "[Messages]")
                    {
                        instWnd.ActivatePage(InstanceWnd.PageId.Messages);
                        _comms.SendEventInfoRequest(objNode.ObjectHandle, instNode.InstanceHandle, true);
                    }
                }
            }
        }

        public void DeactivateGroup(TreeNode groupNode)
        {
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (groupNode.Text == "[Actions]")
                    {
                        instWnd.DeactivatePage(InstanceWnd.PageId.Actions);
                    }
                    else if (groupNode.Text == "[Parameters]")
                    {
                        instWnd.DeactivatePage(InstanceWnd.PageId.Parameters);
                    }
                    else if (groupNode.Text == "[Vectors]")
                    {
                        instWnd.DeactivatePage(InstanceWnd.PageId.Vectors);
                    }
                    else if (groupNode.Text == "[Graphs]")
                    {
                        instWnd.DeactivatePage(InstanceWnd.PageId.Graphs);
                    }
                    else if (groupNode.Text == "[Audio]")
                    {
                        instWnd.DeactivatePage(InstanceWnd.PageId.Audio);
                    }
                    else if (groupNode.Text == "[Inputs]")
                    {
                        instWnd.DeactivatePage(InstanceWnd.PageId.Inputs);
                    }
                    else if (groupNode.Text == "[Outputs]")
                    {
                        instWnd.DeactivatePage(InstanceWnd.PageId.Outputs);
                    }
                    if (groupNode.Text == "[Messages]")
                    {
                        instWnd.DeactivatePage(InstanceWnd.PageId.Messages);
                        _comms.SendEventInfoRequest(objNode.ObjectHandle, instNode.InstanceHandle, false);
                    }
                }
            }
        }


        public void SetSelectedTab(Controls.InstanceTreeNode instNode)
        {
            tbcInstances.SelectedTab = instNode.InstanceWindow;
        }

        public void SetComponentValue(Controls.ComponentTreeNode comNode, Packets.ValueData valData)
        {
            TreeNode groupNode = comNode.Parent as TreeNode;
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (instNode.Checked)
                    {
                        //  forward to instance window
                        instWnd.SetComponentValue(comNode, valData);
                    }

                    return;
                }
            }

            throw new Exception("Cannot find instance window");
        }

        public void SetParameterControlValue(Controls.ComponentTreeNode comNode, Packets.GetParamData paramData)
        {
            TreeNode groupNode = comNode.Parent as TreeNode;
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (instNode.Checked)
                    {
                        //  forward to instance window
                        instWnd.SetParameterControlValue(comNode, paramData);
                    }

                    return;
                }
            }

            throw new Exception("Cannot find instance window");
        }

        public bool GetParameterControlValue(Controls.ComponentTreeNode comNode, ref float value)
        {
            TreeNode groupNode = comNode.Parent as TreeNode;
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (instNode.Checked)
                    {
                        //  forward to instance window
                        return instWnd.GetParameterControlValue(comNode, ref value);
                    }
                }
            }

            return false;
        }

        public void SetComponentAudio(Controls.ComponentTreeNode comNode, Packets.AudioData audioData)
        {
            TreeNode groupNode = comNode.Parent as TreeNode;
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (instNode.Checked)
                    {
                        //  forward to instance window
                        instWnd.SetComponentAudio(comNode, audioData);
                    }

                    return;
                }
            }

            throw new Exception("Cannot find instance window");
        }

        public void SetComponentConnection(Controls.ComponentTreeNode comNode, string thisInst, string thisComType, string thisComName, string thatInst, string thatComType, string thatComName, Packets.ConnectionData conData)
        {
            TreeNode groupNode = comNode.Parent as TreeNode;
            Controls.InstanceTreeNode instNode = groupNode.Parent as Controls.InstanceTreeNode;
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (instNode.Checked)
                    {
                        //  forward to instance window
                        instWnd.SetComponentConnection(thisInst, thisComType, thisComName, thatInst, thatComType, thatComName, conData.IsOutput);
                    }

                    return;
                }
            }

            throw new Exception("Cannot find instance window");
        }

        public void DoMessage(Controls.InstanceTreeNode instNode, Packets.EventData evData)
        {
            Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;
            Packets.ObjectData objData = objNode.ObjectData;

            string comName = objData.GetComponentNameFromHandle(evData.ComponentHandle);
            string msgName = objData.GetMessageNameFromHandle(evData.ComponentHandle, evData.MessageHandle);

            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instNode.InstanceHandle)
                {
                    if (instNode.Checked)
                    {
                        //  forward to instance window
                        instWnd.DoMessage(evData.Frame, objNode.ObjectName, instNode.InstanceName, comName, msgName);
                    }
                    else
                    {   
                        //  not supposed to be receiving events for this instance
                        //  possibly tool restarted? turn them off anyway...
                        _comms.SendEventInfoRequest(objNode.ObjectHandle, instNode.InstanceHandle, false);
                    }

                    return;
                }
            }

            throw new Exception("Cannot find instance window");
        }

        public InstanceWnd GetInstanceWindowFromHandle(UInt32 instHandle)
        {
            foreach (InstanceWnd instWnd in _lstInstanceWindows)
            {
                if (instWnd.InstanceHandle == instHandle)
                {
                    return instWnd;
                }
            }

            throw new Exception("Cannot find instance window");
        }

        public void DoError(Controls.InstanceTreeNode instNode, Packets.MessageData errData)
        {
            string errText = Packets.ObjectData.GetErrorTextFromHandle(errData.MessageHandle);

            if (instNode == null)
            {
                //  send to system event log
                Controls.ObjectTreeNode sysObjNode = tvObjects.Nodes[0] as Controls.ObjectTreeNode;
                Controls.InstanceTreeNode sysInstNode = sysObjNode.Nodes[0] as Controls.InstanceTreeNode;

                InstanceWnd instWnd = GetInstanceWindowFromHandle(sysInstNode.InstanceHandle);
                instWnd.DoError(errData.Frame, sysObjNode.ObjectName, sysInstNode.InstanceName, errText, "");
            }
            else
            {
                Controls.ObjectTreeNode sysObjNode = tvObjects.Nodes[0] as Controls.ObjectTreeNode;
                Controls.InstanceTreeNode sysInstNode = sysObjNode.Nodes[0] as Controls.InstanceTreeNode;

                Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;
                Packets.ObjectData objData = objNode.ObjectData;

                //  send to system event log
                InstanceWnd sysInstWnd = GetInstanceWindowFromHandle(sysInstNode.InstanceHandle);
                sysInstWnd.DoError(errData.Frame, sysObjNode.ObjectName, sysInstNode.InstanceName, errText, objNode.Text);

                //  and send to instance window event log
                InstanceWnd instWnd = GetInstanceWindowFromHandle(instNode.InstanceHandle);
                if (instNode.Checked)
                {
                    //  forward to instance window
                    instWnd.DoError(errData.Frame, objNode.ObjectName, instNode.InstanceName, errText, "");
                }
            }
        }

        public void DoWarning(Controls.InstanceTreeNode instNode, Packets.MessageData warnData)
        {
            string warnText = Packets.ObjectData.GetWarningTextFromHandle(warnData.MessageHandle);

            if (instNode == null)
            {
                //  send to system event log
                Controls.ObjectTreeNode sysObjNode = tvObjects.Nodes[0] as Controls.ObjectTreeNode;
                Controls.InstanceTreeNode sysInstNode = sysObjNode.Nodes[0] as Controls.InstanceTreeNode;

                InstanceWnd instWnd = GetInstanceWindowFromHandle(sysInstNode.InstanceHandle);
                instWnd.DoWarning(warnData.Frame, sysObjNode.ObjectName, sysInstNode.InstanceName, warnText, "");
            }
            else
            {
                Controls.ObjectTreeNode sysObjNode = tvObjects.Nodes[0] as Controls.ObjectTreeNode;
                Controls.InstanceTreeNode sysInstNode = sysObjNode.Nodes[0] as Controls.InstanceTreeNode;

                Controls.ObjectTreeNode objNode = instNode.Parent as Controls.ObjectTreeNode;
                Packets.ObjectData objData = objNode.ObjectData;

                //  send to system event log
                InstanceWnd sysInstWnd = GetInstanceWindowFromHandle(sysInstNode.InstanceHandle);
                sysInstWnd.DoWarning(warnData.Frame, sysObjNode.ObjectName, sysInstNode.InstanceName, warnText, objNode.Text);

                //  and send to instance window event log
                InstanceWnd instWnd = GetInstanceWindowFromHandle(instNode.InstanceHandle);
                if (instNode.Checked)
                {
                    //  forward to instance window
                    instWnd.DoWarning(warnData.Frame, objNode.ObjectName, instNode.InstanceName, warnText, "");
                }
            }
        }

        public void SendActionTrigger(UInt32 objHandle, UInt32 instHandle, string comName)
        {
            Controls.ObjectTreeNode objNode = tvObjects.GetObjectNodeFromHandle(objHandle);

            UInt16 comHandle = objNode.ObjectData.GetComponentHandleFromName(comName);

            _comms.SendActionTriggerRequest(objHandle, instHandle, comHandle);
        }

        public void SendParameterValue(UInt32 objHandle, UInt32 instHandle, string comName, float value, bool overridegame)
        {
            Controls.ObjectTreeNode objNode = tvObjects.GetObjectNodeFromHandle(objHandle);

            UInt16 comHandle = objNode.ObjectData.GetComponentHandleFromName(comName);

            _comms.SendSetParameterRequest(objHandle, instHandle, comHandle, value, overridegame);
        }

        public void SendAudioInfo(UInt32 objHandle, UInt32 instHandle, string comName)
        {
            Controls.ObjectTreeNode objNode = tvObjects.GetObjectNodeFromHandle(objHandle);

            UInt16 comHandle = objNode.ObjectData.GetComponentHandleFromName(comName);

            _comms.SendAudioInfoRequest(objHandle, instHandle, comHandle);
        }

        public void SendMuteEvent(UInt32 objHandle, UInt32 instHandle, string comName, bool mute)
        {
            Controls.ObjectTreeNode objNode = tvObjects.GetObjectNodeFromHandle(objHandle);

            UInt16 comHandle = objNode.ObjectData.GetComponentHandleFromName(comName);

            _comms.SendMuteEventRequest(objHandle, instHandle, comHandle, mute);
        }

        private void tbbRefresh_Click(object sender, EventArgs e)
        {
            _comms.SendInitializeRequest();
        }

        private void tbbLoadSnapshot_Click(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();

            dialog.Filter = "Mix files (*.mix)|*.mix|All files (*.*)|*.*";
            //dialog.InitialDirectory = initialDirectory;
            dialog.Title = "Select a mix file";

            if (dialog.ShowDialog() == DialogResult.OK)
            {
                //string value;
                StreamReader reader = new StreamReader(dialog.FileName);

                tvObjects.LoadMix(reader);

                //while ((line = sr.ReadLine()) != null)
                //    tvObjects.LoadValue(value);
            }
        }

        private void tbbSaveSnapshot_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();

            saveFileDialog1.Filter = "Mix files (*.mix)|*.mix";
            saveFileDialog1.Title = "Save a mix file";
            saveFileDialog1.ShowDialog();

            // If the file name is not an empty string open it for saving.
            if (saveFileDialog1.FileName != "")
            {
                FileStream file = File.Open(saveFileDialog1.FileName, FileMode.Create, FileAccess.Write);
                StreamWriter wtiter = new StreamWriter(file, System.Text.Encoding.UTF8);
                tvObjects.CreateMix(wtiter);
                wtiter.Close();
                file.Close();
            }
        }

        private void tbbGroupComponents_Click(object sender, EventArgs e)
        {
            GroupComponents = tbbGroupComponents.Checked;
        }
    }
}