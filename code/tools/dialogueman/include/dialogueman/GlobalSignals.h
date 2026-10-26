#ifndef DMAN_GLOBAL_SIGNALS_H
#define DMAN_GLOBAL_SIGNALS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	struct C_GlobalSignals
	{
		////// Signals from app //////
		sigslot::signal0<>							quit;

		sigslot::signal0<>							appDbOpened;
		sigslot::signal0<>							appDbClose;
		sigslot::signal0<>							appRefreshDirty;
							
		sigslot::signal0<>							appSelectionChanged;

		sigslot::signal1<bool>						viewsEnabled;

		sigslot::signal1<C_Row*>					rowDeserialised;
		
		sigslot::signal3<C_RowSet*, C_Row*, int>	rowSetRowInserted;
		sigslot::signal2<C_RowSet*, C_Row*>			rowSetRowAppended;
		sigslot::signal2<C_RowSet*, C_Row*>			rowSetRowReordered;
		sigslot::signal2<C_RowSet*, C_Row*>			rowSetRowDelete;

	private:
		//Prevent construction outside of app.
		C_GlobalSignals() {}

		FRIEND_CLASS C_App;
	};
}

#endif //DMAN_GLOBAL_SIGNALS_H
