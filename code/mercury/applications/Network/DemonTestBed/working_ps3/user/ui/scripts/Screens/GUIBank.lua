ScreenTabSize = SCREEN_WIDTH / 16
ScreenRowStart = 28
ScreenVTabSize = 48
ScreenEdgeSize = SCREEN_WIDTH / 13
ScreenVEdgeSize = 44

GUIBank =
{
	ScreenLeftEdge = SCREEN_LEFT + ScreenEdgeSize,
	ScreenRightEdge = SCREEN_RIGHT - ScreenEdgeSize,
	ScreenColumn1 = SCREEN_LEFT + ScreenEdgeSize + (ScreenTabSize*1),
	ScreenColumn2 = SCREEN_LEFT + ScreenEdgeSize + (ScreenTabSize*2),
	ScreenColumn3 = SCREEN_LEFT + ScreenEdgeSize + (ScreenTabSize*3),
	ScreenColumn4 = SCREEN_LEFT + ScreenEdgeSize + (ScreenTabSize*4),
	ScreenColumnR1 = SCREEN_RIGHT - ScreenEdgeSize - (ScreenTabSize*1),
	ScreenColumnR2 = SCREEN_RIGHT - ScreenEdgeSize - (ScreenTabSize*2),
	ScreenColumnR3 = SCREEN_RIGHT - ScreenEdgeSize - (ScreenTabSize*3),
	ScreenColumnR4 = SCREEN_RIGHT - ScreenEdgeSize - (ScreenTabSize*4),
	ScreenTopEdge = ScreenRowStart,
	ScreenBottomEdge = 480 - ScreenVEdgeSize,
	ScreenRow1 = ScreenRowStart+(ScreenVTabSize*0),
	ScreenRow2 = ScreenRowStart+(ScreenVTabSize*1),
	ScreenRow3 = ScreenRowStart+(ScreenVTabSize*2),
	ScreenRow4 = ScreenRowStart+(ScreenVTabSize*3),
	
	--
}

--
