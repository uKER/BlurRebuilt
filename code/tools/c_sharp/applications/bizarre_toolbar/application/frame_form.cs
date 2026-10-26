using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace application
{
	public partial class FrameForm : application.BaseBorderlessForm
	{
		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the FrameForm starts to be dragged.")]
		public event DragEventHandler FrameFormBeginDrag;
		[Category("Mouse Extended"), Browsable(true), Description("Occurs when the FrameForm stops being dragged.")]
		public event DragEventHandler FrameFormEndDrag;

		public delegate void DragEventHandler(object sender, DragEventArgs e);


		public class DragEventArgs : EventArgs
		{
			Gripper m_gripper;

			public Gripper gripType { get { return m_gripper; } }

			public DragEventArgs(Gripper gripper)
				: base()
			{
				m_gripper = gripper;
			}

			public bool IsResizing { get { return ((m_gripper != Gripper.All) && (m_gripper != Gripper.None)); } }

			public bool IsMoving { get { return m_gripper == Gripper.All; } }
		}

		#region Event triggers
		protected virtual void OnFrameFormBeginDrag(DragEventArgs e)
		{
			if(FrameFormBeginDrag != null)
			{
				FrameFormBeginDrag(this, e);
			}
		}

		protected virtual void OnFrameFormEndDrag(DragEventArgs e)
		{
			if(FrameFormEndDrag != null)
			{
				FrameFormEndDrag(this, e);
			}
		}
		#endregion Event triggers

		public enum Gripper
		{
			None = 0,
			All,
			TopLeft,
			Top,
			TopRight,
			Right,
			BottomRight,
			Bottom,
			BottomLeft,
			Left
		}

		public Rectangle ClientArea { get { return this.panelClientArea.Bounds; } set { this.panelClientArea.Bounds = value; } }

		int m_mouseRightBorderOffset;
		int m_mouseBottomBorderOffset;

		int m_mouseLeftBorderOffset;
		int m_mouseTopBorderOffset;
		const int MIN_HEIGHT = 5;
		const int MIN_WIDTH = 5;

		protected Gripper m_gripper;

		protected FrameForm()
		{
			InitializeComponent();
			m_gripper = Gripper.None;
		}

		#region Public constructors
		public FrameForm(int topThickness, int rightThickness, int bottomThickness, int leftThickness)
			: this()
		{
			Rectangle clientBounds = GetNewClientRect(this.Bounds, topThickness, rightThickness, bottomThickness, leftThickness);
			panelClientArea.Bounds = clientBounds;
		}

		public FrameForm(int thickness)
			: this()
		{
			Rectangle clientBounds = GetNewClientRect(this.Bounds, thickness, thickness, thickness, thickness);
			panelClientArea.Bounds = clientBounds;
		}

		public FrameForm(int topThickness, int thickness)
			: this()
		{
			Rectangle clientBounds = GetNewClientRect(this.Bounds, topThickness, thickness, thickness, thickness);
			panelClientArea.Bounds = clientBounds;
		}
		#endregion Public constructors

		public int TopBorder { get { return this.panelClientArea.Location.Y; } }
		public int LeftBorder { get { return this.panelClientArea.Location.X; } }
		public int RightBorder { get { return this.Width - (this.panelClientArea.Location.X + this.panelClientArea.Width); } }
		public int BottomBorder { get { return this.Height - (this.panelClientArea.Location.Y + this.panelClientArea.Height); } }



		internal Rectangle GetNewClientRect(Rectangle frameRect, int topBorder, int rightBorder, int bottomBorder, int leftBorder)
		{
			int top = topBorder;
			int left = leftBorder;
			int width = frameRect.Width - leftBorder - rightBorder;
			int height = frameRect.Height - topBorder - bottomBorder;

			return new Rectangle(left, top, width, height);
		}

		Gripper GetGripperType(int x, int y)
		{
			int x1 = this.panelClientArea.Location.X;
			int y1 = this.panelClientArea.Location.Y;
			int x2 = x1 + this.panelClientArea.Width;
			int y2 = y1 + this.panelClientArea.Height;

			if (x < x1)
			{
				if (y < y1) return Gripper.TopLeft;
				else if (y < y2) return Gripper.Left;
				else return Gripper.BottomLeft;
			}
			if (x < x2)
			{
				if (y < y1) return Gripper.Top;
				else if (y < y2) return Gripper.None;
				else return Gripper.Bottom;
			}
			else
			{
				if (y < y1) return Gripper.TopRight;
				else if (y < y2) return Gripper.Right;
				else return Gripper.BottomRight;
			}
		}

		internal bool IsBeingDragged{get{return m_gripper == Gripper.All;}}



		#region Events

		private void FrameForm_MouseDown(object sender, MouseEventArgs e)
		{
			//this.BringToFront();
			if (e.Button == MouseButtons.Left)
				m_gripper = GetGripperType(e.X, e.Y);
			else if(e.Button == MouseButtons.Right)
				m_gripper = Gripper.All;

			if (m_gripper != Gripper.None)
			{
				OnFrameFormBeginDrag(new DragEventArgs(m_gripper));
				this.Capture = true;
				m_mouseLeftBorderOffset = e.X;
				m_mouseTopBorderOffset = e.Y;

				m_mouseRightBorderOffset = this.Width - e.X;			// Relative to the form.
				m_mouseBottomBorderOffset = this.Height - e.Y;		// Relative to the form.
			}
		}

		private void FrameForm_MouseUp(object sender, MouseEventArgs e)
		{
			OnFrameFormEndDrag(new DragEventArgs(m_gripper));
			m_gripper = Gripper.None;
			this.Capture = false;
		}

		private void FrameForm_MouseMove(object sender, MouseEventArgs e)
		{
			int x, y, w, h;
			switch (m_gripper)
			{
				
				case Gripper.None:
					break;

				case Gripper.All:
					this.BoundsF = new RectangleF(MousePosition.X - m_mouseLeftBorderOffset, MousePosition.Y - m_mouseTopBorderOffset, this.BoundsF.Width, this.BoundsF.Height);
					break;

				case Gripper.TopLeft:
					x = Math.Min(MousePosition.X - m_mouseLeftBorderOffset, this.Location.X + this.Width - MIN_WIDTH);
					y = Math.Min(MousePosition.Y - m_mouseTopBorderOffset, this.Location.Y + this.Height - MIN_HEIGHT);
					w = this.Width - (x - this.Location.X);
					h = Math.Max(this.Height - (y - this.Location.Y), MIN_HEIGHT);
					this.BoundsF = new Rectangle(x, y, w, h);
					break;

				case Gripper.Top:
					x = this.Location.X;
					y = Math.Min(MousePosition.Y - m_mouseTopBorderOffset, this.Location.Y + this.Height - MIN_HEIGHT);
					w = this.Width;
					h = this.Height - (y - this.Location.Y);
					if(h > 4)
						this.BoundsF = new Rectangle(x, y, w, h);
					break;

				case Gripper.TopRight:
					x = this.Location.X;
					y = Math.Min(MousePosition.Y - m_mouseTopBorderOffset, this.Location.Y + this.Height - MIN_HEIGHT);
					w = Math.Max(MousePosition.X - this.Location.X + m_mouseRightBorderOffset, MIN_WIDTH);
					h = Math.Max(this.Height - (y - this.Location.Y), MIN_HEIGHT);
					this.BoundsF = new Rectangle(x, y, w, h);
					break;

				case Gripper.Right:
					this.BoundsF = new RectangleF(BoundsF.X, BoundsF.Y, MousePosition.X - this.Location.X + m_mouseRightBorderOffset, BoundsF.Height);
					break;

				case Gripper.BottomRight:
					this.BoundsF = new RectangleF(this.BoundsF.X, this.BoundsF.Y, MousePosition.X - this.Location.X + m_mouseRightBorderOffset, MousePosition.Y - this.Location.Y + m_mouseBottomBorderOffset);
					break;

				case Gripper.Bottom:
					this.BoundsF = new RectangleF(this.BoundsF.X, this.BoundsF.Y, this.BoundsF.Width, MousePosition.Y - this.Location.Y + m_mouseBottomBorderOffset);
					break;

				case Gripper.BottomLeft:
					x = Math.Min(MousePosition.X - m_mouseLeftBorderOffset, this.Location.X + this.Width - MIN_WIDTH);
					y = this.Location.Y;
					w = this.Width - (x - this.Location.X);
					h = MousePosition.Y - this.Location.Y + m_mouseBottomBorderOffset;
					this.BoundsF = new RectangleF(x, y, w, h);
					break;

				case Gripper.Left:
					x = MousePosition.X - m_mouseLeftBorderOffset;
					y = this.Location.Y;
					w = this.Width - (x - this.Location.X);
					h = this.Height;
					if (w > 4)
						this.BoundsF = new Rectangle(x, y, w, h);
					break;

				default:
					break;
			}
		}
		
		#endregion Events

		private void FrameForm_VirtualBoundsChange(object sender, BoundsChangeEventArgs e)
		{
			if(e.IntegerValueChanged != BoundsChangeEventArgs.ValueChanged.None)
			{
				this.Bounds = new Rectangle((int)BoundsF.X, (int)BoundsF.Y, (int)BoundsF.Width, (int)BoundsF.Height);
				//this.Update();
			}
		}

#if(false)
		private Rectangle SetTopFrameThickness(Rectangle outerBounds, Rectangle innerBounds, int thickness)
		{
			return new Rectangle(innerBounds.Location.X, thickness, innerBounds.Width, innerBounds.Height + innerBounds.Location.Y - thickness);
		}

		private Rectangle SetRightFrameThickness(Rectangle outerBounds, Rectangle innerBounds, int thickness)
		{
			return new Rectangle(innerBounds.Location.X, innerBounds.Location.Y, outerBounds.Width - innerBounds.Location.X - thickness, innerBounds.Height);
		}

		private Rectangle SetBottomFrameThickness(Rectangle outerBounds, Rectangle innerBounds, int thickness)
		{
			return new Rectangle(innerBounds.Location.X, innerBounds.Location.Y, innerBounds.Width, outerBounds.Height - innerBounds.Location.Y - thickness);
		}

		private Rectangle SetLeftFrameThickness(Rectangle outerBounds, Rectangle innerBounds, int thickness)
		{
			return new Rectangle(thickness, innerBounds.Location.Y, innerBounds.Location.X + innerBounds.Width - thickness, innerBounds.Height);
		}
#endif

	}
}

