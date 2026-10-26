using System;
using System.Collections.Generic;
using System.Text;

namespace SuperCollections
{
	public enum ListChangeType
	{
		Add = 0,
		AddRange,
		Clear,
		Insert,
		InsertRange,
		Remove,
		RemoveAll,
		RemoveAt,
		RemoveRange,
		Reverse,
		Sort,
		TrimExcess,

		Unsupported
	}

	public class EventList<T> : List<T>
	{
		// Base constructor should be called for us.

		public delegate void SuperListEventHandler(ListChangeType ct, object obj);

		public event SuperListEventHandler OnChange;

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void Add(T type)
		{
			OnChange(ListChangeType.Add, type);
			base.Add(type);
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void AddRange(IEnumerable<T> collection)
		{
			OnChange(ListChangeType.AddRange, collection);
			base.AddRange(collection);
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void Clear()
		{
			OnChange(ListChangeType.Clear, null);
			base.Clear();
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void Insert(int index, T type)
		{
			OnChange(ListChangeType.Insert, type);
			base.Insert(index, type);
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void InsertRange(int index, IEnumerable<T> collection)
		{
			OnChange(ListChangeType.InsertRange, collection);
			base.InsertRange(index, collection);
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void Remove(T type)
		{
			OnChange(ListChangeType.Remove, type);
			base.Remove(type);
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void RemoveAll(Predicate<T> match)
		{
			OnChange(ListChangeType.RemoveAll, match);
			base.RemoveAll(match);
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void RemoveAt(int index)
		{
			OnChange(ListChangeType.RemoveAt, index);
			base.RemoveAt(index);
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void RemoveRange(int index, int count)
		{
			OnChange(ListChangeType.RemoveRange, new object[] { index, count });
			base.RemoveRange(index, count);
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void Reverse()
		{
			OnChange(ListChangeType.Reverse, null);
			base.Reverse();
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void Sort()
		{
			OnChange(ListChangeType.Sort, null);
			base.Sort();
		}

		/// <summary>
		/// Fires the OnChange event then calls the base class.
		/// </summary>
		public new void TrimExcess()
		{
			OnChange(ListChangeType.TrimExcess, null);
			base.TrimExcess();
		}
	}
}
