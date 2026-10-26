using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Collections;
using System.Xml;
using Global;
using System.Diagnostics;

namespace MenuSystemLibrary
{
	public partial class Container_Number : MenuSystemLibrary.Container_Base
	{
		Color m_backColour;

		internal Container_Number()
		{
			InitializeComponent();
		}

		int iMin;
		int iMax;
		float fMin, fMax;
		bool bUseMinMax;
		bool bIsInt;				// Int or float.
		string strTooltip;

		public Container_Number(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();
		}

		private void Container_Number_Load(object sender, EventArgs e)
		{
			m_backColour = this.textBox.BackColor;

			string strType = XmlHelper.GetKeyValue(m_xObject, "type");
			if(strType == null)
				strType = "(unknown)";

			if(strType == "int") bIsInt = true; else bIsInt = false;

			string strMin = XmlHelper.GetKeyValue(m_xObject, "min");
			string strMax = XmlHelper.GetKeyValue(m_xObject, "max");

			if(strMin == null || strMax == null)
			{
				bExpanded = false;
				bUseMinMax = false;
				this.Width = this.buttonExpand.Left;
				this.textBox.Anchor |= AnchorStyles.Right;

				Graphics g = this.CreateGraphics();
				string strName = XmlHelper.GetKeyValue(m_xObject, "display_name");
				SizeF fs = new SizeF(g.MeasureString(strName, this.textBox.Font));
				g.Dispose();
				int textWidth = (int)fs.Width;
				int textHeight = (int)fs.Height;



				this.Width = Math.Max(buttonExpand.Left, textWidth);
				this.buttonExpand.Visible = false;
				this.trackBar.Visible = false;
			}
			else
			{
				if(bIsInt)
				{
					SetValueFromString(ref iMin, strMin, 0);
					SetValueFromString(ref iMax, strMax, 100);
					strTooltip = "Min: " + iMin.ToString() + " Max: " + iMax.ToString();
				}
				else
				{
					SetValueFromString(ref fMin, strMin, 0.0f);
					SetValueFromString(ref fMax, strMax, 100.0f);
					strTooltip = "Min: " + fMin.ToString() + " Max: " + fMax.ToString();
				}

				bUseMinMax = true;
				bExpanded = true;

				this.trackBar.Minimum = 0;
				this.trackBar.Maximum = this.trackBar.Width;

				bExpanded = false;
				ToggleSize();
			}
		}

		private void SetValueFromString(ref int dest, string strVal, int defaultVal)
		{
			try
			{
				dest = int.Parse(strVal);
			}
			catch
			{
				dest = defaultVal;
			}
		}

		private void SetValueFromString(ref float dest, string strVal, float defaultVal)
		{
			try
			{
				dest = float.Parse(strVal);
			}
			catch
			{
				dest = defaultVal;
			}
		}

		override internal string ObjectName
		{
//			get { return this.label.Text; }
			set { this.label.Text = value; }
		}

		override internal string ObjectValue
		{ 
			get { return this.textBox.Text; } 
			set
			{
				if(value == MenuSystem.CONFLICTED_MARKER)
				{
					CreateConflictedButton("CONFLICTED", this.textBox.Left, this.textBox.Top, this.textBox.Width, this.textBox.Height);
				}
				else
					this.textBox.Text = value;
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.textBox;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			this.textBox.ReadOnly = bReadonly;
			this.trackBar.Enabled = !bReadonly;

			if(bReadonly == true)
				this.textBox.BackColor = SystemColors.Control;
			else
				this.textBox.BackColor = SystemColors.Window;
		}

		bool bInChanged = false;

		private void textBox_TextChanged(object sender, EventArgs e)
		{
			if(bInChanged == false)
			{
				bInChanged = true;

				try
				{
					this.textBox.BackColor = m_backColour;


					if(bIsInt == true)
					{
						int value = int.Parse(this.textBox.Text);
						if(bUseMinMax == true)
						{
							int trackVal = (value - iMin) * trackBar.Width / (iMax - iMin);

							int newVal = trackVal;

							if(newVal < trackBar.Minimum) newVal = trackBar.Minimum;
							if(newVal > trackBar.Maximum) newVal = trackBar.Maximum;

							if(bInTrackBarScroll == false)
							{
								this.trackBar.Value = newVal;
								this.trackBar.Value = trackVal;
							}
						}
					}
					else
					{
						float value = float.Parse(this.textBox.Text);
						if(bUseMinMax == true)
						{

							float trackVal = (value - fMin) * trackBar.Width / (fMax - fMin);

							float newVal = trackVal;

							if(newVal < trackBar.Minimum) newVal = trackBar.Minimum;
							if(newVal > trackBar.Maximum) newVal = trackBar.Maximum;

							if(bInTrackBarScroll == false)
							{
								this.trackBar.Value = (int)newVal;
								this.trackBar.Value = (int)trackVal;
							}
						}
					}

				}
				catch
				{
					this.textBox.BackColor = Color.Red;
				}

				ValueChanged("value", this.textBox.Text, false);

				bInChanged = false;
			}
		}

		bool bExpanded = true;
		private void buttonExpand_Click(object sender, EventArgs e)
		{
			ToggleSize();
			m_owner.LayoutAllChildren();
		}

		void ToggleSize()
		{
			if(bExpanded == true)
			{
				bExpanded = false;
				this.Width = this.buttonExpand.Right;
				this.buttonExpand.Text = ">";
			}
			else
			{
				bExpanded = true;
				this.Width = this.trackBar.Right;
				this.buttonExpand.Text = "<";
			}
		}

		bool bInTrackBarScroll = false;

		private void trackBar_Scroll(object sender, EventArgs e)
		{
			if(bInTrackBarScroll == false)
			{
				bInTrackBarScroll = true;
				if(bIsInt == true)
				{
					int newValue = trackBar.Value * (iMax - iMin) / trackBar.Maximum + iMin;
					this.textBox.Text = newValue.ToString();
				}
				else
				{
					float newValue = trackBar.Value * (fMax - fMin) / trackBar.Maximum + fMin;
					this.textBox.Text = newValue.ToString();
				}
			}
			bInTrackBarScroll = false;
		}

		protected override string GetExtraStatusText()
		{
			return strTooltip;
		}


	}
}

