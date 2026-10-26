using System;
using System.Drawing;
using System.ComponentModel;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public class LED : Panel
    {
        Bitmap bmpOn;
        Bitmap bmpOff;
        private Styles _style = Styles.None;
        private State _on = State.None;

        public enum Styles
        {
            Red,
            Green,
            Yellow,
            Black,
            None
        };

        public enum State
        {
            On,
            Off,
            None
        };

        [Bindable(true), Category("Appearance"),
        DefaultValue(typeof(Styles), "Red"), Description("The LED Color")]
        public Styles Style
        {
            get { return _style; }
            set
            {
                if (_style != value)
                {
                    _style = value;

                    switch (Style)
                    {
                        case Styles.Red:
                            bmpOn = global::ProfilerTool.Properties.Resources.LED_Red_On;
                            bmpOff = global::ProfilerTool.Properties.Resources.LED_Red_Off;
                            break;
                        case Styles.Green:
                            bmpOn = global::ProfilerTool.Properties.Resources.LED_Green_On;
                            bmpOff = global::ProfilerTool.Properties.Resources.LED_Green_Off;
                            break;
                        case Styles.Yellow:
                            bmpOn = global::ProfilerTool.Properties.Resources.LED_Yellow_On;
                            bmpOff = global::ProfilerTool.Properties.Resources.LED_Yellow_Off;
                            break;
                    }

                    bmpOn.MakeTransparent(Color.Magenta);
                    bmpOff.MakeTransparent(Color.Magenta);

                    Invalidate();
                }
            }
        }

        public LED()
        {
            bmpOn = global::ProfilerTool.Properties.Resources.LED_Black;
            bmpOff = global::ProfilerTool.Properties.Resources.LED_Black;

            bmpOn.MakeTransparent(Color.Magenta);
            bmpOff.MakeTransparent(Color.Magenta);
        }

        [Bindable(true), Category("Appearance"),
        DefaultValue(false), Description("The LED State")]
        public State On
        {
            get
            {
                return _on;
            }
            set
            {
                if (_on != value)
                {
                    _on = value;

                    Invalidate();
                }
            }
        }

        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams createParams = base.CreateParams;
                createParams.ExStyle |= 0x00000020; // WS_EX_TRANSPARENT
                return createParams;
            }
        }
 
        protected override void OnPaintBackground(PaintEventArgs e)
        {
            //  don't paint the background.
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            //if (bmpOn != null)
            {
                int width = bmpOn.Size.Width;
                int height = bmpOn.Size.Height;

                Rectangle size = new Rectangle(0, 0, width, height);

                if (On == State.On)
                    e.Graphics.DrawImage(bmpOn, size);
                else
                    e.Graphics.DrawImage(bmpOff, size);
            }
        }
    }
}
