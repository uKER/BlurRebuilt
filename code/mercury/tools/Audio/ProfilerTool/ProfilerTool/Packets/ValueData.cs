using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.Net;

namespace ProfilerTool.Packets
{
    public class ValueData
    {
        UInt32 _frame = 1234;
        UInt32 _obj;
        UInt32 _inst;
        UInt16 _com;
        UInt16 _output;
        Single _value;

        public ValueData()
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

        public UInt16 OutputHandle
        {
            get { return _output; }
            set { _output = value; }
        }

        public Single Value
        {
            get { return _value; }
            set { _value = value; }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            msgReader.Read(out _frame);
            msgReader.Read(out _obj);
            msgReader.Read(out _inst);
            msgReader.Read(out _com);
            msgReader.Read(out _output);
            msgReader.Read(out _value);
        }
    }
}
