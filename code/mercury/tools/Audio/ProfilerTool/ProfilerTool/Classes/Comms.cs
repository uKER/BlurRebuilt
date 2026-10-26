using System;
using System.Collections.Generic;
using System.Text;
using XDevkit;
using System.Windows.Forms;

namespace ProfilerTool.Classes
{
    public class Comms
    {
        #region Helper classes
        /// <summary> 
        /// Class to store comms info
        /// </summary>
        public class Info
        {
            public Info()
            {
                _isXbox = true;
                _ipAddress = (10 << 24) + (35 << 16) + (8 << 8) + 177;// (uint)0xc0a800e6;
                _portNum = 6317;
            }

            public bool _isXbox;
            public uint _ipAddress;
            public uint _portNum;
        };
        #endregion

        enum Status
        {
            Ready,
            WaitingForInitData,
            Running
        };

        public Comms(Windows.MainWnd parent)
        {
            _shuttingDown = new System.Threading.ManualResetEvent(false);
            _parent = parent;
        }

        #region Messages
        /// <summary> 
        /// Send and receive messages
        /// </summary>
        public void MsgSend(string text)
        {
            //Console.Write("{send}" + text + "\n");
        }

        public void MsgRcv(string text)
        {
            //Console.Write("{rcv}" + text + "\n");
        }

        //  request info on objects and instances available
        //  returns info for building the tree view
        public bool SendInitializeRequest()
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "InitData";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendObjectRequest(UInt32 handle)
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "Object";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(handle);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendComValueRequest(UInt32 objHandle, UInt32 instHandle, UInt16 comHandle, UInt16 outHandle)
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "Value";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            b.Write(instHandle);
            b.Write(comHandle);
            b.Write(outHandle);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendActionTriggerRequest(UInt32 objHandle, UInt32 instHandle, UInt16 comHandle)
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "Trigger";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            b.Write(instHandle);
            b.Write(comHandle);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendSetParameterRequest(UInt32 objHandle, UInt32 instHandle, UInt16 comHandle, float value, bool overridegame)
        {
            if (!_con.IsConnected())
                return false;

            UInt16 flags;
            if (overridegame)
                flags = 1;
            else
                flags = 0;

            string msgName = "SetParam";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            b.Write(instHandle);
            b.Write(comHandle);
            b.Write(value);
            b.Write(flags);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendGetParameterRequest(UInt32 objHandle, UInt32 instHandle, UInt16 comHandle)
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "GetParam";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            b.Write(instHandle);
            b.Write(comHandle);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendAudioGraphRequest(UInt32 objHandle)
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "AudioGraph";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendAudioInfoRequest(UInt32 objHandle, UInt32 instHandle, UInt16 comHandle)
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "AudioInfo";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            b.Write(instHandle);
            b.Write(comHandle);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendMuteEventRequest(UInt32 objHandle, UInt32 instHandle, UInt16 comHandle, bool mute)
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "Mute";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            b.Write(instHandle);
            b.Write(comHandle);
            if (mute)
                b.Write((UInt16)1);
            else
                b.Write((UInt16)0);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendConnectionInfoRequest(UInt32 objHandle, UInt32 instHandle, UInt16 comHandle, bool output)
        {
            if (!_con.IsConnected())
                return false;

            UInt16 flags;
            if (output)
                flags = 0x0001;
            else
                flags = 0x0000;

            string msgName = "ConInfo";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            b.Write(instHandle);
            b.Write(comHandle);
            b.Write(flags);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendEventInfoRequest(UInt32 objHandle, UInt32 instHandle, bool enable)
        {
            if (!_con.IsConnected())
                return false;

            UInt16 flags;
            if (enable)
                flags = 0x0001;
            else
                flags = 0x0000;

            string msgName = "Events";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            b.Write(objHandle);
            b.Write(instHandle);
            b.Write(flags);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }

        public bool SendStatsRequest()
        {
            if (!_con.IsConnected())
                return false;

            string msgName = "Stats";
            MsgSend(msgName);
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(msgName);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                _con.Send(msg);
                return true;
            }

            return false;
        }
        #endregion

        /// <summary> 
        /// Shutdown the currently running artery thread and reconnect with different settings
        /// </summary>
        public void Connect(Info info)
        {
            if (_isCommsThreadRunning)
            {
                _shuttingDown.Set();
                _commsThread.Join();
            }
            UpdateConnectionStatus(false);
            _info = info;
            _commsThread = new System.Threading.Thread(WorkerThread_MaintainConnection);
            _commsThread.Start();
        }

        public void Shutdown()
        {
            _parent = null;
            _shuttingDown.Set();
            if (_commsThread != null)
                _commsThread.Join();
        }

        /// <summary> 
        /// Callback function that is called when a connection is established. Waits for incoming messages.
        /// </summary>
        private void MyConnectCallback(Bizarre.Artery.Connection con, System.Object context)
        {
            try
            {
                if (con.IsConnected())
                {
                    UpdateConnectionStatus(true);

                    while (con.IsConnected())
                    {
                        Bizarre.Artery.Message msg = new Bizarre.Artery.Message();

                        if (con.Receive(msg))
                        {
                            Bizarre.Artery.MessageReader msgReader = new Bizarre.Artery.MessageReader(msg);
                            if (_parent == null)
                                continue;

                            String text;
                            msgReader.Read(out text);

                            MsgRcv(text);

                            //  Process any recieved messages
                            if (_status == Status.WaitingForInitData)
                            {
                                if (text == "InitData")
                                {
                                    Packets.InitData packet = new Packets.InitData();
                                    packet.Read(msgReader);
                                    _parent.UpdateInitData(packet);
                                    _status = Status.Running;
                                }
                            }
                            else if (_status == Status.Running)
                            {
                                if (text == "Data")
                                {
                                    msgReader.Read(out text);
                                    while (text != "End")
                                    {
                                        if (text == "Stats")
                                        {
                                            Packets.StatsData packet = new Packets.StatsData();
                                            packet.Read(msgReader);
                                            msgReader.Read(out text);
                                            //_parent.UpdateStatsData(packet);
                                        }
                                        else if (text == "Object")
                                        {
                                            Packets.ObjectData packet = new Packets.ObjectData();
                                            packet.Read(msgReader);
                                            _parent.UpdateObjectData(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "Value")
                                        {
                                            Packets.ValueData packet = new Packets.ValueData();
                                            packet.Read(msgReader);
                                            _parent.UpdateValue(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "GetParam")
                                        {
                                            Packets.GetParamData packet = new Packets.GetParamData();
                                            packet.Read(msgReader);
                                            _parent.UpdateGetParam(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "AudioGraph")
                                        {
                                            Packets.AudioGraphData packet = new Packets.AudioGraphData();
                                            packet.Read(msgReader);
                                            _parent.UpdateAudioGraph(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "AudioInfo")
                                        {
                                            Packets.AudioData packet = new Packets.AudioData();
                                            packet.Read(msgReader);
                                            _parent.UpdateAudio(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "ConInfo")
                                        {
                                            Packets.ConnectionData packet = new Packets.ConnectionData();
                                            packet.Read(msgReader);
                                            _parent.UpdateConnections(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "Event")
                                        {
                                            Packets.EventData packet = new Packets.EventData();
                                            packet.Read(msgReader);
                                            _parent.UpdateEvents(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "Error")
                                        {
                                            Packets.MessageData packet = new Packets.MessageData();
                                            packet.Read(msgReader);
                                            _parent.UpdateErrors(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "Warning")
                                        {
                                            Packets.MessageData packet = new Packets.MessageData();
                                            packet.Read(msgReader);
                                            _parent.UpdateWarnings(packet);
                                            msgReader.Read(out text);
                                        }
                                        else if (text == "Reset")
                                        {
                                            SendInitializeRequest();
                                            msgReader.Read(out text);
                                        }
                                        else
                                        {
                                            Console.WriteLine("Unknown Message \"{s}\"", text);
                                        }
                                    }
                                }
                                else if (text == "InitData")
                                {
                                    _parent.Reset();
                                    Packets.InitData packet = new Packets.InitData();
                                    packet.Read(msgReader);
                                    _parent.UpdateInitData(packet);
                                    _status = Status.Running;
                                }
                            }
                        }
                    }
                }
            }
            catch
            {
            }

            UpdateConnectionStatus(false);
            _isConnected = false;
        }

        private void UpdateConnectionStatus(bool bConnected)
        {
            if (_parent == null)
                return;
            if (bConnected)
            {
                int x1 = (((int)_info._ipAddress >> 24) & 255);
                int x2 = (((int)_info._ipAddress >> 16) & 255);
                int x3 = (((int)_info._ipAddress >> 8) & 255);
                int x4 = (((int)_info._ipAddress >> 0) & 255);

                string text = "found " + x1.ToString() + "." + x2.ToString() + "." + x3.ToString() + "." + x4.ToString();// +" (" + _info.m_portNum.ToString() + ")";

                _parent.UpdateStatusText(text);
            }
            else
            {
                _parent.Reset();
                _parent.UpdateStatusText("not connected");
            }
        }

        /// <summary> 
        /// Main worker thread which tries to connect to an instance of artery "Profiler" and spawns a worker thread when it does
        /// </summary>
        private void WorkerThread_MaintainConnection()
        {
            try
            {
                _isConnected = false;

                Bizarre.Artery.ArteryManager.Startup(false, "AudioProfilerTool");

                _con = new Bizarre.Artery.Connection();

                if (_info._isXbox)
                {
                    XboxManagerClass xboxManager = new XboxManagerClass();
                    XboxConsole xboxConsole = xboxManager.OpenConsole(xboxManager.DefaultConsole);
                    _info._ipAddress = xboxConsole.IPAddressTitle;
                    _info._portNum = 6317;
                }
                Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(_info._ipAddress, _info._portNum);

                if (Bizarre.Artery.ArteryManager.Connect(_con, target, "AudioProfiler", MyConnectCallback, 0))
                    _isConnected = true;

                while (true)
                {
                    if (_shuttingDown.WaitOne(0, true))
                        break;

                    if (_isConnected)
                    {
                        while (_con.IsConnected())
                        {
                            if (_shuttingDown.WaitOne(30, true))
                                break;

                            //  Process any send messages
                            if (_status == Status.Ready)
                            {
                                int x1 = (((int)_info._ipAddress >> 24) & 255);
                                int x2 = (((int)_info._ipAddress >> 16) & 255);
                                int x3 = (((int)_info._ipAddress >> 8) & 255);
                                int x4 = (((int)_info._ipAddress >> 0) & 255);

                                string text = "connected to " + x1.ToString() + "." + x2.ToString() + "." + x3.ToString() + "." + x4.ToString();// +" (" + _info.m_portNum.ToString() + ")";

                                _parent.UpdateStatusText(text);
                                SendInitializeRequest();
                                _status = Status.WaitingForInitData;
                            }

                            //  maintain connection
                            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
                            b.Write("Idle");
                            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                            if (b.Build(msg))
                            {
                                _con.Send(msg);
                            }
                        }
                    }
                    else
                    {
                        _status = Status.Ready;
                        System.Threading.Thread.Sleep(1000);
                        _con.Close();
                        if (Bizarre.Artery.ArteryManager.Connect(_con, target, "AudioProfiler", MyConnectCallback, 0))
                            _isConnected = true;
                    }
                }
            }
            catch
            {
            }
        }

        #region Member vars
        /// <summary> 
        /// Member vars
        /// </summary>
        Windows.MainWnd _parent;
        private Info _info;
        private Bizarre.Artery.Connection _con;
        private System.Threading.ManualResetEvent _shuttingDown;
        private System.Threading.Thread _commsThread;
        private bool _isConnected = false;
        private bool _isCommsThreadRunning = false;
        private Status _status = Status.Ready;
        #endregion
    }
}
