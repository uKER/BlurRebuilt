#ifndef EDITOR_IVIEW_H
#define EDITOR_IVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class I_View
	{
	public:
		virtual ~I_View() {}

		virtual void Refresh() = 0;
	};
}

#endif