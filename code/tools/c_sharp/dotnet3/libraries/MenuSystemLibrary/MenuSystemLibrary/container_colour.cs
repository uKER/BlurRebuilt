using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace MenuSystemLibrary
{

    public partial class Container_Colour : MenuSystemLibrary.Container_Base
    {
        private Colour m_colour;
        private bool m_bExpanded;

        public Container_Colour()
        {
            InitializeComponent();
            m_colour = new Colour(1.0f);
            colourButton.BackColor = Color.FromArgb(m_colour.AlphaAsbyte(), m_colour.RedAsbyte(),
                                                    m_colour.GreenAsbyte(), m_colour.BlueAsbyte());
            Expand(false);
        }

		public Container_Colour(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
            : base(owner, xContainer, bCreateChildren)
        {
            InitializeComponent();

            m_colour = new Colour(1.0f);
            colourButton.BackColor = Color.FromArgb(m_colour.AlphaAsbyte(), m_colour.RedAsbyte(),
                                                    m_colour.GreenAsbyte(), m_colour.BlueAsbyte());
            Expand(false);
        }

        override internal string ObjectName
        {
            //			get { return this.checkBox.Text; }
            set { this.name.Text = value; }
        }

        override internal string ObjectValue
        {
            get { return ColorToARGBAsText(m_colour); }
            set
            {
                m_colour = TextARGBAsColor(value as string);

                //update scrollbars, sliders and picker back colour...

                Colour colour = new Colour(m_colour);
               
                alphaSlider.Value = (int)(Colour.Clamp(m_colour.a * 100.0f, 100.0f));
                redSlider.Value = (int)(Colour.Clamp(m_colour.r * 100.0f, 100.0f));
                greenSlider.Value = (int)(Colour.Clamp(m_colour.g * 100.0f, 100.0f));
                blueSlider.Value = (int)(Colour.Clamp(m_colour.b * 100.0f, 100.0f));

                m_colour.SetFromColour(colour);
                colourButton.BackColor = Color.FromArgb(m_colour.AlphaAsbyte(), m_colour.RedAsbyte(),
                                                        m_colour.GreenAsbyte(), m_colour.BlueAsbyte());

                m_colour.SetFromColour(colour);
                alphaScroll.Value = (decimal)m_colour.a;
                m_colour.SetFromColour(colour);
                redScroll.Value = (decimal)m_colour.r;
                m_colour.SetFromColour(colour);
                greenScroll.Value = (decimal)m_colour.g;
                m_colour.SetFromColour(colour);
                blueScroll.Value = (decimal)m_colour.b;
                m_colour.SetFromColour(colour);
            }
        }

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			this.alphaScroll.ReadOnly = bReadonly;
			this.alphaSlider.Enabled = !bReadonly;
			
			this.redScroll.ReadOnly = bReadonly;
			this.redSlider.Enabled = !bReadonly;

			this.greenScroll.ReadOnly = bReadonly;
			this.greenSlider.Enabled = !bReadonly;

			this.blueScroll.ReadOnly = bReadonly;
			this.blueSlider.Enabled = !bReadonly;

			this.colourButton.Enabled = !bReadonly;

			//this.m_bShadowEnabled = true;	// Set to true by default.
		}

        private void colourScroll_ValueChanged(object sender, EventArgs e)
        {
            m_colour.a = (float)alphaScroll.Value;
            m_colour.r = (float)redScroll.Value;
            m_colour.g = (float)greenScroll.Value;
            m_colour.b = (float)blueScroll.Value;

            alphaSlider.Value = (int)(Colour.Clamp(m_colour.a * 100.0f, 100.0f));
            redSlider.Value = (int)(Colour.Clamp(m_colour.r * 100.0f, 100.0f));
            greenSlider.Value = (int)(Colour.Clamp(m_colour.g * 100.0f, 100.0f));
            blueSlider.Value = (int)(Colour.Clamp(m_colour.b * 100.0f, 100.0f));

            colourButton.BackColor = Color.FromArgb(m_colour.AlphaAsbyte(), m_colour.RedAsbyte(),
                                                    m_colour.GreenAsbyte(), m_colour.BlueAsbyte());
            ValueChanged("value", ColorToARGBAsText(m_colour), false);
        }

        private void colourButton_Click(object sender, EventArgs e)
        {
            ColorDialog dlg = new ColorDialog();
            DialogResult result = dlg.ShowDialog();
            m_colour.SetARGBFromBytes(dlg.Color.A, dlg.Color.R, dlg.Color.G, dlg.Color.B);
            colourButton.BackColor = dlg.Color;
            alphaScroll.Value = (decimal)m_colour.a;
            alphaSlider.Value = (int)(m_colour.a * 100.0f);
            m_colour.SetARGBFromBytes(dlg.Color.A, dlg.Color.R, dlg.Color.G, dlg.Color.B);
            redScroll.Value = (decimal)m_colour.r;
            redSlider.Value = (int)(m_colour.r * 100.0f);
            m_colour.SetARGBFromBytes(dlg.Color.A, dlg.Color.R, dlg.Color.G, dlg.Color.B);
            greenScroll.Value = (decimal)m_colour.g;
            greenSlider.Value = (int)(m_colour.g * 100.0f);
            m_colour.SetARGBFromBytes(dlg.Color.A, dlg.Color.R, dlg.Color.G, dlg.Color.B);
            blueScroll.Value = (decimal)m_colour.b;
            blueSlider.Value = (int)(m_colour.b * 100.0f);
            m_colour.SetARGBFromBytes(dlg.Color.A, dlg.Color.R, dlg.Color.G, dlg.Color.B);
            ValueChanged("value", ColorToARGBAsText(m_colour), false);
        }

        private string ColorToARGBAsText(Colour color)
        {
            return String.Format("{0} {1} {2} {3}", color.a, color.r, color.g, color.b);
        }

        private Colour TextARGBAsColor(string colourString)
        {
            try
            {
                string[] array = colourString.Split(' ');
                Colour colour = new Colour(float.Parse(array[0]), float.Parse(array[1]), float.Parse(array[2]), float.Parse(array[3]));
                return colour;
            }
            catch (Exception e)
            {
               Console.WriteLine(e.ToString());
               Colour colour = new Colour(1.0f);
               return colour;
            }
        }

        private void alphaSlider_Scroll(object sender, EventArgs e)
        {
            TrackBar tb = sender as TrackBar;
            float fVal = (float)tb.Value;
            fVal *= 0.01f;
            decimal dVal = (decimal)fVal;
            alphaScroll.Value = dVal;
        }

        private void redSlider_Scroll(object sender, EventArgs e)
        {
            TrackBar tb = sender as TrackBar;
            float fVal = (float)tb.Value;
            fVal *= 0.01f;
            decimal dVal = (decimal)fVal;
            redScroll.Value = dVal;
        }

        private void greenSlider_Scroll(object sender, EventArgs e)
        {
            TrackBar tb = sender as TrackBar;
            float fVal = (float)tb.Value;
            fVal *= 0.01f;
            decimal dVal = (decimal)fVal;
            greenScroll.Value = dVal;
        }

        private void blueSlider_Scroll(object sender, EventArgs e)
        {
            TrackBar tb = sender as TrackBar;
            float fVal = (float)tb.Value;
            fVal *= 0.01f;
            decimal dVal = (decimal)fVal;
            blueScroll.Value = dVal;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (m_bExpanded)
            {
                Expand(false);
            }
            else
            {
                Expand(true);
            }
             m_owner.LayoutAllChildren();
       }

        private void Expand(bool bExpand)
        {
            m_bExpanded = bExpand;

            if (m_bExpanded)
            {
                this.Height = blueSlider.Bottom + 3;
            }
            else
            {
                this.Height = colourButton.Bottom+6;
            }
        }

    }
    public class Colour
    {
        public float a, r, g, b;
        public Colour()
        {
            a = r = g = b = 1.0f;
        }

        public Colour(Colour c)
        {
            a = c.a;
            r = c.r;
            g = c.g;
            b = c.b;
        }
        public Colour(float f)
        {
            a = r = g = b = f;
        }
        public Colour(float fa, float fr, float fg, float fb)
        {
            a = fa;
            r = fr;
            g = fg;
            b = fb;
        }

        static public float Clamp(float f, float clamp)
        {
            if (f > clamp)
            {
                return clamp;
            }
            return f;
        }

        public int AlphaAsbyte()
        {
            return (int)Clamp(a * 255.0f, 255.0f);
        }
        public int RedAsbyte()
        {
            return (int)Clamp(r * 255.0f, 255.0f);
        }
        public int GreenAsbyte()
        {
            return (int)Clamp(g * 255.0f, 255.0f);
        }
        public int BlueAsbyte()
        {
            return (int)Clamp(b * 255.0f, 255.0f);
        }

        public void SetARGBFromBytes(int ia, int ir, int ig, int ib)
        {
            a = (float)ia / 255.0f;
            r = (float)ir / 255.0f;
            g = (float)ig / 255.0f;
            b = (float)ib / 255.0f;
        }

        public void SetFromColour(Colour c)
        {
            a = c.a;
            r = c.r;
            g = c.g;
            b = c.b;
        }

        public void SetAlphaFromByte(int val)
        {
            a = (float)val / 255.0f;
        }
        public void SetRedFromByte(int val)
        {
            r = (float)val / 255.0f;
        }
        public void SetGreenFromByte(int val)
        {
            g = (float)val / 255.0f;
        }
        public void SetBlueFromByte(int val)
        {
            b = (float)val / 255.0f;
        }
    }

}

