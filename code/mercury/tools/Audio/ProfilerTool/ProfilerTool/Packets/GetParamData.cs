using System;
using System.Collections.Generic;
using System.Text;

namespace ProfilerTool.Packets
{
    public class GetParamData
    {
        UInt32 _obj;
        UInt32 _inst;
        UInt16 _com;
        Single _value;

        public GetParamData()
        {
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

        public Single Value
        {
            get { return _value; }
            set { _value = value; }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            msgReader.Read(out _obj);
            msgReader.Read(out _inst);
            msgReader.Read(out _com);
            msgReader.Read(out _value);
        }
    }
}
