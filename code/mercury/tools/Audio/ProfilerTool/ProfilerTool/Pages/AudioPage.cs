using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilerTool.Pages
{
    class AudioPage : Page
    {
        Controls.ObjectTreeNode _objNode = null;   //  for AudioGraphData
        List<Controls.AudioItem> _items;

        public AudioPage(Windows.InstanceWnd instWnd)
            : base(instWnd)
        {
            //  prefs
            ItemSize = new Size(135, 30);
            Spacing = new Point(35, 5);

            _items = new List<Controls.AudioItem>();
        }

        public override Control CreateControl(string name, Controls.ComponentTreeNode comNode)
        {
            if (_objNode == null)
            {
                //  TODO: validate?
                _objNode = comNode.Parent.Parent.Parent as Controls.ObjectTreeNode;
            }

            Controls.AudioItem item = new Controls.AudioItem();
            item.Text = name;
            item.ComponentHandle = comNode.ComponentHandle;
            item.ComponentName = name;
            item.ComponentType = comNode.ComponentType;

            item.MuteChanged += new EventHandler(AudioItem_MuteChanged);

            _items.Add(item);

            return item;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            Pen pen = new Pen(Color.Gray, 2);
            Pen pen2 = new Pen(Color.Gray, 1);
            SolidBrush brush = new SolidBrush(Color.LightGray);

            foreach (Packets.AudioGraphData.AudioGraphDataItem graphItem in _objNode.AudioGraphData.Items)
            {
                int x1 = GetX(graphItem.Depth);
                int y1 = GetY(graphItem.Height);

                e.Graphics.FillRectangle(brush, x1 + 1, y1 + 1, ItemSize.Width - 1, ItemSize.Height - 1);
                e.Graphics.DrawRectangle(pen2, x1+1, y1+1, ItemSize.Width - 1, ItemSize.Height - 1);

                y1 += ItemSize.Height / 2;

                foreach (Packets.AudioGraphData.AudioGraphDataItem graphInput in graphItem._inputComs)
                {
                    //if (item.Inputs[i].Visible)
                    {
                        int x2 = GetX(graphInput.Depth) + ItemSize.Width;
                        int y2 = GetY(graphInput.Height) + ItemSize.Height / 2;

                        e.Graphics.DrawLine(pen, x1, y1, x2, y2);
                    }
                }
            }

            base.OnPaint(e);
        }

        public override void AlignControls()
        {
            foreach (Controls.AudioItem item in _items)
            {
                if (item.Visible)
                {
                    foreach (Packets.AudioGraphData.AudioGraphDataItem graphItem in _objNode.AudioGraphData.Items)
                    {
                        if (graphItem.com == item.ComponentHandle)
                        {
                            item.Location = new Point(GetX(graphItem.Depth), GetY(graphItem.Height));
                            break;
                        }
                    }
                }
            }

            _height = 0;

            if (!Minimized)
            {
                foreach (Packets.AudioGraphData.AudioGraphDataItem graphItem in _objNode.AudioGraphData.Items)
                    _height = Math.Max(_height, GetY(graphItem.Height));

                if (_height > 0)
                    _height += _topleft.Y + (ItemSize.Height + Spacing.Y) + Border.Y;
            }
        }

        private int GetX(int depth)
        {
            return _topleft.X + Border.X + depth * (ItemSize.Width + Spacing.X);
        }

        private int GetY(int height)
        {
            return _topleft.Y + Border.Y + height * (ItemSize.Height + Spacing.Y);
        }

        public void SetInfo(string name, Packets.AudioData audioData)
        {
            foreach (Controls.AudioItem item in _items)
            {
                if (item.Text == name)
                {
                    item.UpdateInfo(audioData, _items);
                    item.AudioActive = audioData.IsProcessing;
                    break;
                }
            }

            //  TODO: remove this
            //AlignControls();
        }

        private void AudioItem_MuteChanged(object sender, EventArgs e)
        {
            Controls.AudioItem item = sender as Controls.AudioItem;

            InstanceWindow.SendMuteEvent(item.Text, item.AudioMuted);
        }
    }
}
