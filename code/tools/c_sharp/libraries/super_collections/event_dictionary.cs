using System;
using System.Collections.Generic;
using System.Text;

namespace super_collections
{
	/// <summary>
	/// Describes the change that is about to occur that has caused the OnChange event to fire.
	/// </summary>
	public enum DictionaryChangeType
	{
		Add = 0,
		Clear,
		Remove,
	}

	/// <summary>
	/// This is a dictionary that fires an event when it's contents are modified.
	/// It contains an ONChange event that can be used by the caller.
	/// </summary>
	/// <typeparam name="TKey">Key type.</typeparam>
	/// <typeparam name="TValue">Value type.</typeparam>
	public class EventDictionary<TKey, TValue> : Dictionary<TKey, TValue>
	{
		// Base constructor should be called for us.

		public EventHandler<DictionaryChangedEventArgs> DictionaryChangedEvent;

		/// <summary>
		/// This event is called when the dictionary is modified.
		/// </summary>
		//public event EventDictionaryEventHandler OnChange;

		/// <summary>
		/// Adds a key-value pair after calling the OnChange event.
		/// </summary>
		/// <param name="key"></param>
		/// <param name="value"></param>
		public new void Add(TKey key, TValue value)
		{
			OnChange(DictionaryChangeType.Add, key, value);
			base.Add(key, value);
		}

		/// <summary>
		/// Clears the dictionary, after calling the OnChange event.
		/// </summary>
		public new void Clear()
		{
			OnChange(DictionaryChangeType.Clear, default(TKey), default(TValue));
			base.Clear();
		}

		/// <summary>
		/// Removes a key-value pair, after calling the OnChange event.
		/// </summary>
		/// <param name="key"></param>
		public new void Remove(TKey key)
		{
			OnChange(DictionaryChangeType.Remove, key, base[key]);
			base.Remove(key);
		}

		protected void OnChange(DictionaryChangeType ct, TKey objKey, TValue objValue)
		{
			if(DictionaryChangedEvent != null)
			{
				DictionaryChangedEvent(this, new DictionaryChangedEventArgs(ct, objKey, objValue));
			}
		}

		/// <summary>
		/// EventArgs used for the event raised when the dictionary changes.
		/// </summary>
		public class DictionaryChangedEventArgs : EventArgs
		{
			readonly DictionaryChangeType m_changeType;

			public DictionaryChangeType ChangeType
			{
				get { return m_changeType; }
			}

			readonly TKey m_tKey;

			public TKey TKey
			{
				get { return m_tKey; }
			}

			TValue m_tValue;

			public TValue TValue
			{
				get { return m_tValue; }
				set { m_tValue = value; }
			}

			/// <summary>
			/// Constructor for EventArgs used for the event raised when the dictionary changes.
			/// </summary>
			/// <param name="ct">Describes the reason for the event.</param>
			/// <param name="objKey">The key provided when the event was generated, or default(type) if there is no key.</param>
			/// <param name="objValue">The value provided when the event was generated, or default(type) if there is no value.</param>
			public DictionaryChangedEventArgs(DictionaryChangeType ct, TKey objKey, TValue objValue)
			{
				m_changeType = ct;
				m_tKey = objKey;
				m_tValue = objValue;
			}
		}
	}

}
