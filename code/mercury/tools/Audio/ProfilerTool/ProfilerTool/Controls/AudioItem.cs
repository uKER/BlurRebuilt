using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    class AudioItem : UserControl
    {
        private Label lblCom;
        private Label lblName;

        private int _column = 0;
        private int _row = 0;
        private UInt16 _comHandle = 0xffff;
        private AudioItem[] _inputs;
        private bool _hasOutput = false;
        private ProfilerTool.Controls.LED ledMuted;
        private LED ledActive;

        public event EventHandler MuteChanged;

        public AudioItem()
        {
            InitializeComponent();
        }

        private void InitializeComponent()
        {
            this.lblCom = new System.Windows.Forms.Label();
            this.lblName = new System.Windows.Forms.Label();
            this.ledActive = new ProfilerTool.Controls.LED();
            this.ledMuted = new ProfilerTool.Controls.LED();
            this.SuspendLayout();
            // 
            // lblCom
            // 
            this.lblCom.AutoSize = true;
            this.lblCom.Location = new System.Drawing.Point(0, 0);
            this.lblCom.Name = "lblCom";
            this.lblCom.Size = new System.Drawing.Size(61, 13);
            this.lblCom.TabIndex = 0;
            this.lblCom.Text = "Component";
            // 
            // lblName
            // 
            this.lblName.AutoSize = true;
            this.lblName.Location = new System.Drawing.Point(0, 15);
            this.lblName.Name = "lblName";
            this.lblName.Size = new System.Drawing.Size(51, 13);
            this.lblName.TabIndex = 0;
            this.lblName.Text = "unknown";
            // 
            // ledActive
            // 
            this.ledActive.Location = new System.Drawing.Point(121, 12);
            this.ledActive.Name = "ledActive";
            this.ledActive.Size = new System.Drawing.Size(12, 12);
            this.ledActive.Style = ProfilerTool.Controls.LED.Styles.Green;
            this.ledActive.TabIndex = 1;
            // 
            // ledMuted
            // 
            this.ledMuted.Location = new System.Drawing.Point(121, 0);
            this.ledMuted.Name = "ledMuted";
            this.ledMuted.Size = new System.Drawing.Size(12, 12);
            this.ledMuted.Style = ProfilerTool.Controls.LED.Styles.Red;
            this.ledMuted.Click += new EventHandler(MutedLED_Click);
            this.ledMuted.TabIndex = 1;
            // 
            // AudioItem
            // 
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.ledActive);
            this.Controls.Add(this.ledMuted);
            this.Controls.Add(this.lblName);
            this.Controls.Add(this.lblCom);
            this.Name = "AudioItem";
            this.Size = new System.Drawing.Size(135, 30);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        public int Depth
        {
            get { return _column; }
            set { _column = value; }
        }

        public int Row
        {
            get { return _row; }
            set { _row = value; }
        }

        public AudioItem[] Inputs
        {
            get { return _inputs; }
        }

        public bool HasOutput
        {
            get { return _hasOutput; }
            set { _hasOutput = value; }
        }

        public UInt16 ComponentHandle
        {
            get { return _comHandle; }
            set { _comHandle = value; }
        }

        public string ComponentName
        {
            get { return lblName.Text; }
            set { lblName.Text = value; }
        }

        public string ComponentType
        {
            get { return lblCom.Text; }
            set { lblCom.Text = value; }
        }

        public bool AudioActive
        {
            get { return ledActive.On == LED.State.On; }
            set { ledActive.On = value ? LED.State.On : LED.State.Off; }
        }

        public bool AudioMuted
        {
            get { return ledMuted.On == LED.State.On; }
            set
            {
                ledMuted.On = value ? LED.State.On : LED.State.Off;
            }
        }

        public void UpdateInfo(Packets.AudioData audioData, List<Controls.AudioItem> items)
        {
            return;

            //  first time create
            if (_inputs == null)
                _inputs = new AudioItem[audioData.Inputs.Count];

            //  go through comIds in packet
            for (int i = 0; i < audioData.Inputs.Count; i++)
            {
                UInt16 comHandle = audioData.Inputs[i];

                //  find a match in page's items
                foreach (Controls.AudioItem input in items)
                {
                    //  set the item's input item
                    if (input.ComponentHandle == comHandle)
                        _inputs[i] = input;
                }
            }
        }

        public void SetDepth(int depth)
        {
            _column = depth;

            if (_inputs != null)
            {
                foreach (Controls.AudioItem input in _inputs)
                {
                    //  set the input item's depths
                    input.SetDepth(depth + 1);
                }
            }
        }

        private void MutedLED_Click(object sender, EventArgs e)
        {
            AudioMuted = !AudioMuted;

            if (MuteChanged != null)
                MuteChanged(this, e);
        }
    }
}
