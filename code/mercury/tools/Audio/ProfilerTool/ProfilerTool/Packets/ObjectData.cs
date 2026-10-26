using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.IO;
using System.Net;

namespace ProfilerTool.Packets
{
    public class ComponentDef
    {
        public class Child
        {
            private string _name;
            private UInt32 _size;
            private UInt32 _offset;

            public Child()
            {
                _size = 0;
                _offset = 0;
            }

            public string Name
            {
                get { return _name; }
                set { _name = value; }
            }

            public UInt32 Size
            {
                get { return _size; }
                set { _size = value; }
            }

            public UInt32 Offset
            {
                get { return _offset; }
                set { _offset = value; }
            }
        }

        private string _name;
        private UInt32 _type;
        private UInt32 _dataSize;
        private UInt32 _stateSize;
        private List<Child> _children;
        private List<Output> _outputs;
        private List<ComMessage> _messages;
        private bool _isAction = false;
        private bool _isParameter = false;
        private bool _isVector = false;
        private bool _isControl = false;
        private bool _isInput = false;
        private bool _isOutput = false;
        private bool _isAudio = false;
        private bool _isEffect = false;

        public ComponentDef()
        {
            _name = "Untitled";
            _type = 0xffffffff;
            _dataSize = 0;
            _stateSize = 0;
            _children = new List<Child>();
            _outputs = new List<Output>();
            _messages = new List<ComMessage>();
        }

        public string Name
        {
            get { return _name; }
            set { _name = value; }
        }

        public UInt32 Type
        {
            get { return _type; }
            set { _type = value; }
        }

        public UInt32 DataSize
        {
            get { return _dataSize; }
            set { _dataSize = value; }
        }

        public UInt32 StateSize
        {
            get { return _stateSize; }
            set { _stateSize = value; }
        }

        public bool IsAction
        {
            get { return _isAction; }
            set { _isAction = value; }
        }

        public bool IsParameter
        {
            get { return _isParameter; }
            set { _isParameter = value; }
        }

        public bool IsVector
        {
            get { return _isVector; }
            set { _isVector = value; }
        }

        public bool IsControl
        {
            get { return _isControl; }
            set { _isControl = value; }
        }

        public bool IsInput
        {
            get { return _isInput; }
            set { _isInput = value; }
        }

        public bool IsOutput
        {
            get { return _isOutput; }
            set { _isOutput = value; }
        }

        public bool HasMessages
        {
            get { return _messages.Count > 0; }
        }

        public bool IsAudio
        {
            get { return _isAudio; }
            set { _isAudio = value; }
        }

        public bool IsEffect
        {
            get { return _isEffect; }
            set { _isEffect = value; }
        }

        public void AddChild(Child child)
        {
            _children.Add(child);
        }

        public void AddOutput(Output output)
        {
            _outputs.Add(output);
        }

        public void AddMessage(ComMessage msg)
        {
            _messages.Add(msg);
        }

        public List<Child> Children()
        {
            return _children;
        }

        public List<Output> Outputs()
        {
            return _outputs;
        }

        public List<ComMessage> Messages()
        {
            return _messages;
        }
    }

    public class ErrorDef
    {
        private string _text;
        private Int32 _index;

        public ErrorDef()
        {
            _text = "Untitled";
            _index = -1;
        }

        public string Text
        {
            get { return _text; }
            set { _text = value; }
        }

        public Int32 Index
        {
            get { return _index; }
            set { _index = value; }
        }
    }

    public class WarningDef
    {
        private string _text;
        private Int32 _index;

        public WarningDef()
        {
            _text = "Untitled";
            _index = -1;
        }

        public string Text
        {
            get { return _text; }
            set { _text = value; }
        }

        public Int32 Index
        {
            get { return _index; }
            set { _index = value; }
        }
    }

    public class Output
    {
        private string _name;
        private UInt16 _index;

        public Output()
        {
            _index = 0xffff;
        }

        public string Name
        {
            get { return _name; }
            set { _name = value; }
        }

        public UInt16 Index
        {
            get { return _index; }
            set { _index = value; }
        }
    }

    public class ComMessage
    {
        private string _name;
        private UInt16 _index;

        public ComMessage()
        {
            _index = 0xffff;
        }

        public string Name
        {
            get { return _name; }
            set { _name = value; }
        }

        public UInt16 Index
        {
            get { return _index; }
            set { _index = value; }
        }
    }

    public class Component
    {
        private string _name;
        private string _type;
        private UInt32 _state;
        private UInt16 _index;
        private List<Output> _outputs;

        public Component()
        {
            _outputs = new List<Output>();
            _state = 0xffffffff;
            _index = 0xffff;
        }

        public string Name
        {
            get { return _name; }
            set { _name = value; }
        }

        public string Type
        {
            get { return _type; }
            set { _type = value; }
        }

        public UInt32 StateOffset
        {
            get { return _state; }
            set { _state = value; }
        }

        public UInt16 Index
        {
            get { return _index; }
            set { _index = value; }
        }

        public void AddOutput(Output output)
        {
            _outputs.Add(output);
        }

        public List<Output> Outputs()
        {
            return _outputs;
        }
    }

    public class ObjectData
    {
        private string _name;
        private UInt32 _instHandle;
        private List<Component> _components;
        private List<Component> _actions;
        private List<Component> _parameters;
        private List<Component> _vectors;
        private List<Component> _ctrls;
        private List<Component> _inputs;
        private List<Component> _outputs;
        private List<Component> _audio;
        private List<Component> _effects;

        private bool _swap = false;

        static private List<ComponentDef> _comDefs;
        static private List<ErrorDef> _errDefs;
        static private List<WarningDef> _warnDefs;

        public ObjectData(bool bigEndian)
        {
            _name = "Untitled";
            _instHandle = 0xffffffff;
            _components = new List<Component>();
            _actions = new List<Component>();
            _parameters = new List<Component>();
            _vectors = new List<Component>();
            _ctrls = new List<Component>();
            _inputs = new List<Component>();
            _outputs = new List<Component>();
            _audio = new List<Component>();
            _effects = new List<Component>();
            _swap = bigEndian;
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

        public List<Component> Components
        {
            get { return _components; }
            set { _components = value; }
        }

        public List<Component> Actions
        {
            get { return _actions; }
            set { _actions = value; }
        }

        public List<Component> Parameters
        {
            get { return _parameters; }
            set { _parameters = value; }
        }

        public List<Component> Vectors
        {
            get { return _vectors; }
            set { _vectors = value; }
        }

        public List<Component> Controls
        {
            get { return _ctrls; }
            set { _ctrls = value; }
        }

        public List<Component> Inputs
        {
            get { return _inputs; }
            set { _inputs = value; }
        }

        public List<Component> Outputs
        {
            get { return _outputs; }
            set { _outputs = value; }
        }

        public List<Component> Audio
        {
            get { return _audio; }
            set { _audio = value; }
        }

        public List<Component> Effects
        {
            get { return _effects; }
            set { _effects = value; }
        }

        public bool BigEndian
        {
            set { _swap = value; }
        }

        static public void Init()
        {
            _comDefs = new List<ComponentDef>();
            _errDefs = new List<ErrorDef>();
            _warnDefs = new List<WarningDef>();

            string xmlPath = System.Environment.GetEnvironmentVariable("BIZARRE_ROOT") + "\\tools\\common_tools\\win32\\audio_profiler\\components.xml";

            try
            {
                XmlDocument xDoc = new XmlDocument();

                xDoc.Load(xmlPath);

                //  get last child (skips xml def tag)
                XmlNode rootNode = xDoc.LastChild;

                if (rootNode.Name == "System")
                {
                    for (int item = 0; item < rootNode.ChildNodes.Count; item++)
                    {
                        XmlNode baseNode = rootNode.ChildNodes[item];

                        if (baseNode.Name == "Errors")
                        {
                            for (int count = 0; count < rootNode.ChildNodes.Count; count++)
                            {
                                XmlNode errNode = baseNode.ChildNodes[count];

                                if (errNode.Name == "Error")
                                {
                                    ErrorDef error = new ErrorDef();

                                    for (int att = 0; att < errNode.Attributes.Count; att++)
                                    {
                                        if (errNode.Attributes[att].Name == "index")
                                        {
                                            error.Index = Convert.ToInt32(errNode.Attributes[att].Value);
                                        }
                                        else if (errNode.Attributes[att].Name == "text")
                                        {
                                            error.Text = errNode.Attributes[att].Value;
                                        }
                                    }

                                    _errDefs.Add(error);
                                }
                            }
                        }
                        if (baseNode.Name == "Warnings")
                        {
                            for (int count = 0; count < rootNode.ChildNodes.Count; count++)
                            {
                                XmlNode errNode = baseNode.ChildNodes[count];

                                if (errNode.Name == "Warning")
                                {
                                    WarningDef warning = new WarningDef();

                                    for (int att = 0; att < errNode.Attributes.Count; att++)
                                    {
                                        if (errNode.Attributes[att].Name == "index")
                                        {
                                            warning.Index = Convert.ToInt32(errNode.Attributes[att].Value);
                                        }
                                        else if (errNode.Attributes[att].Name == "text")
                                        {
                                            warning.Text = errNode.Attributes[att].Value;
                                        }
                                    }

                                    _warnDefs.Add(warning);
                                }
                            }
                        }
                        else if (baseNode.Name == "Components")
                        {
                            for (int count = 0; count < baseNode.ChildNodes.Count; count++)
                            {
                                XmlNode comNode = baseNode.ChildNodes[count];

                                if (comNode.Name == "Component")
                                {
                                    ComponentDef component = new ComponentDef();

                                    for (int att = 0; att < comNode.Attributes.Count; att++)
                                    {
                                        if (comNode.Attributes[att].Name == "name")
                                        {
                                            component.Name = comNode.Attributes[att].Value;
                                        }
                                        else if (comNode.Attributes[att].Name == "type")
                                        {
                                            component.Type = Convert.ToUInt32(comNode.Attributes[att].Value);
                                        }
                                        else if (comNode.Attributes[att].Name == "action")
                                        {
                                            component.IsAction = comNode.Attributes[att].Value == "true" ? true : false;
                                        }
                                        else if (comNode.Attributes[att].Name == "parameter")
                                        {
                                            component.IsParameter = comNode.Attributes[att].Value == "true" ? true : false;
                                        }
                                        else if (comNode.Attributes[att].Name == "vector")
                                        {
                                            component.IsVector = comNode.Attributes[att].Value == "true" ? true : false;
                                        }
                                        else if (comNode.Attributes[att].Name == "control")
                                        {
                                            component.IsControl = comNode.Attributes[att].Value == "true" ? true : false;
                                        }
                                        else if (comNode.Attributes[att].Name == "input")
                                        {
                                            component.IsInput = comNode.Attributes[att].Value == "true" ? true : false;
                                        }
                                        else if (comNode.Attributes[att].Name == "output")
                                        {
                                            component.IsOutput = comNode.Attributes[att].Value == "true" ? true : false;
                                        }
                                        else if (comNode.Attributes[att].Name == "audio")
                                        {
                                            component.IsAudio = comNode.Attributes[att].Value == "true" ? true : false;
                                        }
                                        else if (comNode.Attributes[att].Name == "effect")
                                        {
                                            component.IsEffect = comNode.Attributes[att].Value == "true" ? true : false;
                                        }
                                        else
                                            throw new Exception(string.Format("Uknown component attribute"));
                                    }

                                    for (int i = 0; i < comNode.ChildNodes.Count; i++)
                                    {
                                        XmlNode comSubNode = comNode.ChildNodes[i];

                                        if (comSubNode.Name == "Data")
                                        {
                                            for (int j = 0; j < comSubNode.ChildNodes.Count; j++)
                                            {
                                                XmlNode comItemNode = comSubNode.ChildNodes[j];

                                                if (comItemNode.Name == "Float")
                                                {
                                                    component.DataSize += 4;
                                                }
                                                else if (comItemNode.Name == "Control")
                                                {
                                                    component.DataSize += 4;
                                                }
                                                else if (comItemNode.Name == "String")
                                                {
                                                    component.DataSize += 4;
                                                }
                                                else if (comItemNode.Name == "Option")
                                                {
                                                    component.DataSize += 8;
                                                }
                                                else if (comItemNode.Name == "Event")
                                                {
                                                    component.DataSize += 4;
                                                }
                                                else if (comItemNode.Name == "Size")
                                                {
                                                    ComponentDef.Child child = new ComponentDef.Child();
                                                    child.Offset = component.DataSize;

                                                    child.Size += 4;

                                                    component.AddChild(child);

                                                    component.DataSize += 2;
                                                }
                                                else if (comItemNode.Name == "Com")
                                                {
                                                    component.DataSize += 2;
                                                }
                                                else if (comItemNode.Name == "Int")
                                                {
                                                    component.DataSize += 4;
                                                }
                                                else if (comItemNode.Name == "UInt")
                                                {
                                                    component.DataSize += 4;
                                                }
                                                else if (comItemNode.Name == "UInt16")
                                                {
                                                    component.DataSize += 2;
                                                }
                                                else if (comItemNode.Name == "Array")
                                                {
                                                    ComponentDef.Child child = new ComponentDef.Child();
                                                    child.Offset = component.DataSize;

                                                    //  for array size value
                                                    component.DataSize += 2;
                                                    
                                                    for (int att = 0; att < comItemNode.Attributes.Count; att++)
                                                    {
                                                        if (comItemNode.Attributes[att].Name == "item")
                                                        {
                                                            child.Name = comNode.Attributes[att].Value;
                                                        }
                                                    }

                                                    for (int k = 0; k < comItemNode.ChildNodes.Count; k++)
                                                    {
                                                        XmlNode comArrayItemNode = comItemNode.ChildNodes[k];

                                                        if (comArrayItemNode.Name == "Float")
                                                        {
                                                            child.Size += 4;
                                                        }
                                                        else if (comArrayItemNode.Name == "Option")
                                                        {
                                                            child.Size += 8;
                                                        }
                                                        else if (comArrayItemNode.Name == "Com")
                                                        {
                                                            child.Size += 4;
                                                        }
                                                        else if (comArrayItemNode.Name == "String")
                                                        {
                                                            child.Size += 4;
                                                        }
                                                        else if (comArrayItemNode.Name == "Event")
                                                        {
                                                            child.Size += 4;
                                                        }
                                                        else
                                                            throw new Exception(string.Format("Unknown tag <{0:s}> in <Array>", comArrayItemNode.Name));
                                                    }

                                                    component.AddChild(child);
                                                }
                                                else
                                                    throw new Exception(string.Format("Unknown tag <{0:s}> in <Data>", comItemNode.Name));
                                            }

                                            //  align to 4 bytes
                                            component.DataSize = (component.DataSize + 3u) & ~3u;
                                        }
                                        else if (comSubNode.Name == "State")
                                        {
                                            for (int j = 0; j < comSubNode.ChildNodes.Count; j++)
                                            {
                                                XmlNode comItemNode = comSubNode.ChildNodes[j];

                                                if (comItemNode.Name == "Float")
                                                {
                                                    component.StateSize += 4;
                                                }
                                                else if (comItemNode.Name == "UInt")
                                                {
                                                    component.StateSize += 4;
                                                }
                                                else if (comItemNode.Name == "Int")
                                                {
                                                    component.StateSize += 4;
                                                }
                                                else if (comItemNode.Name == "FixedArray")
                                                {
                                                    UInt32 size = 0;
                                                    for (int att = 0; att < comItemNode.Attributes.Count; att++)
                                                    {
                                                        if (comItemNode.Attributes[att].Name == "size")
                                                        {
                                                            size = Convert.ToUInt32(comItemNode.Attributes[att].Value);
                                                        }
                                                        else
                                                            throw new Exception(string.Format("FixedArray has no size attribute"));
                                                    }

                                                    for (int k = 0; k < comItemNode.ChildNodes.Count; k++)
                                                    {
                                                        XmlNode comArrayItemNode = comItemNode.ChildNodes[k];

                                                        if (comArrayItemNode.Name == "Float")
                                                        {
                                                            component.StateSize += size * 4;
                                                        }
                                                        else if (comArrayItemNode.Name == "UInt")
                                                        {
                                                            component.StateSize += size * 4;
                                                        }
                                                        else if (comArrayItemNode.Name == "Int")
                                                        {
                                                            component.StateSize += size * 4;
                                                        }
                                                        else
                                                            throw new Exception(string.Format("Unknown tag <{0:s}> in <FixedArray>", comSubNode.Name));
                                                    }
                                                }
                                                else
                                                    throw new Exception(string.Format("Unknown tag <{0:s}> in <State>", comItemNode.Name));
                                            }

                                            //  align to 16 bytes
                                            component.StateSize = (component.StateSize + 15) & ~15U;
                                        }
                                        else if (comSubNode.Name == "Outputs")
                                        {
                                            for (int j = 0; j < comSubNode.ChildNodes.Count; j++)
                                            {
                                                XmlNode comItemNode = comSubNode.ChildNodes[j];

                                                if (comItemNode.Name == "Output")
                                                {
                                                    Output output = new Output();

                                                    for (int att = 0; att < comItemNode.Attributes.Count; att++)
                                                    {
                                                        if (comItemNode.Attributes[att].Name == "name")
                                                        {
                                                            output.Name = comItemNode.Attributes[att].Value;
                                                        }
                                                        else if (comItemNode.Attributes[att].Name == "index")
                                                        {
                                                            output.Index = Convert.ToUInt16(comItemNode.Attributes[att].Value);
                                                        }
                                                        else
                                                            throw new Exception(string.Format("Unknown attribute {0:s} in <Output>", comItemNode.Name));
                                                    }

                                                    component.AddOutput(output);
                                                }
                                                else
                                                    throw new Exception(string.Format("Unknown tag <{0:s}> in <Outputs>", comSubNode.Name));
                                            }
                                        }
                                        else if (comSubNode.Name == "Messages")
                                        {
                                            for (int j = 0; j < comSubNode.ChildNodes.Count; j++)
                                            {
                                                XmlNode comItemNode = comSubNode.ChildNodes[j];

                                                if (comItemNode.Name == "Message")
                                                {
                                                    ComMessage msg = new ComMessage();

                                                    for (int att = 0; att < comItemNode.Attributes.Count; att++)
                                                    {
                                                        if (comItemNode.Attributes[att].Name == "name")
                                                        {
                                                            msg.Name = comItemNode.Attributes[att].Value;
                                                        }
                                                        else if (comItemNode.Attributes[att].Name == "index")
                                                        {
                                                            msg.Index = Convert.ToUInt16(comItemNode.Attributes[att].Value);
                                                        }
                                                        else
                                                            throw new Exception(string.Format("Unknown attribute {0:s} in <Output>", comItemNode.Name));
                                                    }

                                                    component.AddMessage(msg);
                                                }
                                                else
                                                    throw new Exception(string.Format("Unknown tag <{0:s}> in <Outputs>", comSubNode.Name));
                                            }
                                        }
                                        else
                                            throw new Exception(string.Format("Unknown tag <{0:s}> in <Component>", comSubNode.Name));
                                    }

                                    //  add id and align to 4 bytes
                                    component.DataSize = 4 + (component.DataSize + 3) & ~3U;

                                    //  add to coms
                                    _comDefs.Add(component);

                                    //Console.WriteLine("{0:s} data={1:d} state={2:d}", component.Name, component.DataSize, component.StateSize);
                                }
                                else
                                    throw new Exception(string.Format("Unknown tag <{0:s}> in <Components>", comNode.Name));
                            }
                        }
                    }
                }
                else
                    throw new Exception(string.Format("Unknown root tag <{0:s}>", rootNode.Name));
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "Error reading component defs", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public UInt32 DoEndian(UInt32 val)
        {
            return (UInt32)IPAddress.HostToNetworkOrder((int)val);
        }

        public UInt16 DoEndian(UInt16 val)
        {
            return (UInt16)IPAddress.HostToNetworkOrder((short)val);
        }

        public UInt32 GetU32(byte[] data, UInt32 offset)
        {
            if (_swap)
            {
                byte[] temp = new byte[4];
                temp[0] = data[offset + 3];
                temp[1] = data[offset + 2];
                temp[2] = data[offset + 1];
                temp[3] = data[offset];
                return BitConverter.ToUInt32(temp, 0);
            }
            else
                return BitConverter.ToUInt32(data, (int)offset);
        }

        public UInt16 GetU16(byte[] data, UInt32 offset)
        {
            if (_swap)
            {
                byte[] temp = new byte[2];
                temp[0] = data[offset + 1];
                temp[1] = data[offset];
                return BitConverter.ToUInt16(temp, 0);
            }
            else
                return BitConverter.ToUInt16(data, (int)offset);
        }

        public Single GetF32(byte[] data, UInt32 offset)
        {
            if (_swap)
            {
                byte[] temp = new byte[4];
                temp[0] = data[offset + 3];
                temp[1] = data[offset + 2];
                temp[2] = data[offset + 1];
                temp[3] = data[offset];
                return BitConverter.ToSingle(temp, 0);
            }
            else
                return BitConverter.ToSingle(data, (int)offset);
        }

        public string GetString(byte[] data, UInt32 offset)
        {
            try
            {
                int length = 0;
                while (data[offset + length] != 0)
                    length++;

                ASCIIEncoding enc = new ASCIIEncoding();
                return enc.GetString(data, (int)offset, length);
            }
            catch
            {
                return "";
            }
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            UInt32 size;
            byte[] data;
            msgReader.Read(out _instHandle);
            msgReader.Read(out size);
            if (size > 0)
            {
                data = new byte[size];
                msgReader.Read(data, (int)size);

                UInt32 tagAOBJ = GetU32(data, 0);
                UInt16 verMaj = GetU16(data, 4);
                UInt16 verMin = GetU16(data, 6);
                UInt32 stringTable = GetU32(data, 8);
                UInt32 nameStr = GetU32(data, 12);
                UInt32 numComs = GetU32(data, 16);
                UInt32 frameSkip = GetU32(data, 20);

                _name = GetString(data, stringTable + nameStr);

                UInt32 next = 24;
                UInt32 state = 0;
                for (int i = 0; i < numComs; i++)
                {
                    UInt32 comId = GetU32(data, next);
                    bool found = false;
                    foreach (ComponentDef com in _comDefs)
                    {
                        if (comId == com.Type)
                        {
                            //  offset for com id
                            UInt32 current = next + 4;

                            //Console.WriteLine(com.Name);

                            Component component = new Component();
                            component.Name = GetString(data, stringTable + GetU32(data, current));
                            component.Type = com.Name;
                            component.StateOffset = state;
                            component.Index = (UInt16)i;
                            _components.Add(component);

                            if (com.IsAction)
                                _actions.Add(component);

                            if (com.IsParameter)
                                _parameters.Add(component);

                            if (com.IsVector)
                                _vectors.Add(component);

                            if (com.IsControl)
                            {
                                _ctrls.Add(component);

                                foreach (Output output in com.Outputs())
                                    component.AddOutput(output);
                            }

                            if (com.IsInput)
                                _inputs.Add(component);

                            if (com.IsOutput)
                                _outputs.Add(component);

                            if (com.IsAudio)
                                _audio.Add(component);

                            if (com.IsEffect)
                                _effects.Add(component);

                            foreach (ComponentDef.Child child in com.Children())
                            {
                                UInt16 num = GetU16(data, current + child.Offset);
                                next += num * child.Size;
                            }

                            next += com.DataSize;
                            state += com.StateSize;

                            found = true;

                            break;
                        }
                    }

                    if (!found)
                    {
                        MessageBox.Show(string.Format("Unknown component id {0:d}", comId), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        break;
                    }
                }
            }
            else
            {
                //  fake an object
            }
        }

        public UInt16 GetComponentHandleFromName(string name)
        {
            foreach (Component com in _components)
            {
                if (com.Name == name)
                    return com.Index;
            }

            return 0xffff;
        }

        public string GetComponentNameFromHandle(UInt16 comId)
        {
            foreach (Component com in _components)
            {
                if (com.Index == comId)
                    return com.Name;
            }

            return "";
        }

        public string GetMessageNameFromHandle(UInt16 comId, UInt16 msgId)
        {
            string comType = "";
            foreach (Component com in _components)
            {
                if (com.Index == comId)
                    comType = com.Type;
            }

            foreach (ComponentDef comDef in _comDefs)
            {
                if (comDef.Name == comType)
                {
                    if (comDef.HasMessages)
                    {
                        foreach (ComMessage msg in comDef.Messages())
                        {
                            if (msg.Index == msgId)
                                return msg.Name;
                        }
                    }
                }
            }

            return "";
        }

        public static string GetErrorTextFromHandle(Int16 errId)
        {
            foreach (ErrorDef errDef in _errDefs)
            {
                if (errDef.Index == errId)
                {
                    return errDef.Text;
                }
            }

            return "";
        }

        public static string GetWarningTextFromHandle(Int16 warnId)
        {
            foreach (WarningDef warnDef in _warnDefs)
            {
                if (warnDef.Index == warnId)
                {
                    return warnDef.Text;
                }
            }

            return "";
        }
    }
}
