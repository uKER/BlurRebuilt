
#ifndef DIALOGHANDLER_H
#define DIALOGHANDLER_H

#define kADMNonModalDialogID	32000

// private functions
AIErr				createADMDialog( AINotifierMessage *message );
ASErr ASAPI			DlgInit( ADMDialogRef dlg );
static void ASAPI	DlgDestroyProc( ADMDialogRef dlg );

#endif // DIALOGHANDLER_H



