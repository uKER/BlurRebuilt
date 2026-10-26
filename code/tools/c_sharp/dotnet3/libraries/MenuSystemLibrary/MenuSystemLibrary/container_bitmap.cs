using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Collections;
using Global;

namespace MenuSystemLibrary
{
	public partial class Container_Bitmap : MenuSystemLibrary.Container_Base
	{
		public Container_Bitmap()
		{
			InitializeComponent();
		}

		public Container_Bitmap(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

//			string strDefault = XmlHelper.GetKeyValue(m_xObject, "default");
//			XmlHelper.AddAttribute(m_xObject, "value", strDefault);

			string strBitmap = XmlHelper.GetKeyValue(m_xObject, "bitmap");

			/// Load bitmap.
			Bitmap bmp;
			try
			{
				bmp = new Bitmap(strBitmap);
				this.panel1.BackgroundImage = bmp;
			}
			catch (Exception ex)
			{
				/// Scale the control to fit the bitmap that is already in it.
                /// 
                Console.WriteLine(ex.ToString());
			}

			this.Width = this.panel1.BackgroundImage.Width;
			this.Height = this.panel1.BackgroundImage.Height;
		}

		private void Container_Bitmap_Load(object sender, EventArgs e)
		{

		}

		internal override string ObjectName
		{
			set { this.panel1.Text = value; }
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.panel1;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			// Do nothing.
		}

		internal override string ObjectValue { get { return "*** NO VALUE ***"; } set { } }
	}
}

