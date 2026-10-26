using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.Net;

namespace ProfilerTool.Packets
{
    public class ConnectionData
    {
        const UInt16 flagOutput = 0x0001;

        private UInt32 _objThis;
        private UInt32 _objThat;
        private UInt32 _instThis;
        private UInt32 _instThat;
        private UInt16 _comThis;
        private UInt16 _comThat;
        private UInt16 _flags;

        public ConnectionData()
        {
        }

        public UInt32 ThisObjectHandle
        {
            get { return _objThis; }
            set { _objThis = value; }
        }

        public UInt32 ThatObjectHandle
        {
            get { return _objThat; }
            set { _objThat = value; }
        }

        public UInt32 ThisInstanceHandle
        {
            get { return _instThis; }
            set { _instThis = value; }
        }

        public UInt32 ThatInstanceHandle
        {
            get { return _instThat; }
            set { _instThat = value; }
        }

        public UInt16 ThisComponentHandle
        {
            get { return _comThis; }
            set { _comThis = value; }
        }

        public UInt16 ThatComponentHandle
        {
            get { return _comThat; }
            set { _comThat = value; }
        }

        public bool IsOutput
        {
            get { return (_flags & flagOutput) > 0; }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            msgReader.Read(out _objThis);
            msgReader.Read(out _instThis);
            msgReader.Read(out _comThis);
            msgReader.Read(out _objThat);
            msgReader.Read(out _instThat);
            msgReader.Read(out _comThat);
            msgReader.Read(out _flags);
        }
    }
}
