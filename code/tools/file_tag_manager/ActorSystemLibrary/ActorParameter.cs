using System;
using System.Windows.Forms;
//using System.Data;
using System.Xml;
namespace ActorSystem
{
	/// <summary>
	/// Summary description for ActorParameter.
	/// </summary>
	/// 
	enum ActorType
	{
		COMBOBOX = 0,
		INT,
		FLOAT,
		BOOL,
		ENUM,
		STRING,
		COLOUR,
		SLIDER
	};

	public interface IActorParameter
	{
		string ActorName{get;set;}
		string ActorValue{get;set;}
	}

	// This class sohuld be abstract but then the form designer has problems instantiating classes derived from it.
	public class ActorParameter : System.Windows.Forms.UserControl, IActorParameter
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
//		private System.ComponentModel.Container components;

		private Control m_ownerControl;

		public ActorParameter()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			// 
			// ActorParameter
			// 
			this.Name = "ActorParameter";
			this.Size = new System.Drawing.Size(552, 104);

		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
//			if( disposing )
//			{
//				if(components != null)
//				{
//					components.Dispose();
//				}
//			}
			base.Dispose( disposing );
		}

		public ActorParameter(Control ownerControl, XmlNode valueNode)
		{
			InitializeComponent();
			// 
			// ActorParameter
			// 
			m_ownerControl = ownerControl;
			m_valueNode = valueNode;
		}

		XmlNode m_valueNode;

		public XmlNode ValueNode{get{return m_valueNode;}set{m_valueNode = value;}}
		public Control OwnerForm{get{return m_ownerControl;}set{m_ownerControl = value;}}


		public event EventHandler ParameterChanged;

		private bool m_useParameterChangedEventHandler = true;

		protected void CallParameterChanged(object sender)
		{
			if (ParameterChanged != null && m_useParameterChangedEventHandler == true)
//				ParameterChanged(this, new EventArgs());
				ParameterChanged(sender, new EventArgs());
		}
		#region IActorParameter Members

		// Cannot make these abstract without problems in the form designer.
		public string ActorName
		{
			get
			{
				return _ActorName;
			}
			set
			{
				_ActorName = value;
			}
		}

		public string ActorValue
		{
			get
			{
				return _ActorValue;
			}
			set
			{
				m_useParameterChangedEventHandler = false;
				_ActorValue = value;
				m_useParameterChangedEventHandler = true;
			}
		}

		virtual public string _ActorName
		{
			get
			{
				throw new Exception("Don't call this");
			}
			set
			{
				throw new Exception("Don't call this");
			}
		}

		virtual public string _ActorValue
		{
			get
			{
				throw new Exception("Don't call this");
			}
			set
			{
				throw new Exception("Don't call this");
			}
		}


		#endregion
	}
}
