using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.Net;

namespace ProfilerTool.Packets
{
    public class AudioGraphData
    {
        public class AudioGraphDataItem
        {
            public UInt16 com = 0xffff;
            public bool _hasOutput = false;
            public int _depth = 0;
            public int _height = 0;
            public List<UInt16> _inputs;
            public List<AudioGraphDataItem> _inputComs;

            public AudioGraphDataItem()
            {
                _inputs = new List<UInt16>();
                _inputComs = new List<AudioGraphDataItem>();
            }

            public int Depth
            {
                get { return _depth; }
                set { _depth = value; }
            }

            public int Height
            {
                get { return _height; }
                set { _height = value; }
            }

            public void SetDepth(int depth)
            {
                _depth = depth;

                foreach (AudioGraphDataItem input in _inputComs)
                {
                    //  set the input item's depths
                    input.SetDepth(depth + 1);
                }
            }
        }

        private UInt32 _obj;
        private List<AudioGraphDataItem> _items;

        public AudioGraphData()
        {
            _items = new List<AudioGraphDataItem>();
        }

        public UInt32 ObjectHandle
        {
            get { return _obj; }
            set { _obj = value; }
        }

        public List<AudioGraphDataItem> Items
        {
            get { return _items; }
            set { _items = value; }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            msgReader.Read(out _obj);

            UInt16 numAudioComs;
            msgReader.Read(out numAudioComs);

            for (int i=0; i<numAudioComs; i++)
            {
                AudioGraphDataItem item = new AudioGraphDataItem();

                msgReader.Read(out item.com);

                UInt16 numInputs;
                msgReader.Read(out numInputs);

                for (int j = 0; j < numInputs; j++)
                {
                    UInt16 com;
                    msgReader.Read(out com);
                    item._inputs.Add(com);
                }

                _items.Add(item);
            }

            //  lookup com -> item
            foreach (AudioGraphDataItem item in _items)
            {
                foreach (UInt16 comInput in item._inputs)
                {
                    foreach (AudioGraphDataItem input in _items)
                    {
                        if (comInput == input.com)
                            item._inputComs.Add(input);
                    }
                }

            }
            //  build graph view - tag if item has output
            foreach (AudioGraphDataItem item in _items)
            {
                foreach (AudioGraphDataItem comInput in item._inputComs)
                    comInput._hasOutput = true;
            }

            //  recurse from leaf items to set depth
            foreach (AudioGraphDataItem item in _items)
            {
                if (!item._hasOutput)
                    item.SetDepth(0);
            }

            //  find largest depth
            int maxdepth = 0;
            foreach (AudioGraphDataItem item in _items)
            {
                if (maxdepth < item.Depth)
                    maxdepth = item.Depth;
            }

            int[] heights = new int[maxdepth + 1];

            //  set heights, reverse depths
            foreach (AudioGraphDataItem item in _items)
            {
                item.Height = heights[item.Depth]++;
                item.Depth = maxdepth - item.Depth;
            }

            //  TODO: finish graph building
        }
    }
}
