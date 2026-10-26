using System;
using System.Collections.Generic;
using System.Text;

namespace ProfilerTool.Packets
{
    class StatsData
    {
        UInt16 _numObjs;
        UInt16 _numInsts;
        UInt16 _numWaves;
        UInt16 _numMixers;

        public StatsData()
        {
        }

        public UInt16 NumObjects
        {
            get { return _numObjs; }
            set { _numObjs = value; }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            msgReader.Read(out _numObjs);
            msgReader.Read(out _numInsts);
            msgReader.Read(out _numWaves);
            msgReader.Read(out _numMixers);
        }
    }
}
