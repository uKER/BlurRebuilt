using System;
using System.Xml;
using System.Xml.Serialization;
using System.Collections;
using System.Runtime.InteropServices;

[assembly: ComVisible(false)]
namespace ActorSystem
{
	/// <summary>
	/// Summary description for XmlHelper.
	/// </summary>
//	public abstract class XmlHelper
	
	public sealed class XmlHelper
	{
		#region Xml helper functions.
		////////////////////////////////////////////////////////////////////////////////
		/// XML helper functions.
		////////////////////////////////////////////////////////////////////////////////

		private XmlHelper(){}

		public static string FullNodePath(XmlNode node, string delim)
		{
			if (delim == null)
				delim = "/";
 
			if (node.ParentNode != null && !(node.ParentNode is XmlDocument))
				return FullNodePath(node.ParentNode, delim) + delim + node.Name;
			else
				return node.Name;
		}

		public static string GetKeyValue(XmlNode node, string strKey)
		{
			string strRetval = null;
			if(node != null)
			{
				XmlNode attr = node.Attributes.GetNamedItem(strKey);
				if(attr != null)
				{
					strRetval = attr.Value;
				}
			}
			return strRetval;
		}


		public static XmlNode FindNodeByKeyValue(XmlNodeList xmlList, string strName, string strValue)
		{
			// TODO: In a foreach, do we need to test for null?
			if(xmlList != null)
			{
				foreach(XmlNode n in xmlList)
				{
					if(n.Attributes.GetNamedItem(strName).Value == strValue)
						return n;
				}
			}
			return null;
		}

		public static bool AddAttribute(XmlNode node, string strKey, string strValue)
		{
			if(node != null)
			{
				XmlAttribute attr = node.OwnerDocument.CreateAttribute(strKey);
				attr.Value = strValue;
				node.Attributes.SetNamedItem(attr);
				return true;
			}
			return false;
		}

		public static XmlNode FindChildNode(XmlNode parent, string strName, string strKey, string strValue)
		{
			if(parent != null)
			{
				XmlNodeList childList = parent.SelectNodes(strName);

				if(childList != null)
				{
					foreach(XmlNode childNode in childList)
					{
						if(GetKeyValue(childNode, strKey) == strValue)
							return childNode;
					}
				}
			}
			return null;
		}



		/// <summary>
		/// Given a node list and a key, make a list of strings holding the value of every
		/// node that holds the key.
		/// </summary>
		/// <param name="nodeList">List of XML nodes</param>
		/// <param name="strKey">The key on which to search</param>
		/// <returns></returns>
		public static ArrayList GetValueList(XmlNodeList nodeList, string strKey)
		{
			ArrayList arrayResult = new ArrayList();

			foreach(XmlNode n in nodeList)
			{
				if(n.Attributes.GetNamedItem(strKey) != null)
				{
					arrayResult.Add(n.Attributes.GetNamedItem(strKey).Value);
				}
			}
			return arrayResult;
		}


		#endregion Xml helper functions.

	}
}
