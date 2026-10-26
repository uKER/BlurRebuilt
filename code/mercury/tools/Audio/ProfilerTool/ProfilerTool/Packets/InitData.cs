using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.Net;

namespace ProfilerTool.Packets
{
    public class InitData
    {
        public struct InstanceEntry
        {
            public string name;
            public UInt32 handle;
        }

        public class ObjectEntry
        {
            public ObjectEntry()
            {
                instances = new List<InstanceEntry>();
            }

            public string name;
            public UInt32 handle;
            public List<InstanceEntry> instances;
        }

        public List<ObjectEntry> m_ObjectList;

        public InitData()
        {
            m_ObjectList = new List<ObjectEntry>();
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            UInt16 numObjects;
            UInt16 numInstances;
            msgReader.Read(out numObjects);
            for (int i = 0; i < numObjects; i++)
            {
                ObjectEntry obj = new ObjectEntry();
                msgReader.Read(out obj.name);
                msgReader.Read(out obj.handle);

                msgReader.Read(out numInstances);
                for (int j = 0; j < numInstances; j++)
                {
                    InstanceEntry inst = new InstanceEntry();
                    //msgReader.Read(out inst.name);
                    msgReader.Read(out inst.handle);
                    inst.name = Convert.ToString(inst.handle);

                    obj.instances.Add(inst);

                    //Console.Write("Object '" + obj.name + "' handle = " + obj.handle + "\n");
                }

                m_ObjectList.Add(obj);

                //Console.Write("Object '" + obj.name + "' handle = " + obj.handle + "\n");
            }
        }
    }
}
