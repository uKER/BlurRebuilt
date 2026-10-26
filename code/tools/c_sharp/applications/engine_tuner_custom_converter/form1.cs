using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using Global;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;

namespace EngineTunerCustomConverter
{
	public partial class Form1 : Form
	{
		private Form1()
		{
			InitializeComponent();
		}

		public static bool m_bQuit = false;

		public Form1(string[] args) : this()
		{
			foreach(string strPathFile in args)
			{
				try
				{
					DoConvert(strPathFile);
				}
				catch
				{
					MessageBox.Show("Failed to convert '" + strPathFile + "'");
				}
			}
			m_bQuit = true;
		}

		private bool DoConvert(string strPathFile)
		{
			XmlDocument xInputDoc = new XmlDocument();
			xInputDoc.Load(strPathFile);
			XmlHelper.RenameNode(xInputDoc.DocumentElement, null, "Group");

			XmlNodeList xNodeList = xInputDoc.DocumentElement.ChildNodes;

			xInputDoc.DocumentElement.RemoveChild(xInputDoc.DocumentElement["globalComInput"]);

			int index = 0;

			while(index + 3 <= xNodeList.Count)
			{
				XmlElement xLoopingSound = xNodeList[index] as XmlElement;
				XmlElement xPiecewiseLinearFunction1 = xNodeList[index + 1] as XmlElement;
				XmlElement xPiecewiseLinearFunction2 = xNodeList[index + 2] as XmlElement;

				xLoopingSound = ModifyLoopingSound(xLoopingSound);
				xPiecewiseLinearFunction1 = ModifyPiecewiseLinearFunction(xPiecewiseLinearFunction1);
				xPiecewiseLinearFunction2 = ModifyPiecewiseLinearFunction(xPiecewiseLinearFunction2);

				index += 3;
			}

			Debug.WriteLine(xInputDoc.OuterXml);
			string strNewPathFile = Path.GetDirectoryName(strPathFile) + "\\" + "Modified_" + Path.GetFileName(strPathFile);

			XmlWriterSettings settings = new XmlWriterSettings();
			settings.Indent = true;
			settings.IndentChars = "\t";
			settings.OmitXmlDeclaration = true;
//			settings.NewLineOnAttributes = true;

			StringBuilder sBuilder = new StringBuilder();
			XmlWriter xWriter = XmlWriter.Create(sBuilder, settings);
			xInputDoc.Save(xWriter);

			string strOutput = sBuilder.ToString();
			
			/// Satan lives here ...
			int regexindex = 0;
			Regex regex = new Regex(" [A-Za-z0-9_]+=\"[A-Za-z0-9_]*\"");
			Regex regexOpen = new Regex("<[A-Za-z0-9_]+");
			Regex regexClose = new Regex("</[A-Za-z0-9_]+|/>");

			Match m;

			m = regex.Match(strOutput, regexindex);

			while(m.Success == true)
			{
				string strWhitespace = "\r\n";
				// Work out how many tabs we need after the <cr>
				MatchCollection matchOpen = regexOpen.Matches(strOutput.Substring(0, m.Index));
				MatchCollection matchClose = regexClose.Matches(strOutput.Substring(0, m.Index));

				int nestLevel = matchOpen.Count - matchClose.Count;
				for(int c = 0; c < nestLevel; c++)
				{
					strWhitespace = strWhitespace + "\t";
				}
				
				// Substitute the match value with the match preceeded by strWhitespace. Should've used string.Insert but so what.
				strOutput = strOutput.Substring(0, m.Index) + strWhitespace + m.Value + strOutput.Substring(m.Index + m.Length);
				regexindex = m.Index + m.Length + strWhitespace.Length;
				m = regex.Match(strOutput, regexindex);
			}
			

			/// Satan lives here ...
			regexindex = 0;
			regex = new Regex("><");

			m = regex.Match(strOutput, regexindex);

			while(m.Success == true)
			{
				string strWhitespace = "\r\n";
				// Work out how many tabs we need after the <cr>
				MatchCollection matchOpen = regexOpen.Matches(strOutput.Substring(0, m.Index));
				MatchCollection matchClose = regexClose.Matches(strOutput.Substring(0, m.Index));

				int nestLevel = matchOpen.Count - matchClose.Count - 1;
				for(int c = 0; c < nestLevel; c++)
				{
					strWhitespace = strWhitespace + "\t";
				}

				// Substitute the match value with the match preceeded by strWhitespace. Should've used string.Insert but so what.
//				strOutput = strOutput.Substring(0, m.Index) + strWhitespace + m.Value + strOutput.Substring(m.Index + m.Length);
				strOutput = strOutput.Substring(0, m.Index) + ">" + strWhitespace + "<" + strOutput.Substring(m.Index + m.Length);
				regexindex = m.Index + m.Length + strWhitespace.Length;
				m = regex.Match(strOutput, regexindex);
			}
			StreamWriter sw = new StreamWriter(strNewPathFile);
			sw.Write(strOutput);
			sw.Flush();
			sw.Close();
			return true;
		}

		private XmlElement ModifyLoopingSound(XmlElement xLoopingSound)
		{
			string strIchannels = XmlHelper.GetKeyValue(xLoopingSound, "iChannels");		/// Might be null.

			if(strIchannels == "2")
				xLoopingSound = XmlHelper.RenameNode(xLoopingSound, null, "StereoLoopingSound") as XmlElement;
			else
				xLoopingSound = XmlHelper.RenameNode(xLoopingSound, null, "MonoLoopingSound") as XmlElement;

			XmlHelper.RenameNode(xLoopingSound.Attributes.GetNamedItem("name"), null, "Name");

			XmlHelper.RenameNode(xLoopingSound.Attributes.GetNamedItem("bank"), null, "Bank");

			XmlHelper.RenameNode(xLoopingSound.Attributes.GetNamedItem("comWaveIndex"), null, "Wave");
			XmlNode attr = xLoopingSound.Attributes.GetNamedItem("Wave");
			string strWave = attr.Value;
			strWave = strWave.Substring(strWave.IndexOf('.') + 1);
			attr.Value = strWave;

			XmlHelper.RenameNode(xLoopingSound.Attributes.GetNamedItem("comVolume"), null, "LevelCom");
			XmlHelper.RenameNode(xLoopingSound.Attributes.GetNamedItem("comPitch"), null, "FrequencyCom");

			XmlHelper.RemoveAttribute(xLoopingSound, "Group");
			XmlHelper.RemoveAttribute(xLoopingSound, "Fullname");
			XmlHelper.RemoveAttribute(xLoopingSound, "iChannels");
			XmlHelper.RemoveAttribute(xLoopingSound, "comGroupVolume");

			while(xLoopingSound.ChildNodes.Count != 0)
				xLoopingSound.RemoveChild(xLoopingSound.FirstChild);		/// Remove "Output" tag.

			return xLoopingSound;

		}

		private XmlElement ModifyPiecewiseLinearFunction(XmlElement xPlf)
		{
			XmlHelper.RenameNode(xPlf.Attributes.GetNamedItem("name"), null, "Name");
			XmlHelper.RenameNode(xPlf.Attributes.GetNamedItem("comInput"), null, "InputCom");

			XmlNodeList xNodeList = xPlf.ChildNodes;

			int index = 0;

			while(index < xNodeList.Count)
			{
				ModifyStaticPoint(xNodeList[index] as XmlElement);
				index++;
			}

			return xPlf;
		}

		private bool ModifyStaticPoint(XmlElement xStaticPoint)
		{
			xStaticPoint = XmlHelper.RenameNode(xStaticPoint, null, "DataPoint") as XmlElement;
			XmlHelper.RenameNode(xStaticPoint.Attributes.GetNamedItem("x"), null, "XVal");
			XmlHelper.RenameNode(xStaticPoint.Attributes.GetNamedItem("y"), null, "YVal");

			return true;
		}


	}
}