// ============================================================================
//
// This file contains routines that are handled during the Runtime.
//
// Including creating, display, and handling your object.
// 
// ============================================================================

// Common Include
#include	"common.h"

// DEBUGGER /////////////////////////////////////////////////////////////////

#if !defined(RUN_ONLY)
// Identifiers of items displayed in the debugger
enum
{
// Example
// -------
//	DB_CURRENTSTRING,
//	DB_CURRENTVALUE,
//	DB_CURRENTCHECK,
//	DB_CURRENTCOMBO
};

// Items displayed in the debugger
WORD DebugTree[]=
{
// Example
// -------
//	DB_CURRENTSTRING|DB_EDITABLE,
//	DB_CURRENTVALUE|DB_EDITABLE,
//	DB_CURRENTCHECK,
//	DB_CURRENTCOMBO,

	// End of table (required)
	DB_END
};

#endif // !defined(RUN_ONLY)


// --------------------
// GetRunObjectDataSize
// --------------------
// Returns the size of the runtime datazone of the object
// 
ushort WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr)
{
	return(sizeof(RUNDATA));
}


// ----------------
// OnLostDevice : called when the device is lost
// ----------------
void CALLBACK OnLostDevice(cSurface* pSf, LPARAM lParam)
{
	if ( lParam != 0 )
	{
		LPRDATA rdPtr = (LPRDATA)lParam;
		if ( rdPtr != NULL )
		{
			// Delete HWA surfaces
			if ( rdPtr->TempImage != NULL )
			{
				delete rdPtr->TempImage;
				rdPtr->TempImage = NULL;
			}
		}
	}
}

// ---------------
// CreateRunObject
// ---------------
// The routine where the object is actually created
// 
short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr)
{
	fprh rhPtr = rdPtr->rHo.hoAdRunHeader;
	LPSURFACE ps = WinGetSurface((int)rhPtr->rhIdEditWin);

	rdPtr->ZoomValue =		edPtr->DefaultZoom;
	rdPtr->Offset =			edPtr->DefaultOffset;
	rdPtr->Effect =			edPtr->Effect;
	rdPtr->Direction =		edPtr->Direction;
	rdPtr->PerspectiveDir =	edPtr->PerspectiveDir;
	rdPtr->SineWaveWaves =	edPtr->SineWaveWaves;

	rdPtr->roc.rcChanged = true;

	rdPtr->rHo.hoImgWidth = edPtr->swidth;
	rdPtr->rHo.hoImgHeight = edPtr->sheight;

	rdPtr->resample = edPtr->resample;

	int size = (rdPtr->Direction == HORIZONTAL) ? rdPtr->rHo.hoImgWidth : rdPtr->rHo.hoImgHeight;

	rdPtr->CustomArray = new int[size];
	for(int i=0;i<size;i++)
		rdPtr->CustomArray[i] = rdPtr->ZoomValue;

	BOOL bHWA = ((rhPtr->rh4.rh4Mv->mvAppMode & SM_D3D) != 0);
	if ( bHWA )
	{
		mvNeebBackgroundAccess(rhPtr->rh4.rh4Mv, rhPtr->rhFrame, TRUE);
		if ( ps != NULL )
			ps->AddLostDeviceCallBack(OnLostDevice, (LPARAM)rdPtr);
	}

	return 0;
}


// ----------------
// DestroyRunObject
// ----------------
// Destroys the run-time object
// 
short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast)
{
	// HWA: remove lost device callback function
	fprh rhPtr = rdPtr->rHo.hoAdRunHeader;
	BOOL bHWA = ((rhPtr->rh4.rh4Mv->mvAppMode & SM_D3D) != 0);
	if ( bHWA )
	{
		cSurface* psf = WinGetSurface((int)rdPtr->rHo.hoAdRunHeader->rhIdEditWin);
		if ( psf != NULL )
			psf->RemoveLostDeviceCallBack(OnLostDevice, (LPARAM)rdPtr);
	}

	if ( rdPtr->TempImage != NULL )
	{
		delete rdPtr->TempImage;
		rdPtr->TempImage = NULL;
	}

	delete[] rdPtr->CustomArray;
	return 0;
}


// ----------------
// HandleRunObject
// ----------------
// Called (if you want) each loop, this routine makes the object live
// 
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
   if (rdPtr->roc.rcChanged)
      return REFLAG_DISPLAY;
   else
      return 0;

	return 0;
}

// ----------------
// DisplayRunObject
// ----------------
// Draw the object in the application screen.
// 
short WINAPI DLLExport DisplayRunObject(LPRDATA rdPtr)
{
	fprh rhPtr = rdPtr->rHo.hoAdRunHeader;
	LPSURFACE psw = WinGetSurface((int)rhPtr->rhIdEditWin);
	LPSURFACE ps = psw;
	BOOL bHWA = ((rhPtr->rh4.rh4Mv->mvAppMode & SM_D3D) != 0);

	// Create temp surface
	if ( rdPtr->TempImage == NULL )
	{
		LPSURFACE pProto = ps;
		if ( bHWA )
			GetSurfacePrototype(&pProto, ps->GetDepth(), ST_HWA_RTTEXTURE, ps->GetDriver());

		rdPtr->TempImage = new cSurface;
		rdPtr->TempImage->Create(rdPtr->rHo.hoImgWidth, rdPtr->rHo.hoImgHeight, pProto );
		
		if( !rdPtr->TempImage->IsValid() )
		{
			// Create a DIB surface if it couldn't create the same surface type as the PS surface.
			if ( GetSurfacePrototype(&pProto, ps->GetDepth(), ST_MEMORY, SD_DIB) )
				rdPtr->TempImage->Create(rdPtr->rHo.hoImgWidth, rdPtr->rHo.hoImgHeight,  pProto);
		}
	}

	int			ImageWidth		= rdPtr->rHo.hoImgWidth;
	int			ImageHeight		= rdPtr->rHo.hoImgHeight;
	int			X				= rdPtr->rHo.hoRect.left;
	int			Y				= rdPtr->rHo.hoRect.top;
	int			Effect			= rdPtr->Effect;
	int			Zoom			= rdPtr->ZoomValue;
	int			Offset			= rdPtr->Offset;
	DWORD		drawMode		= (rdPtr->resample) ? STRF_RESAMPLE : 0;
	DWORD		blitFlags		= 0;
	BlitMode	blitMethod		= BMODE_OPAQUE;
	float		WaveIncrement	= (rdPtr->SineWaveWaves * 360)/(float)ImageHeight;

	if(Effect==SINEOFFSET || Effect == CUSTOMOFFSET){
		if ( !bHWA )
		{
			blitMethod = BMODE_TRANSP;
			rdPtr->TempImage->Fill(0,0,0);
		}
	}

	// HWA : get current render target (to use as source) and begin rendering (= set render target to temp surface)
	if ( bHWA )
	{
		ps = psw->GetRenderTargetSurface();
		rdPtr->TempImage->BeginRendering(TRUE, 0);
		//drawMode |= STRF_COPYALPHA;
		//blitFlags |= BLTF_COPYALPHA;
	}

	// Note for Andos : a switch / case is usually more optimized than series of if

	//-------------------
	// PANORAMA EFFECT
	//-------------------

	if(Effect == PANORAMA && rdPtr->Direction == HORIZONTAL)
	for(int i = 0; i <= ImageWidth; i++)
	{
		float CurrentSinStep = (float)((float)(i-ImageWidth/2)/(ImageWidth/3.1415) + 3.1415/2);
		int CurrentHeight = (int)max(1,ImageHeight + sin( CurrentSinStep ) * Zoom - Zoom);

		ps->Stretch( *rdPtr->TempImage,	i,	0,	1,	ImageHeight,	X+i,	Y+ImageHeight/2-CurrentHeight/2,	1,	CurrentHeight,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}

	if(Effect == PANORAMA && rdPtr->Direction == VERTICAL)
	for(int i = 0; i <= ImageHeight; i++)
	{
		float CurrentSinStep = (float)((float)(i-ImageHeight/2)/(ImageHeight/3.1415) + 3.1415/2);
		int CurrentWidth = (int)max(1,ImageWidth + sin( CurrentSinStep ) * Zoom - Zoom);

		ps->Stretch( *rdPtr->TempImage,	0,	i,	ImageWidth,	1,	X+ImageWidth/2-CurrentWidth/2,	Y+i,	CurrentWidth,	1,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}

	//-------------------
	// PERSPECTIVE EFFECT
	//-------------------

	//LEFT -> RIGHT
	if(Effect == PERSPECTIVE && rdPtr->Direction == HORIZONTAL && rdPtr->PerspectiveDir == LEFTRIGHTTOPBOTTOM)
	for(int i = 0; i <= ImageWidth; i++)
	{
		int CurrentZoom = (i*Zoom)/ImageWidth;
		float CurrentFactor = (float)((ImageHeight + CurrentZoom)/( ImageHeight+0.0));
		int FinalHeight = (int)(ImageHeight / CurrentFactor + 0.5);
		ps->Stretch( *rdPtr->TempImage,	i,	0,	1,	ImageHeight,	X+i,	Y + ImageHeight/2 - FinalHeight/2,	1,	FinalHeight,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}

	//TOP -> BOTTOM
	if(Effect == PERSPECTIVE && rdPtr->Direction == VERTICAL && rdPtr->PerspectiveDir == LEFTRIGHTTOPBOTTOM)
	for(int i = 0; i <= ImageHeight; i++)
	{
		int CurrentZoom = (i*Zoom)/ImageHeight;
		float CurrentFactor = (float)((ImageWidth + CurrentZoom)/( ImageWidth+0.0));
		int FinalWidth = (int)(ImageWidth / CurrentFactor + 0.5);
		ps->Stretch( *rdPtr->TempImage,	0, i,	ImageWidth,	1,	X+ImageWidth/2-FinalWidth/2, Y+i,	FinalWidth, 1,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}


	//LEFT <- RIGHT
	if(Effect == PERSPECTIVE && rdPtr->Direction == HORIZONTAL && rdPtr->PerspectiveDir == RIGHTLEFTBOTTOMTOP)
	for(int i = 0; i <= ImageWidth; i++)
	{
		int CurrentZoom = ((ImageWidth-i-1)*Zoom)/ImageWidth;
		float CurrentFactor = (float)((ImageHeight + CurrentZoom)/( ImageHeight+0.0));
		int FinalHeight = (int)(ImageHeight / CurrentFactor + 0.5);
		ps->Stretch( *rdPtr->TempImage,	i,	0,	1,	ImageHeight,	X+i,	Y + ImageHeight/2 - FinalHeight/2,	1,	FinalHeight,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}

	//TOP <- BOTTOM
	if(Effect == PERSPECTIVE && rdPtr->Direction == VERTICAL && rdPtr->PerspectiveDir == RIGHTLEFTBOTTOMTOP)
	for(int i = 0; i <= ImageHeight; i++)
	{
		int CurrentZoom = ((ImageHeight-i-1)*Zoom)/ImageHeight;
		float CurrentFactor = (float)((ImageWidth + CurrentZoom)/( ImageWidth+0.0));
		int FinalWidth = (int)(ImageWidth / CurrentFactor + 0.5);
		ps->Stretch( *rdPtr->TempImage,	0, i,	ImageWidth,	1,	X+ImageWidth/2-FinalWidth/2, Y+i,	FinalWidth, 1,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}


	//-------------------
	// SINE WAVE EFFECT
	//-------------------

	float delta = (float)(3.141592/180.0);

	if(Effect == SINEWAVE && rdPtr->Direction == HORIZONTAL)
	for(int i = 0; i <= ImageWidth; i++){
		int FinalHeight = max( 1,(int)(ImageHeight + sin((i*WaveIncrement + rdPtr->Offset)*delta)*rdPtr->ZoomValue - rdPtr->ZoomValue));
		ps->Stretch( *rdPtr->TempImage,	i,	0,	1,	ImageHeight,	X+i,	Y + ImageHeight/2 - FinalHeight/2,	1,	FinalHeight,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}

	if(Effect == SINEWAVE && rdPtr->Direction == VERTICAL)
	for(int i = 0; i <= ImageHeight; i++){
		int FinalWidth = max( 1,(int)(ImageWidth + sin((i*WaveIncrement + rdPtr->Offset)*delta)*rdPtr->ZoomValue - rdPtr->ZoomValue));
		ps->Stretch( *rdPtr->TempImage,	0,	i,	ImageWidth,	1,	X+ImageWidth/2 - FinalWidth/2, Y+i,	FinalWidth , 1,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}


	//-------------------
	// SINE OFFSET EFFECT
	//-------------------

	if(Effect == SINEOFFSET && rdPtr->Direction == HORIZONTAL)
	for(int i = 0; i <= ImageHeight; i++){
		ps->Blit( *rdPtr->TempImage, (int)(sin((i*WaveIncrement+Offset)*delta)*Zoom) , i, X, Y+i,ImageWidth,1,BMODE_OPAQUE,BOP_COPY,0,blitFlags);
	}

	if(Effect == SINEOFFSET && rdPtr->Direction == VERTICAL)
	for(int i = 0; i <= ImageWidth; i++){
		ps->Blit( *rdPtr->TempImage, i , (int)(sin((i*WaveIncrement+Offset)*delta)*Zoom), X+i, Y,1,ImageHeight,BMODE_OPAQUE,BOP_COPY,0,blitFlags);
	}


	//-------------------
	// CUSTOM
	//-------------------
	if(Effect == CUSTOM && rdPtr->Direction == HORIZONTAL)
	for(int i = 0; i <= ImageWidth; i++)
	{
		int FinalHeight = (int)(ImageHeight / ((rdPtr->CustomArray[i] * (Zoom/100.0) +100) / 100.0));
		ps->Stretch( *rdPtr->TempImage,	i,	0,	1,	ImageHeight,	X+i,	Y + ImageHeight/2 - FinalHeight/2+Offset,	1,	FinalHeight,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}

	if(Effect == CUSTOM && rdPtr->Direction == VERTICAL)
	for(int i = 0; i <= ImageHeight; i++)
	{
		int FinalWidth = (int)(ImageWidth / ((rdPtr->CustomArray[i] * (Zoom/100.0) +100) / 100.0));
		ps->Stretch( *rdPtr->TempImage,	0,	i,	ImageWidth,	1,	X+ ImageWidth/2 - FinalWidth/2+Offset,	Y+i,	FinalWidth,	1,BMODE_OPAQUE,BOP_COPY,0,drawMode);
	}

	//-------------------
	// CUSTOM OFFSET
	//-------------------
	if(Effect == CUSTOMOFFSET && rdPtr->Direction == HORIZONTAL)
	for(int i = 0; i <= ImageWidth; i++){
		ps->Blit(*rdPtr->TempImage, i, (int)(rdPtr->CustomArray[i]*(Zoom/100.0))+Offset, X+i,Y,1,ImageHeight,BMODE_OPAQUE,BOP_COPY,0,blitFlags);
	}

	if(Effect == CUSTOMOFFSET && rdPtr->Direction == VERTICAL)
	for(int i = 0; i <= ImageHeight; i++){
		ps->Blit(*rdPtr->TempImage, (int)(rdPtr->CustomArray[i]*(Zoom/100.0))+Offset, i,  X,Y+i,ImageWidth,1,BMODE_OPAQUE,BOP_COPY,0,blitFlags);
	}

	if ( bHWA )
	{
		rdPtr->TempImage->EndRendering();
		psw->ReleaseRenderTargetSurface(ps);
	}

	//-------------------
	// BLIT
	//-------------------

	rdPtr->TempImage->Blit(
		*psw,
		rdPtr->rHo.hoRect.left,
		rdPtr->rHo.hoRect.top,
		0,0,
		rdPtr->rHo.hoImgWidth,
		rdPtr->rHo.hoImgHeight,
		blitMethod,
		BOP_COPY,
		0
	);

	WinAddZone(rhPtr->rhIdEditWin, &rdPtr->rHo.hoRect);

	return 0;
}

// -------------------
// GetRunObjectSurface
// -------------------
// Implement this function instead of DisplayRunObject if your extension
// supports ink effects and transitions. Note: you can support ink effects
// in DisplayRunObject too, but this is automatically done if you implement
// GetRunObjectSurface (MMF applies the ink effect to the transition).
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
/*
cSurface* WINAPI DLLExport GetRunObjectSurface(LPRDATA rdPtr)
{
	return NULL;
}
*/

// -------------------------
// GetRunObjectCollisionMask
// -------------------------
// Implement this function if your extension supports fine collision mode (OEPREFS_FINECOLLISIONS),
// Or if it's a background object and you want Obstacle properties for this object.
//
// Should return NULL if the object is not transparent.
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
//
/*
cSurface* WINAPI DLLExport GetRunObjectCollisionMask(LPRDATA rdPtr, LPARAM lParam)
{
	// Typical example for active objects
	// ----------------------------------
	// Opaque? collide with box
	if ( (rdPtr->rs.rsEffect & EFFECTFLAG_TRANSPARENT) == 0 )	// Note: only if your object has the OEPREFS_INKEFFECTS option
		return NULL;

	// Transparent? Create mask
	LPSMASK pMask = rdPtr->m_pColMask;
	if ( pMask == NULL )
	{
		if ( rdPtr->m_pSurface != NULL )
		{
			DWORD dwMaskSize = rdPtr->m_pSurface->CreateMask(NULL, lParam);
			if ( dwMaskSize != 0 )
			{
				pMask = (LPSMASK)calloc(dwMaskSize, 1);
				if ( pMask != NULL )
				{
					rdPtr->m_pSurface->CreateMask(pMask, lParam);
					rdPtr->m_pColMask = pMask;
				}
			}
		}
	}

	// Note: for active objects, lParam is always the same.
	// For background objects (OEFLAG_BACKGROUND), lParam maybe be different if the user uses your object
	// as obstacle and as platform. In this case, you should store 2 collision masks
	// in your data: one if lParam is 0 and another one if lParam is different from 0.

	return pMask;
}
*/

// ----------------
// PauseRunObject
// ----------------
// Enters the pause mode
// 
short WINAPI DLLExport PauseRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// -----------------
// ContinueRunObject
// -----------------
// Quits the pause mode
//
short WINAPI DLLExport ContinueRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// ============================================================================
//
// START APP / END APP / START FRAME / END FRAME routines
// 
// ============================================================================

// -------------------
// StartApp
// -------------------
// Called when the application starts or restarts.
// Useful for storing global data
// 
void WINAPI DLLExport StartApp(mv _far *mV, CRunApp* pApp)
{
	OutputDebugString("Start app\n");

	// Example
	// -------
	// Delete global data (if restarts application)
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->mvSetExtUserData(pApp, hInstLib, NULL);
//	}
}

// -------------------
// EndApp
// -------------------
// Called when the application ends.
// 
void WINAPI DLLExport EndApp(mv _far *mV, CRunApp* pApp)
{
	OutputDebugString("End app\n");

	// Example
	// -------
	// Delete global data
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->mvSetExtUserData(pApp, hInstLib, NULL);
//	}
}

// -------------------
// StartFrame
// -------------------
// Called when the frame starts or restarts.
// 
void WINAPI DLLExport StartFrame(mv _far *mV, DWORD dwReserved, int nFrameIndex)
{
	char tutu[100];
	wsprintf(tutu, "Start Frame %d\n", nFrameIndex);

	OutputDebugString(tutu);
}

// -------------------
// EndFrame
// -------------------
// Called when the frame ends.
// 
void WINAPI DLLExport EndFrame(mv _far *mV, DWORD dwReserved, int nFrameIndex)
{
	char tutu[100];
	wsprintf(tutu, "End Frame %d\n", nFrameIndex);

	OutputDebugString(tutu);
}

// ============================================================================
//
// TEXT ROUTINES (if OEFLAG_TEXT)
// 
// ============================================================================

// -------------------
// GetRunObjectFont
// -------------------
// Return the font used by the object.
// 
/*

  // Note: do not forget to enable the functions in the .def file 
  // if you remove the comments below.

void WINAPI GetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf)
{
	// Example
	// -------
	// GetObject(rdPtr->m_hFont, sizeof(LOGFONT), pLf);
}

// -------------------
// SetRunObjectFont
// -------------------
// Change the font used by the object.
// 
void WINAPI SetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf, RECT* pRc)
{
	// Example
	// -------
//	HFONT hFont = CreateFontIndirect(pLf);
//	if ( hFont != NULL )
//	{
//		if (rdPtr->m_hFont!=0)
//			DeleteObject(rdPtr->m_hFont);
//		rdPtr->m_hFont = hFont;
//		SendMessage(rdPtr->m_hWnd, WM_SETFONT, (WPARAM)rdPtr->m_hFont, FALSE);
//	}

}

// ---------------------
// GetRunObjectTextColor
// ---------------------
// Return the text color of the object.
// 
COLORREF WINAPI GetRunObjectTextColor(LPRDATA rdPtr)
{
	// Example
	// -------
	return 0;	// rdPtr->m_dwColor;
}

// ---------------------
// SetRunObjectTextColor
// ---------------------
// Change the text color of the object.
// 
void WINAPI SetRunObjectTextColor(LPRDATA rdPtr, COLORREF rgb)
{
	// Example
	// -------
	rdPtr->m_dwColor = rgb;
	InvalidateRect(rdPtr->m_hWnd, NULL, TRUE);
}
*/


// ============================================================================
//
// DEBUGGER ROUTINES
// 
// ============================================================================

// -----------------
// GetDebugTree
// -----------------
// This routine returns the address of the debugger tree
//
LPWORD WINAPI DLLExport GetDebugTree(LPRDATA rdPtr)
{
#if !defined(RUN_ONLY)
	return DebugTree;
#else
	return NULL;
#endif // !defined(RUN_ONLY)
}

// -----------------
// GetDebugItem
// -----------------
// This routine returns the text of a given item.
//
void WINAPI DLLExport GetDebugItem(LPSTR pBuffer, LPRDATA rdPtr, int id)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
/*
	char temp[DB_BUFFERSIZE];

	switch (id)
	{
	case DB_CURRENTSTRING:
		LoadString(hInstLib, IDS_CURRENTSTRING, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->text);
		break;
	case DB_CURRENTVALUE:
		LoadString(hInstLib, IDS_CURRENTVALUE, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->value);
		break;
	case DB_CURRENTCHECK:
		LoadString(hInstLib, IDS_CURRENTCHECK, temp, DB_BUFFERSIZE);
		if (rdPtr->check)
			wsprintf(pBuffer, temp, "TRUE");
		else
			wsprintf(pBuffer, temp, "FALSE");
		break;
	case DB_CURRENTCOMBO:
		LoadString(hInstLib, IDS_CURRENTCOMBO, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->combo);
		break;
	}
*/

#endif // !defined(RUN_ONLY)
}

// -----------------
// EditDebugItem
// -----------------
// This routine allows to edit editable items.
//
void WINAPI DLLExport EditDebugItem(LPRDATA rdPtr, int id)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
/*
	switch (id)
	{
	case DB_CURRENTSTRING:
		{
			EditDebugInfo dbi;
			char buffer[256];

			dbi.pText=buffer;
			dbi.lText=TEXT_MAX;
			dbi.pTitle=NULL;

			strcpy(buffer, rdPtr->text);
			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITTEXT, 0, (LPARAM)&dbi);
			if (ret)
				strcpy(rdPtr->text, dbi.pText);
		}
		break;
	case DB_CURRENTVALUE:
		{
			EditDebugInfo dbi;

			dbi.value=rdPtr->value;
			dbi.pTitle=NULL;

			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITINT, 0, (LPARAM)&dbi);
			if (ret)
				rdPtr->value=dbi.value;
		}
		break;
	}
*/
#endif // !defined(RUN_ONLY)
}


