using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Global;

namespace memory_variable
{
	public static class MemoryVariable
	{
		static int MIN_VERSION = 1;
		static private XmlDocument m_xml = null;

		/// <summary>
		/// Create the memory document and add the root element.
		/// </summary>
		/// <returns>The newly created document</returns>
		private static XmlDocument CreateXmlDocument()
		{
			XmlDocument xmlDocument = new XmlDocument();
			xmlDocument.LoadXml("<memory_variables version = '1' />");
			return xmlDocument;
		}

		/// <summary>
		/// Returns the root entry of the memory document so it can be stored by the controlling application.
		/// If the memory document does not exist then one is created.
		/// </summary>
		/// <returns>Returns the root entry of the memory document so it can be stored by the controlling application.</returns>
		public static XmlElement GetMemoryRoot()
		{
			try
			{
				return m_xml["memory_variables"];
			}
			catch
			{
				m_xml = CreateXmlDocument();
				return m_xml["memory_variables"];
			}
		}

		/// <summary>
		/// Imports the settings for a memory document so states can be restored by the controlling application.
		/// </summary>
		/// <param name="xElement">An Xml element representing a copy of the root memory element</param>
		/// <returns>true</returns>
		public static bool SetMemoryRoot(XmlElement xElement)
		{
			if(xElement != null)
			{
				if(xElement.Name == "memory_variables" && XmlHelper.GetKeyInt(xElement, "version", 0) >= MIN_VERSION)
				{
					m_xml = new XmlDocument();
					XmlElement xMemory = m_xml.ImportNode(xElement, true) as XmlElement;
					m_xml.AppendChild(xMemory);
					return true;
				}
			}
			return false;
		}

		/// <summary>
		/// Returns a new string that has any non-alphanumeric characters replaced with an underscore.
		/// </summary>
		/// <param name="strInput">The string that may have non-alphanumeric characters.</param>
		/// <returns>A new string consisting of only alphanumberic characters or underscores.</returns>
		private static string MungeString(string strInput)
		{
			char[] cha = strInput.ToCharArray();

			for(int c = 0; c < cha.Length; c++)
			{
				if(char.IsLetterOrDigit(cha[c]) == false)
				{
					cha[c] = '_';
				}
			}
			return new string(cha);
		}

		/// <summary>
		/// Stores a value against a key.
		/// </summary>
		/// <param name="strKey">This is the key.</param>
		/// <param name="strNewValue">This is the value.</param>
		/// <returns>Success or fail.</returns>
		public static bool SetMemory(string strKey, string strNewValue)
		{
			try
			{
				strKey = MungeString(strKey);

				XmlElement xMemoryRoot = GetMemoryRoot();

				XmlElement xElement = XmlHelper.FindChildNode(xMemoryRoot, "memory", "name", strKey);

				if(xElement == null)
				{
					xElement = xMemoryRoot.AppendChild(m_xml.ImportNode(m_xml.CreateElement("memory"), false)) as XmlElement;
					XmlHelper.AddAttribute(xElement, "name", strKey);
				}

				XmlHelper.AddAttribute(xElement, "value", strNewValue);
				return true;
			}
			catch
			{
			}
			return false;
		}

		/// <summary>
		/// This gets the value associated with a key.
		/// </summary>
		/// <param name="strKey">The key. :-)</param>
		/// <returns></returns>
		public static string GetMemory(string strKey)
		{
			XmlElement xMemory = GetMemoryRoot();

			strKey = MungeString(strKey);
			string strValue = null;

			XmlElement xMemoryRoot = GetMemoryRoot();

			XmlElement xElement = XmlHelper.FindChildNode(xMemoryRoot, "memory", "name", strKey);

			if(xElement != null)
			{
				strValue = XmlHelper.GetKeyValue(xElement, "value");	// May or may not exist.
			}
			return strValue;
		}
	}
}
