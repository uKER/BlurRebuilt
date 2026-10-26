using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.IO;
using ProfilerTool.Packets;

namespace ProfilerTool.Controls
{
    public class MainTreeView : TreeView
    {
        private Windows.MainWnd _parent;

        public MainTreeView()
        {
            CheckBoxes = true;
            AfterCheck += new TreeViewEventHandler(This_AfterCheck);
            AfterSelect += new TreeViewEventHandler(This_AfterSelect);
            DoubleClick += new System.EventHandler(This_DoubleClick);
        }

        public Windows.MainWnd Parent
        {
            get { return _parent; }
            set { _parent = value; }
        }

        private void This_AfterCheck(object sender, TreeViewEventArgs e)
        {
            ComponentTreeNode comNode = e.Node as ComponentTreeNode;
            InstanceTreeNode instNode = e.Node as InstanceTreeNode;

            if (e.Node.Checked)
            {
                if (comNode != null && Parent.GroupComponents)
                {
                    //  check all other coms
                    foreach (TreeNode i in comNode.Parent.Nodes)
                    {
                        if (i.Checked == false)
                            i.Checked = true;
                    }
                }

                if (e.Node.Parent != null && !e.Node.Parent.Checked)
                    e.Node.Parent.Checked = true;

                if (instNode != null)
                    _parent.ActivateInstance(instNode);

                if (comNode != null)
                {
                    instNode = e.Node.Parent.Parent as InstanceTreeNode;
                    _parent.SetSelectedTab(instNode);

                    _parent.ActivateComponent(comNode);
                }

                //  is a group node
                if (e.Node.Parent != null && instNode == null && comNode == null)
                    _parent.ActivateGroup(e.Node);
            }
            else
            {
                if (comNode != null && Parent.GroupComponents)
                {
                    //  check all other coms
                    foreach (TreeNode i in comNode.Parent.Nodes)
                    {
                        if (i.Checked == true)
                            i.Checked = false;
                    }
                }

                foreach (TreeNode i in e.Node.Nodes)
                {
                    if (i.Checked)
                        i.Checked = false;
                }

                if (instNode != null)
                    _parent.DeactivateInstance(instNode);

                if (comNode != null)
                    _parent.DeactivateComponent(comNode);

                //  is a group node
                if (e.Node.Parent != null && instNode == null && comNode == null)
                    _parent.DeactivateGroup(e.Node);
            }
        }

        private void This_AfterSelect(object sender, TreeViewEventArgs e)
        {
            //  instance node?
            InstanceTreeNode instNode = e.Node as InstanceTreeNode;

            //  try parent node
            if (instNode == null)
                instNode = e.Node.Parent as InstanceTreeNode;

            //  try parent parent node
            if (instNode == null)
            {
                if (e.Node.Parent != null)
                    instNode = e.Node.Parent.Parent as InstanceTreeNode;
            }

            if (instNode != null)
            {
                //  got it
                _parent.tbcInstances.SelectedTab = instNode.InstanceWindow;

                //  TODO: activate the tab page if not already available
                if (_parent.tbcInstances.SelectedTab != null)
                    _parent.tbcInstances.SelectedTab.Show();
            }
        }

        private void This_DoubleClick(object sender, EventArgs e)
        {
            if (SelectedNode != null)
            {
                TreeNode node = SelectedNode;

                if (node.Level == 2)
                {
                    InstanceTreeNode instNode = node.Parent as InstanceTreeNode;

                    instNode.InstanceWindow.ToggleDock(node.Text);
                }
            }
        }

        public void Create(Packets.InitData initData)
        {
            Nodes.Clear();

            //  create system object node
            Controls.ObjectTreeNode sysObjNode = new Controls.ObjectTreeNode("Audio", "Audio", 0xffffffff);
            Nodes.Add(sysObjNode);

            //  create system instance node
            Controls.InstanceTreeNode sysInstNode = new Controls.InstanceTreeNode("System", 0xffffffff);
            sysInstNode.InstanceWindow = _parent.CreateInstanceWindow("System", 0xffffffff, 0xffffffff);
            sysObjNode.Nodes.Add(sysInstNode);

            //  iterate through objects
            for (int i = 0; i < initData.m_ObjectList.Count; i++)
            {
                //  create object node
                string title = string.Format("{0:s} [{1:d}]", initData.m_ObjectList[i].name, initData.m_ObjectList[i].handle);
                Controls.ObjectTreeNode objNode = new Controls.ObjectTreeNode(initData.m_ObjectList[i].name, title, initData.m_ObjectList[i].handle);

                //  req object
                _parent.SendObjectRequest(objNode);

                //  req audio graph
                _parent.SendAudioGraphRequest(objNode);

                Nodes.Add(objNode);

                SelectedNode = objNode;

                //  iterate through instances
                for (int j = 0; j < initData.m_ObjectList[i].instances.Count; j++)
                {
                    //  create instance node
                    Controls.InstanceTreeNode instNode = new Controls.InstanceTreeNode(initData.m_ObjectList[i].instances[j].name, initData.m_ObjectList[i].instances[j].handle);

                    string caption = string.Format("{0:s} ({1:s})", initData.m_ObjectList[i].name, initData.m_ObjectList[i].instances[j].name);
                    instNode.InstanceWindow = _parent.CreateInstanceWindow(caption, initData.m_ObjectList[i].instances[j].handle, initData.m_ObjectList[i].handle);

                    SelectedNode.Nodes.Add(instNode);
                }

                //  select parent object node
                SelectedNode = Nodes[0];
            }

            ExpandAll();

            //  create system graph nodes
            TreeNode sysGraphNode = new TreeNode("[Graphs]");
            sysInstNode.Nodes.Add(sysGraphNode);

            ComponentTreeNode sysCPUGraphNode0 = new ComponentTreeNode("CPU", "Total", "", 0, 0);
            sysGraphNode.Nodes.Add(sysCPUGraphNode0);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysCPUGraphNode0);
            ComponentTreeNode sysCPUGraphNode1 = new ComponentTreeNode("CPU", "Queue", "", 1, 0);
            sysGraphNode.Nodes.Add(sysCPUGraphNode1);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysCPUGraphNode1);
            ComponentTreeNode sysCPUGraphNode2 = new ComponentTreeNode("CPU", "Streams", "", 2, 0);
            sysGraphNode.Nodes.Add(sysCPUGraphNode2);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysCPUGraphNode2);
            ComponentTreeNode sysCPUGraphNode3 = new ComponentTreeNode("CPU", "Objects", "", 3, 0);
            sysGraphNode.Nodes.Add(sysCPUGraphNode3);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysCPUGraphNode3);

            ComponentTreeNode sysMemGraphNode0 = new ComponentTreeNode("Memory", "Heap", "", 100, 0);
            sysGraphNode.Nodes.Add(sysMemGraphNode0);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysMemGraphNode0);
            ComponentTreeNode sysMemGraphNode1 = new ComponentTreeNode("Memory", "Queue", "", 100, 0);
            sysGraphNode.Nodes.Add(sysMemGraphNode1);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysMemGraphNode1);

            ComponentTreeNode sysStreamGraphNode = new ComponentTreeNode("I/O", "Bandwidth", "", 200, 0);
            sysGraphNode.Nodes.Add(sysStreamGraphNode);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysStreamGraphNode);

            ComponentTreeNode sysObjectGraphNode0 = new ComponentTreeNode("Objects", "Created", "", 300, 0);
            sysGraphNode.Nodes.Add(sysObjectGraphNode0);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysObjectGraphNode0);

            ComponentTreeNode sysInstanceGraphNode0 = new ComponentTreeNode("Instances", "Created", "", 400, 0);
            sysGraphNode.Nodes.Add(sysInstanceGraphNode0);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysInstanceGraphNode0);
            ComponentTreeNode sysInstanceGraphNode1 = new ComponentTreeNode("Instances", "Active", "", 401, 0);
            sysGraphNode.Nodes.Add(sysInstanceGraphNode1);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysInstanceGraphNode1);
            ComponentTreeNode sysInstanceGraphNode2 = new ComponentTreeNode("Instances", "Muted", "", 402, 0);
            sysGraphNode.Nodes.Add(sysInstanceGraphNode2);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysInstanceGraphNode2);

            ComponentTreeNode sysVoicesGraphNode0 = new ComponentTreeNode("Voices", "Created", "", 500, 0);
            sysGraphNode.Nodes.Add(sysVoicesGraphNode0);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysVoicesGraphNode0);
            ComponentTreeNode sysVoicesGraphNode1 = new ComponentTreeNode("Voices", "Active", "", 501, 0);
            sysGraphNode.Nodes.Add(sysVoicesGraphNode1);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysVoicesGraphNode1);

            ComponentTreeNode sysMixersGraphNode0 = new ComponentTreeNode("Mixers", "Created", "", 600, 0);
            sysGraphNode.Nodes.Add(sysMixersGraphNode0);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysMixersGraphNode0);
            ComponentTreeNode sysMixersGraphNode1 = new ComponentTreeNode("Mixers", "Active", "", 601, 0);
            sysGraphNode.Nodes.Add(sysMixersGraphNode1);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysMixersGraphNode1);

            ComponentTreeNode sysEffectsGraphNode0 = new ComponentTreeNode("Effects", "Created", "", 700, 0);
            sysGraphNode.Nodes.Add(sysEffectsGraphNode0);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysEffectsGraphNode0);
            ComponentTreeNode sysEffectsGraphNode1 = new ComponentTreeNode("Effects", "Active", "", 701, 0);
            sysGraphNode.Nodes.Add(sysEffectsGraphNode1);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysEffectsGraphNode1);

            ComponentTreeNode sysSpeakerGraphNode0 = new ComponentTreeNode("Speaker", "FrontLeft", "", 800, 0);
            sysGraphNode.Nodes.Add(sysSpeakerGraphNode0);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysSpeakerGraphNode0);
            ComponentTreeNode sysSpeakerGraphNode1 = new ComponentTreeNode("Speaker", "FrontRight", "", 801, 0);
            sysGraphNode.Nodes.Add(sysSpeakerGraphNode1);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysSpeakerGraphNode1);
            ComponentTreeNode sysSpeakerGraphNode2 = new ComponentTreeNode("Speaker", "SurroundLeft", "", 802, 0);
            sysGraphNode.Nodes.Add(sysSpeakerGraphNode2);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysSpeakerGraphNode2);
            ComponentTreeNode sysSpeakerGraphNode3 = new ComponentTreeNode("Speaker", "SurroundRight", "", 803, 0);
            sysGraphNode.Nodes.Add(sysSpeakerGraphNode3);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysSpeakerGraphNode3);
            ComponentTreeNode sysSpeakerGraphNode4 = new ComponentTreeNode("Speaker", "Distance", "", 804, 0);
            sysGraphNode.Nodes.Add(sysSpeakerGraphNode4);
            sysInstNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, sysSpeakerGraphNode4);

            //  create messages node
            TreeNode sysMsgNode = new TreeNode("[Messages]");
            sysInstNode.Nodes.Add(sysMsgNode);

            //sysObjNode.Collapse(true);
        }

        public void AddObject(Packets.ObjectData objData)
        {
            foreach (ObjectTreeNode objNode in Nodes)
            {
                if (objNode.ObjectHandle == objData.ObjectHandle)
                {
                    //  set obj data for this node
                    objNode.ObjectData = objData;

                    foreach (InstanceTreeNode instNode in objNode.Nodes)
                    {
                        TreeNode group;
                        if (objData.Audio.Count > 0)
                        {
                            group = new TreeNode("[Audio]");
                            instNode.Nodes.Add(group);
                            foreach (Packets.Component com in objData.Audio)
                            {
                                Controls.ComponentTreeNode comNode = new Controls.ComponentTreeNode(com.Name, "", com.Type, com.Index, 0);
                                group.Nodes.Add(comNode);

                                instNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Audio, comNode);
                            }
                        }
                        if (objData.Effects.Count > 0)
                        {
                            group = new TreeNode("[Effects]");
                            instNode.Nodes.Add(group);
                            foreach (Packets.Component com in objData.Effects)
                            {
                                Controls.ComponentTreeNode comNode = new Controls.ComponentTreeNode(com.Name, "", com.Type, com.Index, 0);
                                group.Nodes.Add(comNode);
                            }
                        }
                        if (objData.Actions.Count > 0)
                        {
                            group = new TreeNode("[Actions]");
                            instNode.Nodes.Add(group);
                            foreach (Packets.Component com in objData.Actions)
                            {
                                Controls.ComponentTreeNode comNode = new Controls.ComponentTreeNode(com.Name, "", com.Type, com.Index, 0);
                                group.Nodes.Add(comNode);

                                instNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Actions, comNode);
                            }
                        }
                        if (objData.Parameters.Count > 0)
                        {
                            group = new TreeNode("[Parameters]");
                            instNode.Nodes.Add(group);
                            foreach (Packets.Component com in objData.Parameters)
                            {
                                Controls.ComponentTreeNode comNode = new Controls.ComponentTreeNode(com.Name, "", com.Type, com.Index, 0);
                                group.Nodes.Add(comNode);

                                instNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Parameters, comNode);
                            }
                        }
                        if (objData.Vectors.Count > 0)
                        {
                            group = new TreeNode("[Vectors]");
                            instNode.Nodes.Add(group);
                            foreach (Packets.Component com in objData.Vectors)
                            {
                                Controls.ComponentTreeNode comNode = new Controls.ComponentTreeNode(com.Name, "", com.Type, com.Index, 0);
                                group.Nodes.Add(comNode);

                                instNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Vectors, comNode);
                            }
                        }
                        if (objData.Controls.Count > 0)
                        {
                            group = new TreeNode("[Graphs]");
                            instNode.Nodes.Add(group);
                            foreach (Packets.Component com in objData.Controls)
                            {
                                foreach (Packets.Output output in com.Outputs())
                                {
                                    Controls.ComponentTreeNode comNode = new Controls.ComponentTreeNode(com.Name, output.Name, com.Type, com.Index, output.Index);
                                    group.Nodes.Add(comNode);

                                    instNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Graphs, comNode);
                                }
                            }
                        }
                        if (objData.Inputs.Count > 0)
                        {
                            group = new TreeNode("[Inputs]");
                            instNode.Nodes.Add(group);
                            foreach (Packets.Component com in objData.Inputs)
                            {
                                Controls.ComponentTreeNode comNode = new Controls.ComponentTreeNode(com.Name, "", com.Type, com.Index, 0);
                                group.Nodes.Add(comNode);

                                instNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Inputs, comNode);
                            }
                        }
                        if (objData.Outputs.Count > 0)
                        {
                            group = new TreeNode("[Outputs]");
                            instNode.Nodes.Add(group);
                            foreach (Packets.Component com in objData.Outputs)
                            {
                                Controls.ComponentTreeNode comNode = new Controls.ComponentTreeNode(com.Name, "", com.Type, com.Index, 0);
                                group.Nodes.Add(comNode);

                                instNode.InstanceWindow.AddControl(Windows.InstanceWnd.PageId.Outputs, comNode);
                            }
                        }

                        {
                            group = new TreeNode("[Messages]");
                            instNode.Nodes.Add(group);
                        }
                    }

                    break;
                }
            }
        }

        public ObjectTreeNode GetObjectNodeFromHandle(UInt32 objHandle)
        {
            foreach (ObjectTreeNode objNode in Nodes)
            {
                if (objNode.ObjectHandle == objHandle)
                    return objNode;
            }

            return null;
        }

        public void SetValue(Packets.ValueData valData)
        {
            foreach (ObjectTreeNode objNode in Nodes)
            {
                if (objNode.ObjectHandle == valData.ObjectHandle)
                {
                    foreach (InstanceTreeNode instNode in objNode.Nodes)
                    {
                        if (instNode.InstanceHandle == valData.InstanceHandle)
                        {
                            foreach (TreeNode groupNode in instNode.Nodes)
                            {
                                if (groupNode.Text == "[Graphs]")
                                {
                                    foreach (ComponentTreeNode comNode in groupNode.Nodes)
                                    {
                                        if (comNode.ComponentHandle == valData.ComponentHandle)
                                        {
                                            if (comNode.ComponentOutputHandle == valData.OutputHandle)
                                            {
                                                if (comNode.Pending)
                                                {
                                                    //  update value
                                                    _parent.SetComponentValue(comNode, valData);

                                                    //  clear pending data from game flag
                                                    comNode.Pending = false;

                                                    //  if still checked, ask for next value
                                                    if (comNode.Checked)
                                                    {
                                                        _parent._comms.SendComValueRequest(valData.ObjectHandle, valData.InstanceHandle, valData.ComponentHandle, valData.OutputHandle);
                                                        comNode.Pending = true;
                                                    }
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }

        public void SetParamValue(Packets.GetParamData paramData)
        {
            foreach (ObjectTreeNode objNode in Nodes)
            {
                if (objNode.ObjectHandle == paramData.ObjectHandle)
                {
                    foreach (InstanceTreeNode instNode in objNode.Nodes)
                    {
                        if (instNode.InstanceHandle == paramData.InstanceHandle)
                        {
                            foreach (TreeNode groupNode in instNode.Nodes)
                            {
                                if (groupNode.Text == "[Parameters]")
                                {
                                    foreach (ComponentTreeNode comNode in groupNode.Nodes)
                                    {
                                        if (comNode.ComponentHandle == paramData.ComponentHandle)
                                        {
                                            if (comNode.PendingParamIn)
                                            {
                                                //  update value
                                                _parent.SetParameterControlValue(comNode, paramData);

                                                //  clear pending data from game flag
                                                comNode.PendingParamIn = false;

                                                //  if still checked, ask for next value
                                                if (comNode.Checked)
                                                {
                                                    _parent._comms.SendGetParameterRequest(paramData.ObjectHandle, paramData.InstanceHandle, paramData.ComponentHandle);
                                                    comNode.PendingParamIn = true;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }

        public void SetAudioGraph(Packets.AudioGraphData audioGraphData)
        {
            foreach (ObjectTreeNode objNode in Nodes)
            {
                if (objNode.ObjectHandle == audioGraphData.ObjectHandle)
                {
                    objNode.AudioGraphData = audioGraphData;
                    break;
                }
            }
        }

        public void SetAudio(Packets.AudioData audioData)
        {
            foreach (ObjectTreeNode objNode in Nodes)
            {
                if (objNode.ObjectHandle == audioData.ObjectHandle)
                {
                    foreach (InstanceTreeNode instNode in objNode.Nodes)
                    {
                        if (instNode.InstanceHandle == audioData.InstanceHandle)
                        {
                            foreach (TreeNode groupNode in instNode.Nodes)
                            {
                                if (groupNode.Text == "[Audio]")
                                {
                                    foreach (ComponentTreeNode comNode in groupNode.Nodes)
                                    {
                                        if (comNode.ComponentHandle == audioData.ComponentHandle)
                                        {
                                            if (comNode.Pending)
                                            {
                                                //  update value
                                                _parent.SetComponentAudio(comNode, audioData);

                                                //  clear pending data from game flag
                                                comNode.Pending = false;

                                                //  if still checked, ask for next value
                                                if (comNode.Checked)
                                                {
                                                    _parent._comms.SendAudioInfoRequest(audioData.ObjectHandle, audioData.InstanceHandle, audioData.ComponentHandle);
                                                    comNode.Pending = true;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }

        public void SetConnections(Packets.ConnectionData conData)
        {
            string thisInst = "";
            string thisComType = "";
            string thisComName = "";
            string thatInst = "";
            string thatComType = "";
            string thatComName = "";
            //  do 'that' object
            foreach (ObjectTreeNode objNode in Nodes)
            {
                if (objNode.ObjectHandle == conData.ThatObjectHandle)
                {
                    thatInst = objNode.Text;
                    foreach (InstanceTreeNode instNode in objNode.Nodes)
                    {
                        if (instNode.InstanceHandle == conData.ThatInstanceHandle)
                        {
                            foreach (TreeNode groupNode in instNode.Nodes)
                            {
                                if ((conData.IsOutput && groupNode.Text == "[Inputs]") ||
                                    (!conData.IsOutput && groupNode.Text == "[Outputs]"))
                                {
                                    foreach (ComponentTreeNode comNode in groupNode.Nodes)
                                    {
                                        if (comNode.ComponentHandle == conData.ThatComponentHandle)
                                        {
                                            thatComType = comNode.ComponentType;
                                            thatComName = comNode.ComponentName;
                                        }
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            foreach (ObjectTreeNode objNode in Nodes)
            {
                if (objNode.ObjectHandle == conData.ThisObjectHandle)
                {
                    thisInst = objNode.Text;
                    foreach (InstanceTreeNode instNode in objNode.Nodes)
                    {
                        if (instNode.InstanceHandle == conData.ThisInstanceHandle)
                        {
                            foreach (TreeNode groupNode in instNode.Nodes)
                            {
                                if ((conData.IsOutput && groupNode.Text == "[Outputs]") ||
                                    (!conData.IsOutput && groupNode.Text == "[Inputs]"))
                                {
                                    foreach (ComponentTreeNode comNode in groupNode.Nodes)
                                    {
                                        if (comNode.ComponentHandle == conData.ThisComponentHandle)
                                        {
                                            if (comNode.Pending)
                                            {
                                                thisComType = comNode.ComponentType;
                                                thisComName = comNode.ComponentName;

                                                //  update value
                                                _parent.SetComponentConnection(comNode, thisInst, thisComType, thisComName, thatInst, thatComType, thatComName, conData);

                                                //  clear pending data from game flag
                                                comNode.Pending = false;

                                                //  if still checked, ask for next value
                                                if (comNode.Checked)
                                                {
                                                    _parent._comms.SendConnectionInfoRequest(conData.ThisObjectHandle, conData.ThisInstanceHandle, conData.ThisComponentHandle, conData.IsOutput);
                                                    comNode.Pending = true;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }

        public void Select(string name)
        {
            foreach (ObjectTreeNode objNode in Nodes)
            {
                foreach (InstanceTreeNode instNode in objNode.Nodes)
                {
                    if (instNode.InstanceWindow.Text == name)
                    {
                        //  only set it to the instance node if not already a child of that node
                        if (SelectedNode.Parent != null)
                        {
                            if (SelectedNode.Parent == instNode)
                                return;

                            if (SelectedNode.Parent.Parent != null)
                            {
                                if (SelectedNode.Parent.Parent == instNode)
                                    return;
                            }
                        }

                        SelectedNode = instNode;
                        Select();
                        break;
                    }
                }
            }
        }

        public void DoMessage(Packets.EventData evData)
        {
            if (evData.InstanceHandle == 0xffffffff)
            {
                _parent.DoMessage(null, evData);
            }
            else
            {
                foreach (ObjectTreeNode objNode in Nodes)
                {
                    if (objNode.ObjectHandle == evData.ObjectHandle)
                    {
                        foreach (InstanceTreeNode instNode in objNode.Nodes)
                        {
                            if (instNode.InstanceHandle == evData.InstanceHandle)
                            {
                                _parent.DoMessage(instNode, evData);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }

        public void DoError(Packets.MessageData errData)
        {
            if (errData.InstanceHandle == 0xffffffff)
            {
                _parent.DoError(null, errData);
            }
            else
            {
                foreach (ObjectTreeNode objNode in Nodes)
                {
                    if (objNode.ObjectHandle == errData.ObjectHandle)
                    {
                        foreach (InstanceTreeNode instNode in objNode.Nodes)
                        {
                            if (instNode.InstanceHandle == errData.InstanceHandle)
                            {
                                _parent.DoError(instNode, errData);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }

        public void DoWarning(Packets.MessageData warnData)
        {
            if (warnData.InstanceHandle == 0xffffffff)
            {
                _parent.DoWarning(null, warnData);
            }
            else
            {
                foreach (ObjectTreeNode objNode in Nodes)
                {
                    if (objNode.ObjectHandle == warnData.ObjectHandle)
                    {
                        foreach (InstanceTreeNode instNode in objNode.Nodes)
                        {
                            if (instNode.InstanceHandle == warnData.InstanceHandle)
                            {
                                _parent.DoWarning(instNode, warnData);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }

        public void LoadMix(StreamReader reader)
        {
            string value;
            while ((value = reader.ReadLine()) != null)
                LoadMixValue(value);
        }

        public void LoadMixValue(string item)
        {
            try
            {
                string[] items = item.Split(new char [] {' ', '[', ']'} );
                string objName = items[0];
                uint instHandle = Convert.ToUInt32(items[1]);
                string comName = items[2];
                float value = Convert.ToSingle(items[3]);

                GetParamData paramData = new GetParamData();

                foreach (ObjectTreeNode objNode in Nodes)
                {
                    if (objNode.ObjectName == objName)
                    {
                        foreach (InstanceTreeNode instNode in objNode.Nodes)
                        {
                            if (instNode.InstanceHandle == instHandle)
                            {
                                foreach (TreeNode groupNode in instNode.Nodes)
                                {
                                    if (groupNode.Text == "[Parameters]")
                                    {
                                        foreach (ComponentTreeNode comNode in groupNode.Nodes)
                                        {
                                            if (comNode.ComponentName == comName)
                                            {
                                                Parent.SendParameterValue(objNode.ObjectHandle, instNode.InstanceHandle, comNode.ComponentName, value, true);
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            catch
            {
                MessageBox.Show(string.Format("Cannot process line '{0:s}' in mixer snapshot.", item), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void CreateMix(StreamWriter writer)
        {
            foreach (ObjectTreeNode objNode in Nodes)
            {
                foreach (InstanceTreeNode instNode in objNode.Nodes)
                {
                    foreach (TreeNode groupNode in instNode.Nodes)
                    {
                        if (groupNode.Text == "[Parameters]")
                        {
                            foreach (ComponentTreeNode comNode in groupNode.Nodes)
                            {
                                float value = 0.0f;
                                if (_parent.GetParameterControlValue(comNode, ref value))
                                    writer.WriteLine(objNode.ObjectName + "[" + instNode.InstanceHandle + "]" + comNode.ComponentName + " " + value);
                            }
                        }
                    }
                }
            }
        }
    }

    public class ObjectTreeNode : TreeNode
    {
        private string _name;
        private UInt32 _instHandle;
        private Packets.ObjectData _data;
        private Packets.AudioGraphData _audioGraphData;

        public ObjectTreeNode(string name, string title, UInt32 handle)
            : base(title)
        {
            ObjectName = name;
            ObjectHandle = handle;
        }

        public string ObjectName
        {
            get { return _name; }
            set { _name = value; }
        }

        public UInt32 ObjectHandle
        {
            get { return _instHandle; }
            set { _instHandle = value; }
        }

        public Packets.ObjectData ObjectData
        {
            get { return _data; }
            set { _data = value; }
        }

        public Packets.AudioGraphData AudioGraphData
        {
            get { return _audioGraphData; }
            set { _audioGraphData = value; }
        }
    }

    public class InstanceTreeNode : TreeNode
    {
        private string _name;
        private UInt32 _instHandle;
        public Windows.InstanceWnd _wnd;

        public InstanceTreeNode(string name, UInt32 handle)
            //: base(string.Format("{0:s} [{1:d}]", name, handle))
            : base(name)
        {
            InstanceName = name;
            InstanceHandle = handle;
        }

        public string InstanceName
        {
            get { return _name; }
            set { _name = value; }
        }

        public UInt32 InstanceHandle
        {
            get { return _instHandle; }
            set { _instHandle = value; }
        }

        public Windows.InstanceWnd InstanceWindow
        {
            get { return _wnd; }
            set { _wnd = value; }
        }
    }

    public class ComponentTreeNode : TreeNode
    {
        private string _name;
        private string _output;
        private string _type;
        private UInt16 _instHandle;
        private UInt16 _outindex;
        private bool _isPending = false;
        private bool _isPendingParamIn = false;

        public ComponentTreeNode(string name, string output, string type, UInt16 handle, UInt16 outindex)
        {
            ComponentName = name;
            ComponentOutput = output;
            ComponentType = type;
            ComponentHandle = handle;
            ComponentOutputHandle = outindex;

            if (output == "")
            {
                if (type == "")
                    Text = string.Format("{0:s}", name);
                else
                    Text = string.Format("{0:s} ({1:s})", name, type);
            }
            else
            {
                if (type == "")
                    Text = string.Format("{0:s}.{1:s}", name, output);
                else
                    Text = string.Format("{0:s}.{1:s} ({2:s})", name, output, type);
            }
        }

        public string ComponentName
        {
            get { return _name; }
            set { _name = value; }
        }

        public string ComponentOutput
        {
            get { return _output; }
            set { _output = value; }
        }

        public string ComponentType
        {
            get { return _type; }
            set { _type = value; }
        }

        public UInt16 ComponentHandle
        {
            get { return _instHandle; }
            set { _instHandle = value; }
        }

        public UInt16 ComponentOutputHandle
        {
            get { return _outindex; }
            set { _outindex = value; }
        }

        public bool Pending
        {
            get { return _isPending; }
            set { _isPending = value; }
        }

        public bool PendingParamIn
        {
            get { return _isPendingParamIn; }
            set { _isPendingParamIn = value; }
        }
    }
}
