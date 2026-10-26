using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace application
{
	public partial class Toolbar : application.GadgetFrame
	{
		public enum State
		{
			Unknown = 0,
			Hidden,
			Expanding,
			FadingIn,
			Expanded,
			FadingOut,
			Contracting,
			
			ResizingOrMoving,
		}

		bool m_bChildHasFocus = false;

		public bool ChildHasFocus { get { return m_bChildHasFocus; } set { m_bChildHasFocus = value; } }



		State m_state;

		public State CurrentState { get { return m_state; } }


		const int VISIBLE_TIMEOUT = 100;
		const int MIN_WIDTH = 4;

		public int m_originalWidth;
		double m_currentWidth;
		double m_targetWidth;

		public int GadgetX { get { return Screen.PrimaryScreen.WorkingArea.Right - m_originalWidth; } }

		public int m_idealY;

		public Toolbar()
		{
			InitializeComponent();
		}

		public Toolbar(int thickness, BaseGadget baseGadget)
			: base(thickness, baseGadget)
		{
			InitializeComponent();

			this.SetStyle(ControlStyles.UserPaint, true);
			this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
			this.SetStyle(ControlStyles.ResizeRedraw, true);
			this.SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
		}

		private void BizarreToolbar_Load(object sender, EventArgs e)
		{
			this.m_originalWidth = this.Width;

			Screen[] screens = Screen.AllScreens;
			Screen screen = Screen.PrimaryScreen;

			this.Location = new Point(screen.WorkingArea.Right - MIN_WIDTH, screen.WorkingArea.Top);
			this.Width = MIN_WIDTH;
			this.Height = screen.WorkingArea.Height;
		}

		private void MakeVisible()
		{
			this.Visible = true;
			this.m_targetWidth = m_originalWidth;
			this.BringToFront();
		}

		double m_childFadeValue = 1.0f;

		public double ChildFadeValue { get { return m_childFadeValue; } }


		internal void DoIt()
		{
			m_idealY = 10;

			if(this.IsBeingDragged == false)
			{
				//m_stateTimerValue -= m_stateTimerDecrement;

				bool bMouseIn = false;

				if(this.Bounds.Contains(MousePosition))
					bMouseIn = true;

				switch(m_state)
				{
					case State.Unknown:
						ChangeState(State.Hidden);
						break;
					case State.Hidden:
						if(bMouseIn)
							ChangeState(State.Expanding);
						break;
					case State.Expanding:
						m_currentWidth = Helpers.Gravitate(m_currentWidth, m_targetWidth, 0.45f, 2.0f);
						this.BoundsF = new RectangleF(Screen.PrimaryScreen.WorkingArea.Right - BoundsF.Width, Screen.PrimaryScreen.WorkingArea.Top, (float)m_currentWidth, Screen.PrimaryScreen.WorkingArea.Height);
						if(m_currentWidth == m_targetWidth)
							ChangeState(State.FadingIn);
						break;
					case State.FadingIn:
						m_childFadeValue = Helpers.Gravitate(m_childFadeValue, 1.0, 0.05, 0.01);
						if(m_childFadeValue == 1.0f)
							ChangeState(State.Expanded);
						break;
					case State.Expanded:
						if(bMouseIn == false && this.Focused == false && m_bChildHasFocus == false)
							ChangeState(State.FadingOut);
						break;
					case State.FadingOut:
						m_childFadeValue = Helpers.Gravitate(m_childFadeValue, 0.0f, 0.05f, 0.01f);
						if(bMouseIn)
							ChangeState(State.FadingIn);
						else if(m_childFadeValue < 0.1f)
							ChangeState(State.Contracting);
						break;
					case State.Contracting:
						if(bMouseIn)
							ChangeState(State.Expanding);
						else
						{
							m_currentWidth = Helpers.Gravitate(m_currentWidth, m_targetWidth, 0.45f, 2.0f);
							this.BoundsF = new RectangleF(Screen.PrimaryScreen.WorkingArea.Right - BoundsF.Width, Screen.PrimaryScreen.WorkingArea.Top, (float)m_currentWidth, Screen.PrimaryScreen.WorkingArea.Height);
							if(m_currentWidth == m_targetWidth)
								ChangeState(State.Hidden);
						}
						break;
					case State.ResizingOrMoving:
						break;
					default:
						break;
				}



				//if(MousePosition.X >= BoundsF.X)
				//    this.m_targetWidth = m_originalWidth;
				//else
				//{
				//    this.m_targetWidth = MIN_WIDTH;
				//}

				if(m_gripper == Gripper.None)
				{

				}
				else
				{
					m_originalWidth = (int)this.BoundsF.Width;
					m_targetWidth = this.BoundsF.Width;
					m_currentWidth = this.BoundsF.Width;
				}
			}
			else
			{

			}

			return;
		}

		//float m_stateTimerDecrement = 0.0f;
		//float m_stateTimerValue = 0.0f;

		private void ChangeState(State state)
		{
			//m_stateTimerValue = 1.0f;

			switch(state)
			{
				case State.Unknown:
					break;
				case State.Hidden:
					break;
				case State.Expanding:
					this.m_targetWidth = m_originalWidth;
					break;
				case State.FadingIn:
					break;
				case State.Expanded:
					break;
				case State.FadingOut:
					break;
				case State.Contracting:
					this.m_targetWidth = MIN_WIDTH;
					break;
				case State.ResizingOrMoving:
					break;
				default:
					break;
			}
			m_state = state;
			this.Invalidate();
		}

		private void Toolbar_FrameFormBeginDrag(object sender, DragEventArgs e)
		{
			Rectangle clientBounds = this.GetNewClientRect(this.Bounds, 20, 20, 20, 20);
			this.panelClientArea.Bounds = clientBounds;
		}

		private void Toolbar_FrameFormEndDrag(object sender, DragEventArgs e)
		{
			Rectangle clientBounds = this.GetNewClientRect(this.Bounds, 0, 0, 0, 10);
			this.panelClientArea.Bounds = clientBounds;
		}


		protected override void OnPaint(PaintEventArgs e)
		{
			//Brush b = new System.Drawing.Drawing2D.LinearGradientBrush(;

			// TODO: Draw grab-handle differently.
			e.Graphics.FillRectangle(Brushes.Red, this.DisplayRectangle);
			
			base.OnPaint(e);
		}

		protected override void OnPaintBackground(PaintEventArgs e)
		{
			base.OnPaintBackground(e);
		}
	}
}

