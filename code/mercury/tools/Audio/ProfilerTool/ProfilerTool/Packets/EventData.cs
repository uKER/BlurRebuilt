using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.Net;

namespace ProfilerTool.Packets
{
    public class EventData
    {
        UInt32 _frame;
        UInt32 _obj;
        UInt32 _inst;
        UInt16 _com;
        UInt16 _msg;

        public EventData()
        {
        }

        public UInt32 Frame
        {
            get { return _frame; }
            set { _frame = value; }
        }

        public UInt32 ObjectHandle
        {
            get { return _obj; }
            set { _obj = value; }
        }

        public UInt32 InstanceHandle
        {
            get { return _inst; }
            set { _inst = value; }
        }

        public UInt16 ComponentHandle
        {
            get { return _com; }
            set { _com = value; }
        }

        public UInt16 MessageHandle
        {
            get { return _msg; }
            set { _msg = value; }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            msgReader.Read(out _frame);
            msgReader.Read(out _obj);
            msgReader.Read(out _inst);
            msgReader.Read(out _com);
            msgReader.Read(out _msg);
        }
    }

    public class MessageData
    {
        UInt32 _frame;
        UInt32 _obj;
        UInt32 _inst;
        Int16 _msg;

        public MessageData()
        {
        }

        public UInt32 Frame
        {
            get { return _frame; }
            set { _frame = value; }
        }

        public UInt32 ObjectHandle
        {
            get { return _obj; }
            set { _obj = value; }
        }

        public UInt32 InstanceHandle
        {
            get { return _inst; }
            set { _inst = value; }
        }

        public Int16 MessageHandle
        {
            get { return _msg; }
            set { _msg = value; }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            msgReader.Read(out _frame);
            msgReader.Read(out _obj);
            msgReader.Read(out _inst);
            msgReader.Read(out _msg);
        }
    }
}
