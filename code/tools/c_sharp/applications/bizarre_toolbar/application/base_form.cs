using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using Windows.Message;

namespace application
{
	public /*abstract*/ partial class BaseForm : Form
	{

		#region Form events

		#region Event declarations

		//Mouse events that are global to the form.
		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse pointer is over the form and a mouse button is pressed.")]
		public event EventHandler MouseFormDown;

		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse enters the visible part of the form.")]
		public event MouseEventHandler MouseFormEnter;

		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse remains stationary inside of the form for an amount of time.")]
		public event EventHandler MouseFormHover;

		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse leaves the visible part of the form.")]
		public event EventHandler MouseFormLeave;

		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse pointer is moved over the form.")]
		public event MouseEventHandler MouseFormMove;

		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse pointer is over the form and a mouse button is released.")]
		public event MouseEventHandler MouseFormUp;



		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse enters the bounds of the form, regardless of transparency.")]
		public event MouseEventHandler MouseFormBoundsEnter;

		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse pointer is moved within the bounds of the form, regardless of transparency.")]
		public event MouseEventHandler MouseFormBoundsMove;

		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the mouse pointer is within the bounds of the form and a mouse button is released.")]
		public event MouseEventHandler MouseFormBoundsUp;

		[Category("Layout - Virtual"), Browsable(true), Description("Occurs when the virtual location or size of the form is updated.")]
		public event BoundsChangeHandler VirtualBoundsChange;


		#endregion Event declarations

		public delegate void BoundsChangeHandler(object sender, BoundsChangeEventArgs e);

		private RectangleF m_boundsF = new RectangleF();

		public class BoundsChangeEventArgs : EventArgs
		{
			[Flags]
			public enum ValueChanged
			{
				None = 0,
				Left = 1,
				Right = 2,
				Top = 4,
				Bottom = 8
			}

			ValueChanged floatValuesChanged = ValueChanged.None;
			ValueChanged integerValueChanged = ValueChanged.None;

			public ValueChanged FloatValuesChanged { get { return floatValuesChanged; } }
			public ValueChanged IntegerValueChanged { get { return integerValueChanged; } }

			RectangleF m_oldBounds;
			RectangleF m_newBounds;
			public BoundsChangeEventArgs(RectangleF oldBounds, RectangleF newBounds)
			{
				m_oldBounds = oldBounds;
				m_newBounds = newBounds;

				if(oldBounds.X != newBounds.X) floatValuesChanged |= ValueChanged.Left;
				if(oldBounds.Y != newBounds.Y) floatValuesChanged |= ValueChanged.Top;
				if(oldBounds.Width != newBounds.Width) floatValuesChanged |= ValueChanged.Right;
				if(oldBounds.Height != newBounds.Height) floatValuesChanged |= ValueChanged.Bottom;

				if((int)oldBounds.X != (int)newBounds.X) integerValueChanged |= ValueChanged.Left;
				if((int)oldBounds.Y != (int)newBounds.Y) integerValueChanged |= ValueChanged.Top;
				if((int)oldBounds.Width != (int)newBounds.Width) integerValueChanged |= ValueChanged.Right;
				if((int)oldBounds.Height != (int)newBounds.Height) integerValueChanged |= ValueChanged.Bottom;
			}

			public RectangleF OldBoundsF { get { return m_oldBounds; } }
			public RectangleF NewBoundsF { get { return m_newBounds; } }
		}



		public RectangleF BoundsF
		{
			get { return m_boundsF; }
			set
			{
				if(m_boundsF.X != value.X ||
					m_boundsF.Y != value.Y ||
					m_boundsF.Width != value.Width ||
					m_boundsF.Height != value.Height)
				{
					RectangleF oldBoundsF = m_boundsF;
					m_boundsF = value;

					OnVirtualBoundsChange(new BoundsChangeEventArgs(oldBoundsF, m_boundsF));
				}
			}
		}



		/////////////////////////////////

		#region Event triggers
		protected virtual void OnMouseFormDown(EventArgs e)
		{
			if(MouseFormDown != null)
			{
				MouseFormDown(this, e);
			}
		}

		protected virtual void OnMouseFormEnter(MouseEventArgs e)
		{
			if(MouseFormEnter != null)
			{
				MouseFormEnter(this, e);
			}
		}

		protected virtual void OnMouseFormHover(EventArgs e)
		{
			if(MouseFormHover != null)
			{
				MouseFormHover(this, e);
			}
		}

		protected virtual void OnMouseFormLeave(EventArgs e)
		{
			if(MouseFormLeave != null)
			{
				MouseFormLeave(this, e);
			}
		}

		protected virtual void OnMouseFormMove(MouseEventArgs e)
		{
			if(MouseFormMove != null)
			{
				MouseFormMove(this, e);
			}
		}

		protected virtual void OnMouseFormUp(MouseEventArgs e)
		{
			if(MouseFormUp != null)
			{
				MouseFormUp(this, e);
			}
		}

		protected virtual void OnMouseFormBoundsEnter(MouseEventArgs e)
		{
			if(MouseFormBoundsEnter != null)
			{
				MouseFormBoundsEnter(this, e);
			}
		}

		protected virtual void OnMouseFormBoundsMove(MouseEventArgs e)
		{
			if(MouseFormBoundsMove != null)
			{
				MouseFormBoundsMove(this, e);
			}
		}

		protected virtual void OnMouseFormBoundsUp(MouseEventArgs e)
		{
			if(MouseFormBoundsUp != null)
			{
				MouseFormBoundsUp(this, e);
			}
		}



		protected virtual void OnVirtualBoundsChange(BoundsChangeEventArgs e)
		{
			if(VirtualBoundsChange != null)
			{
				VirtualBoundsChange(this, e);
			}
		}





		#endregion Event triggers

		#endregion Form events

		/////////////////////////

		public BaseForm()
			: base()
		{
			InitializeComponent();
		}

		internal bool ProcessCustomMessage(ref Message m)
		{
			WM msg = (WM)m.Msg;

			switch(msg)
			{
				case WM.MouseMove:
					{
						return false;
					}
				case WM.MouseLeave:
					{
						return false;
					}
				case WM.LButtonDown:
				case WM.RButtonDown:
				case WM.MButtonDown:
					{
					}
					break;

				case WM.LButtonUp:
				case WM.RButtonUp:
				case WM.MButtonUp:
					{
					}
					break;

				default:
					break;
			}
			return false;

		}

		private void BaseForm_Load(object sender, EventArgs e)
		{
			BoundsF = new RectangleF(Bounds.X, Bounds.Y, Bounds.Width, Bounds.Height);
		}


		//public new double Opacity { /*get { return 0; } */set { base.Opacity = value; } }
		public double NewOpacity
		{
			get { return this.Opacity; }
			set
			{
				if(this.Opacity != value)
				{
					this.Opacity = value;

					if(value < 0.1f)
					{
						if(this.Visible == true)
							this.Visible = false;
					}
					else
					{
						if(this.Visible == false)
							this.Visible = true;
					}
				}
			}
		}
	}
}