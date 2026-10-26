using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace application
{
	public partial class GadgetFrame : application.FrameForm
	{
		public BaseGadget m_baseGadget;

		Point m_oldMousePos = new Point();
		PointF m_mouseVelocity = new PointF(0f, 0f);

		//protected Rectangle BoundsDocked;
		////protected Rectangle BoundsUndocked;
		////protected Rectangle BoundsTarget;

		Size m_undockedSize;
		Size m_dockedSize;

		internal double m_targetOpacity = 1.0;

		public bool HasBorder = false;		// Used by gadget manager.

		public Toolbar m_dockedTo = null;

		protected GadgetFrame() : base()
		{
			InitializeComponent();
		}

		public GadgetFrame(int thickness, BaseGadget baseGadget)
			: base(thickness)
		{
			InitializeComponent();

			m_dockedSize = new Size(this.Width, this.Height);
			m_undockedSize = new Size(this.Width, this.Height);

			m_baseGadget = baseGadget;

			if(baseGadget != null)
			{
				baseGadget.Owner = this;
				baseGadget.Show();
			}
		}


		/// <summary>
		/// Move and/or size the base gadget so it remains inside the frame.
		/// </summary>
		private void GadgetFrame_Move(object sender, EventArgs e)
		{
			//if(m_gripper != Gripper.None && m_gripper != Gripper.All)
			if(this.Capture == true)
			{
				//if(m_dockedTo == null)
				//{
				//    m_undockedSize = new Size(this.Width, this.Height);
				//}
				//else
				//{
				//    m_dockedSize = new Size(this.Width, this.Height);
				//}
			}


			if(m_baseGadget != null)
			{
				int x = this.Location.X + this.ClientArea.Location.X;
				int y = this.Location.Y + this.ClientArea.Location.Y;
				int w = this.ClientArea.Width;
				int h = this.ClientArea.Height;

				m_baseGadget.Bounds = new Rectangle(x, y, w, h);
			}
		}

		/// <summary>
		/// Move and/or size the base gadget so it remains inside the frame.
		/// </summary>
		private void GadgetFrame_Resize(object sender, EventArgs e)
		{
			GadgetFrame_Move(sender, e);
		}

		public void UpdatePos()
		{
			#region Inertia.

			if(this.m_dockedTo != null)
				m_mouseVelocity = new PointF(0, 0);

			else if(this.IsBeingDragged == true)
			{
				m_mouseVelocity = new PointF(MousePosition.X - m_oldMousePos.X, MousePosition.Y - m_oldMousePos.Y);
				Debug.WriteLine("MV:" + m_mouseVelocity.ToString());
			}
			else if(m_mouseVelocity.X != 0 || m_mouseVelocity.Y != 0)
			{
				double dx = m_mouseVelocity.X;
				double dy = m_mouseVelocity.Y;

				if(Screen.PrimaryScreen.Bounds.Left > ((int)(Location.X + dx)) || (Screen.PrimaryScreen.Bounds.Right < (int)(Location.X + dx)))
					dx = -dx;
				if(Screen.PrimaryScreen.Bounds.Top > ((int)(Location.Y + dy)) || (Screen.PrimaryScreen.Bounds.Bottom < (int)(Location.Y + dy)))
					dy = -dy;

				this.BoundsF = new RectangleF(BoundsF.X + m_mouseVelocity.X, BoundsF.Y + m_mouseVelocity.Y, BoundsF.Width, BoundsF.Height);

				dx = Helpers.Gravitate(dx, 0.0, 0.01, 0.0);
				dy = Helpers.Gravitate(dy, 0.0, 0.01, 0.0);
				if(Math.Abs(dx) < 0.01 && Math.Abs(dy) < 0.01)
					dx = dy = 0.0f;
				m_mouseVelocity = new PointF((float)dx, (float)dy);
				this.Invalidate();
			}
			#endregion Inertia.
			// TODO: Don't update BoundsF in the Inertia block. Make targetX etc. into floats. Sort this function out.
			int targetX;
			int targetY;
			int targetW;
			int targetH;

			if(m_dockedTo != null)
			{
				//if(this.HasBorder == true)
					m_dockedTo.m_idealY += TopBorder;

				// TODO: Use this.SetDesktopBounds?

				targetW = this.m_dockedTo.m_originalWidth;// mememememe
				targetH = m_dockedSize.Height;

				if(this.IsBeingDragged == false)
				{
					targetX = this.m_dockedTo.GadgetX;
					targetY = m_dockedTo.m_idealY;

					if(m_baseGadget.NewOpacity != m_dockedTo.ChildFadeValue)
					{
						this.m_baseGadget.NewOpacity = m_dockedTo.ChildFadeValue;
						this.m_baseGadget.Invalidate(false);
					}
				}
				else
				{
					targetX = (int)BoundsF.X;
					targetY = (int)BoundsF.Y;
				}
				m_dockedTo.m_idealY = m_dockedTo.m_idealY + this.m_baseGadget.Height;// +2;
				//if(this.HasBorder == true)
					m_dockedTo.m_idealY += BottomBorder;
			}
			else
			{
				targetX = (int)this.BoundsF.X;
				targetY = (int)this.BoundsF.Y;
				targetW = this.m_undockedSize.Width;
				targetH = this.m_undockedSize.Height;
			}

			//if(this.IsBeingDragged == false)
			if(this.bUserResize == false)
				if(this.m_baseGadget != null && this.m_baseGadget.Visible == true)
				{
					if(targetX != BoundsF.X || targetY != BoundsF.Y || targetW != BoundsF.Width || targetH != BoundsF.Height)
					{
						Rectangle rectTo = new Rectangle(targetX, targetY, targetW, targetH);
						RectangleF newBoundsF = Helpers.GravitateBounds(this.BoundsF, rectTo, 0.45f, 3);
						BoundsF = newBoundsF;
						//this.Update();
					}
				}

			m_oldMousePos = MousePosition;
		}

		bool bUserDrag = false;
		bool bUserResize = false;

		private void GadgetFrame_FrameFormBeginDrag(object sender, DragEventArgs e)
		{
			if(e.IsMoving == true)
				bUserDrag = true;
			else if(e.IsResizing == true)
				bUserResize = true;
		}

		private void GadgetFrame_FrameFormEndDrag(object sender, DragEventArgs e)
		{
			bUserDrag = false;
			bUserResize = false;

			if(m_dockedTo == null)
			{
				m_undockedSize = new Size(this.Width, this.Height);
			}
			else
			{
				m_dockedSize = new Size(this.Width, this.Height);
			}
		}
	}
}

