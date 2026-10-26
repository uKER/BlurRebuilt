using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Windows
{
    public partial class InstanceWnd : TabPage
    {
        //  add any new page types here
        public enum PageId
        {
            Audio,
            Actions,
            Parameters,
            Vectors,
            Graphs,
            PLFs,
            Inputs,
            Outputs,
            Messages,

            NumPages
        };

        private MainWnd _parent;
        private UInt32 _instHandle;
        private UInt32 _objHandle;
        private Pages.Page[] _pages;

        public InstanceWnd(MainWnd parent, string name, UInt32 instHandle, UInt32 objHandle)
        {
            InitializeComponent();

            _parent = parent;

            Name = name;
            Text = name;
            InstanceHandle = instHandle;
            ObjectHandle = objHandle;

            _pages = new Pages.Page[(int)PageId.NumPages];

            //  create the pages
            AddPage(PageId.Audio, new Pages.AudioPage(this), "Audio");
            AddPage(PageId.Actions, new Pages.ActionsPage(this), "Actions");
            AddPage(PageId.Parameters, new Pages.ParametersPage(this), "Parameters");
            AddPage(PageId.Vectors, new Pages.VectorsPage(this), "Vectors");
            AddPage(PageId.Graphs, new Pages.GraphPage(this), "Graphs");
            AddPage(PageId.PLFs, new Pages.Page(this), "PiecewiseLinearFunctions");
            AddPage(PageId.Inputs, new Pages.ConnectionsPage(this), "Inputs");
            AddPage(PageId.Outputs, new Pages.ConnectionsPage(this), "Outputs");

            if (objHandle == 0xffffffff)    //  is the system object, so last column is instance
                AddPage(PageId.Messages, new Pages.EventPage(this, "Object"), "Messages");
            else
                AddPage(PageId.Messages, new Pages.EventPage(this, "Component"), "Messages");
        }

        public MainWnd ParentWindow
        {
            get { return _parent; }
            set { _parent = value; }
        }

        public UInt32 InstanceHandle
        {
            get { return _instHandle; }
            set { _instHandle = value; }
        }

        public UInt32 ObjectHandle
        {
            get { return _objHandle; }
            set { _objHandle = value; }
        }

        private void Page_Resized(object sender, System.EventArgs e)
        {
            AlignPages();

            for (int page = 0; page < (int)PageId.NumPages; page++)
                _pages[page].AlignControls();
        }

        private void Page_FormClosing(object sender, System.EventArgs e)
        {
            Pages.PageWindow wnd = sender as Pages.PageWindow;

            wnd.Page.Windowed = false;

            TabControl parent = (TabControl)Parent;
            parent.SelectedTab = this;

            Controls.Add(wnd.Page);

            AlignPages();

            wnd.Page.AlignControls();
        }

        public void AlignPages()
        {
            SuspendLayout();

            int height = 0;
            for (int i = 0; i < (int)PageId.NumPages; i++)
            {
                if (!_pages[i].Windowed)
                {
                    _pages[i].Location = new Point(0, height);
                    height += _pages[i].PageHeight;
                    _pages[i].Size = new Size(Size.Width, _pages[i].PageHeight);
                }

                if (_pages[i].Windowed)
                {
                    _pages[i].Window.ClientSize = new Size(_pages[i].Size.Width, _pages[i].Size.Height);
                }
            }

            ResumeLayout(true);
        }

        public void ToggleDock(string name)
        {
            foreach (Pages.Page p in _pages)
            {
                if ("[" + p.Name + "]" == name)
                {
                    if (!p.Windowed)
                    {
                        p.Window = new Pages.PageWindow(p);
                        p.Window.Text = name + " (" + Name + ")";
                        p.Window.FormClosing += new FormClosingEventHandler(Page_FormClosing);
                        p.Windowed = true;
                        p.Window.Show();
                    }
                    else
                    {
                        p.Window.Close();
                    }

                    p.AlignControls();
                }
            }

            AlignPages();
        }

        private string GetComponentText(Controls.ComponentTreeNode comNode)
        {
            if (comNode.ComponentOutput == "")
                return comNode.ComponentName;
            else
                return string.Format("{0:s}.{1:s}", comNode.ComponentName, comNode.ComponentOutput);
        }

        private void AddPage(PageId pageID, Pages.Page page, string name)
        {
            page.Name = name;
            _pages[(int)pageID] = page;
            Controls.Add(page);

            _pages[(int)pageID].Dock = DockStyle.None;
            _pages[(int)pageID].BorderStyle = BorderStyle.Fixed3D;
            _pages[(int)pageID].Show();
        }

        public void AddControl(PageId pageId, Controls.ComponentTreeNode comNode)
        {
            _pages[(int)pageId].AddControl(comNode);
        }

        public void ActivateControl(PageId pageId, Controls.ComponentTreeNode comNode)
        {
            _pages[(int)pageId].Activate(GetComponentText(comNode));

            AlignPages();
        }

        public void DeactivateControl(PageId pageId, Controls.ComponentTreeNode comNode)
        {
            _pages[(int)pageId].Deactivate(GetComponentText(comNode));

            AlignPages();
        }

        public void ActivatePage(PageId pageId)
        {
            _pages[(int)pageId].Maximize();

            AlignPages();
        }

        public void DeactivatePage(PageId pageId)
        {
            _pages[(int)pageId].Minimize();

            AlignPages();
        }

        public void CloseAll()
        {
            foreach (Pages.Page p in _pages)
            {
                if (p.Window != null)
                    p.Window.Close();
            }
        }

        public void SetComponentValue(Controls.ComponentTreeNode comNode, Packets.ValueData valData)
        {
            //  set value on graphs
            Pages.GraphPage graphs = _pages[(int)PageId.Graphs] as Pages.GraphPage;
            graphs.SetValue(GetComponentText(comNode), valData.Frame, valData.Value);

            //  set value on parameters, will be ignored if not a parameter
            //Pages.ParametersPage parameters = _pages[(int)PageId.Parameters] as Pages.ParametersPage;
            //parameters.SetValue(comNode.ComponentName, valData.Value);
        }

        public void SetParameterControlValue(Controls.ComponentTreeNode comNode, Packets.GetParamData paramData)
        {
            //  set value on graphs
            Pages.ParametersPage paramins = _pages[(int)PageId.Parameters] as Pages.ParametersPage;
            paramins.SetValue(GetComponentText(comNode), paramData.Value);

            //  set value on parameters, will be ignored if not a parameter
            //Pages.ParametersPage parameters = _pages[(int)PageId.Parameters] as Pages.ParametersPage;
            //parameters.SetValue(comNode.ComponentName, valData.Value);
        }

        public bool GetParameterControlValue(Controls.ComponentTreeNode comNode, ref float value)
        {
            //  get value from param in
            Pages.ParametersPage paramins = _pages[(int)PageId.Parameters] as Pages.ParametersPage;
            return paramins.GetValue(GetComponentText(comNode), ref value);
        }

        public void SetComponentAudio(Controls.ComponentTreeNode comNode, Packets.AudioData audioData)
        {
            //  set info on audio
            Pages.AudioPage audio = _pages[(int)PageId.Audio] as Pages.AudioPage;
            audio.SetInfo(GetComponentText(comNode), audioData);
        }

        public void SendActionTrigger(string comName)
        {
            ParentWindow.SendActionTrigger(ObjectHandle, InstanceHandle, comName);
        }

        public void SendParameterValue(string comName, float value, bool overridegame)
        {
            ParentWindow.SendParameterValue(ObjectHandle, InstanceHandle, comName, value, overridegame);
        }

        public void SendMuteEvent(string comName, bool mute)
        {
            ParentWindow.SendMuteEvent(ObjectHandle, InstanceHandle, comName, mute);
        }

        public void SetComponentConnection(string thisInst, string thisComType, string thisComName, string thatInst, string thatComType, string thatComName, bool output)
        {
            if (output)
            {
                Pages.ConnectionsPage cons = _pages[(int)PageId.Outputs] as Pages.ConnectionsPage;
                cons.SetInfo(thisInst, thisComType, thisComName, thatInst, thatComType, thatComName);
            }
            else
            {
                Pages.ConnectionsPage cons = _pages[(int)PageId.Inputs] as Pages.ConnectionsPage;
                cons.SetInfo(thisInst, thisComType, thisComName, thatInst, thatComType, thatComName);
            }
        }

        public void DoMessage(UInt32 frame, string obj, string inst, string com, string msg)
        {
            Pages.EventPage evnts = _pages[(int)PageId.Messages] as Pages.EventPage;

            evnts.LogMessage(frame, "Event", com, msg, Color.Black, Color.White);
        }

        public void DoError(UInt32 frame, string obj, string inst, string error, string errInst)
        {
            Pages.EventPage evnts = _pages[(int)PageId.Messages] as Pages.EventPage;

            evnts.LogMessage(frame, "Error", errInst, error, Color.White, Color.Red);
        }

        public void DoWarning(UInt32 frame, string obj, string inst, string warning, string warnInst)
        {
            Pages.EventPage evnts = _pages[(int)PageId.Messages] as Pages.EventPage;

            evnts.LogMessage(frame, "Warning", warnInst, warning, Color.Black, Color.Yellow);
        }
    }
}
