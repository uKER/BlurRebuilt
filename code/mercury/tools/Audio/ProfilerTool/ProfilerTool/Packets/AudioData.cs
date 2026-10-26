using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.Net;

namespace ProfilerTool.Packets
{
    public class AudioData
    {
        const UInt16 flagActive = 0x0001;

        private UInt32 _obj;
        private UInt32 _inst;
        private UInt16 _com;
        private UInt16 _flags;
        private bool _isActive;
        private List<UInt16> _inputs;

        public AudioData()
        {
            _inputs = new List<UInt16>();
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

        public bool IsActive
        {
            get { return _isActive; }
            set { _isActive = value; }
        }

        public bool IsProcessing
        {
            get { return ((_flags & flagActive) > 0) ? true : false; }
        }

        public List<UInt16> Inputs
        {
            get { return _inputs; }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            msgReader.Read(out _obj);
            msgReader.Read(out _inst);
            msgReader.Read(out _com);
            msgReader.Read(out _flags);
            UInt16 numInputs;
            msgReader.Read(out numInputs);

            for (int i = 0; i < numInputs; i++)
            {
                UInt16 com;
                msgReader.Read(out com);
                Inputs.Add(com);
            }
        }
    }
}
