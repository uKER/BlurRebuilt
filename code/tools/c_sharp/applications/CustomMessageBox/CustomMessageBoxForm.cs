using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace CustomMessageBox
{
	public partial class CustomMessageBoxForm : Form
	{
		readonly string m_strText;
		readonly string m_strCaption;
		readonly Icon m_icon;

		//Point m_currentPoint = new Point(10, 0);

		object m_result;

		public object Result
		{
			get { return m_result; }
		}

		public CustomMessageBoxForm(string strText, string strCaption, Icon icon)
		{
			InitializeComponent();

			m_strText = strText;
			m_strCaption = strCaption;
			m_icon = icon;
		}

		List<Button> m_buttons = new List<Button>();

		public bool AddButton(string strCaption, object retVal)
		{
			Button b = new Button();
			b.Text = strCaption;
			b.Tag = retVal;
			b.Click += new EventHandler(b_Click);
			b.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
			m_buttons.Add(b);
			return true;
		}

		void b_Click(object sender, EventArgs e)
		{
			Button b = sender as Button;

			this.m_result = b.Tag;

			this.DialogResult = DialogResult.OK;
		}



		private void CustomMessageBoxForm_Load(object sender, EventArgs e)
		{
			int buttonsWidth = 0;
			this.richTextBox.Text = m_strText;
			this.Text = m_strCaption;

			int index = 1;

			foreach(Button b in m_buttons)
			{
				b.Location = new Point(this.ClientSize.Width - (index * (b.Width + 5)) , this.ClientSize.Height - b.Height - 10);
				buttonsWidth += b.Width + 5;
				index++;
				this.Controls.Add(b);
			}

			/// Get the size that the text box needs to be to show the text without resorting to scroll bars.
			/// Use this to set the size of the form, also considering the visibility of the buttons.
			Graphics g = this.richTextBox.CreateGraphics();
			Font font = this.richTextBox.Font;
			SizeF size = g.MeasureString(m_strText, font);

			this.richTextBox.Width = (int)((float)size.Width * g.PageScale) + 10;
			this.richTextBox.Height = (int)((float)size.Height * g.PageScale) + 10;

			int width = Math.Max(richTextBox.Left + richTextBox.Right, buttonsWidth);
			this.ClientSize = new Size(width, richTextBox.Bottom + richTextBox.Top + 40);
			g.Dispose();
		}
	}
}
