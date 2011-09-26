// ============================================================================
//
// This file are where the Conditions/Actions/Expressions are defined.
// You can manually enter these, or use CICK (recommended)
// See the Extension FAQ in this SDK for more info and where to download it
//
// ============================================================================

// Common Include
#include	"common.h"
int resizePerspective(LPRDATA rdPtr, int oldsize);

// Quick memo: content of the eventInformations arrays
// ---------------------------------------------------
// Menu ID
// String ID
// Code
// Flags
// Number_of_parameters
// Parameter_type [Number_of_parameters]
// Parameter_TitleString [Number_of_parameters]

// Definitions of parameters for each condition
short conditionsInfos[]=
	{ 
	0 };

// Definitions of parameters for each action
short actionsInfos[]=
	{ 
	AID_actSetZoomValue,	AID_actSetZoomValue,	0,	0,	1,	PARAM_EXPRESSION,	AP0ID_actSetZoomValue,
	AID_actSetPanorama,		AID_actSetPanorama,		1,	0,	0,
	AID_actSetPerspective,	AID_actSetPerspective,	2,	0,	0,
	AID_actSetSineWave,		AID_actSetSineWave,		3,	0,	0,
	AID_actSetCustom,		AID_actSetCustom,		4,	0,	0,
	AID_actSetNumWaves,		AID_actSetNumWaves,		5,	0,	1,	PARAM_EXPRESSION,	AP0ID_actSetNumWaves,
	AID_actSetOffset,		AID_actSetOffset,		6,	0,	1,	PARAM_EXPRESSION,	AP0ID_actSetOffset,
	AID_actSetHorizontal,	AID_actSetHorizontal,	7,	0,	0,
	AID_actSetVertical,		AID_actSetVertical,		8,	0,	0,
	AID_actSetLeftTop,		AID_actSetLeftTop,		9,	0,	0,
	AID_actSetRightBottom,	AID_actSetRightBottom,	10,	0,	0,
	AID_actSetCustomValue,	AID_actSetCustomValue,	11,	0,	2, PARAM_EXPRESSION,	PARAM_EXPRESSION, AP0ID_actSetCustomValue, AP1ID_actSetCustomValue,
	AID_actSetWidth,		AID_actSetWidth,		12,	0,	1,	PARAM_EXPRESSION,	AP0ID_actSetWidth,
	AID_actSetHeight,		AID_actSetHeight,		13,	0,	1,	PARAM_EXPRESSION,	AP0ID_actSetHeight,
	AID_actSetResampleOn,	AID_actSetResampleOn,	14,	0,	0,
	AID_actSetResampleOff,	AID_actSetResampleOff,	15,	0,	0,
	AID_actSetSineOffset,	AID_actSetSineOffset,	16,	0,	0,
	AID_actSetCustomOffset,	AID_actSetCustomOffset,	17,	0,	0,
	0 };


// Definitions of parameters for each expression
short expressionsInfos[]=
	{ 
	EID_expGetZoomValue,	EID_expGetZoomValue,	0,	0,	0,
	EID_expGetOffset,		EID_expGetOffset,		1,	0,	0,
	EID_expNumWaves,		EID_expNumWaves,		2,	0,	0,
	EID_expGetCustom,		EID_expGetCustom,		3,	0,	1,	EXPPARAM_LONG,	0,
	EID_expGetWidth,		EID_expGetWidth,		4,	0,	0,
	EID_expGetHeight,		EID_expGetHeight,		5,	0,	0,
	0 };



// ============================================================================
//
// CONDITION ROUTINES
// 
// ============================================================================



// ============================================================================
//
// ACTIONS ROUTINES
// 
// ============================================================================

short WINAPI DLLExport actSetZoomValue(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->ZoomValue = param1;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetPanorama(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->Effect = PANORAMA;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetPerspective(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->Effect = PERSPECTIVE;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetSineWave(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->Effect = SINEWAVE;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetCustom(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->Effect = CUSTOM;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetSineOffset(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->Effect = SINEOFFSET;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetCustomOffset(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->Effect = CUSTOMOFFSET;
	rdPtr->roc.rcChanged = true;
	return 0;
}


short WINAPI DLLExport actSetNumWaves(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->SineWaveWaves = param1;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetOffset(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->Offset = param1;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetHorizontal(LPRDATA rdPtr, long param1, long param2)
{
	int OldSize = (rdPtr->Direction == HORIZONTAL) ? rdPtr->rHo.hoImgWidth : rdPtr->rHo.hoImgHeight;
	int NewSize = rdPtr->rHo.hoImgWidth;
	
	rdPtr->Direction = HORIZONTAL;
	rdPtr->roc.rcChanged = true;
	
	int MinSize = min(OldSize,NewSize);
	int * NewCustom = new int[NewSize];
	memset(NewCustom,0,NewSize*sizeof(int));

	for(int i=0; i<MinSize; i++)
		NewCustom[i] = rdPtr->CustomArray[i];

	delete[] rdPtr->CustomArray;
	rdPtr->CustomArray = NewCustom;

	return 0;
}

short WINAPI DLLExport actSetVertical(LPRDATA rdPtr, long param1, long param2)
{
	int OldSize = (rdPtr->Direction == HORIZONTAL) ? rdPtr->rHo.hoImgWidth : rdPtr->rHo.hoImgHeight;
	int NewSize = rdPtr->rHo.hoImgHeight;
	
	rdPtr->Direction = VERTICAL;
	rdPtr->roc.rcChanged = true;
	
	int MinSize = min(OldSize,NewSize);
	int * NewCustom = new int[NewSize];
	memset(NewCustom,0,NewSize*sizeof(int));

	for(int i=0; i<MinSize; i++)
		NewCustom[i] = rdPtr->CustomArray[i];

	delete[] rdPtr->CustomArray;
	rdPtr->CustomArray = NewCustom;

	return 0;
}

short WINAPI DLLExport actSetLeftTop(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->PerspectiveDir = 0;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetRightBottom(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->PerspectiveDir = 1;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetCustomValue(LPRDATA rdPtr, long param1, long param2)
{
	int size = (rdPtr->Direction == HORIZONTAL) ? rdPtr->rHo.hoImgWidth : rdPtr->rHo.hoImgHeight;

	if(param1 >= 0 && param1 < size)
		rdPtr->CustomArray[param1] = param2;
	
	rdPtr->roc.rcChanged = true;

	return 0;
}

short WINAPI DLLExport actSetWidth(LPRDATA rdPtr, long param1, long param2)
{
	int oldSize = (rdPtr->Direction == HORIZONTAL) ? rdPtr->rHo.hoImgWidth : rdPtr->rHo.hoImgHeight;
	rdPtr->rHo.hoImgWidth = param1;
	return resizePerspective(rdPtr, oldSize);
}

short WINAPI DLLExport actSetHeight(LPRDATA rdPtr, long param1, long param2)
{
	int oldSize = (rdPtr->Direction == HORIZONTAL) ? rdPtr->rHo.hoImgWidth : rdPtr->rHo.hoImgHeight;
	rdPtr->rHo.hoImgHeight = param1;
	return resizePerspective(rdPtr, oldSize);
}


int resizePerspective(LPRDATA rdPtr, int oldsize){
	//Recreate temp-image
/*	rdPtr->TempImage->Delete();
	fprh rhPtr = rdPtr->rHo.hoAdRunHeader;
	LPSURFACE ps = WinGetSurface((int)rhPtr->rhIdEditWin);
	LPSURFACE pProto = NULL;
	if ( GetSurfacePrototype(&pProto, ps->GetDepth(), ST_MEMORY, SD_DIB) ){
		rdPtr->TempImage->Create(rdPtr->rHo.hoImgWidth, rdPtr->rHo.hoImgHeight,  pProto);
			if( !rdPtr->TempImage->IsValid() )
			{return -1;}
	} */

	// Delete temp image so that it's automatically recreated with the correct size
	if ( rdPtr->TempImage != NULL )
	{
		delete rdPtr->TempImage;
		rdPtr->TempImage = NULL;
	}

	//Recreate temp-array
	
	int size = (rdPtr->Direction == HORIZONTAL) ? rdPtr->rHo.hoImgWidth : rdPtr->rHo.hoImgHeight;
	int * newArray = new int[size];
	int numNewInts = min(size,oldsize);
	for(int i=0;i<size;i++){
		if(i<numNewInts)
			newArray[i] = rdPtr->CustomArray[i];
		else
			newArray[i] = 0;
	}
	delete [] rdPtr->CustomArray;
	rdPtr->CustomArray = newArray;
	rdPtr->roc.rcChanged = true;
	
	return 0;
}


short WINAPI DLLExport actSetResampleOn(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->resample = true;
	rdPtr->roc.rcChanged = true;
	return 0;
}

short WINAPI DLLExport actSetResampleOff(LPRDATA rdPtr, long param1, long param2)
{
	rdPtr->resample = false;
	rdPtr->roc.rcChanged = true;
	return 0;
}


// ============================================================================
//
// EXPRESSIONS ROUTINES
// 
// ============================================================================

long WINAPI DLLExport expGetZoomValue(LPRDATA rdPtr, long param1)
{
	return rdPtr->ZoomValue;
}

long WINAPI DLLExport expGetOffset(LPRDATA rdPtr, long param1)
{
	return rdPtr->Offset;
}

long WINAPI DLLExport expNumWaves(LPRDATA rdPtr, long param1)
{
	return rdPtr->SineWaveWaves;
}

long WINAPI DLLExport expGetCustom(LPRDATA rdPtr, long param1)
{
	long p1=CNC_GetFirstExpressionParameter(rdPtr, param1, TYPE_INT);
	int size = (rdPtr->Direction == HORIZONTAL) ? rdPtr->rHo.hoImgWidth : rdPtr->rHo.hoImgHeight;

	return rdPtr->CustomArray[min(max(0,p1),size-1)];
}

long WINAPI DLLExport expGetWidth(LPRDATA rdPtr, long param1)
{
	return rdPtr->rHo.hoImgWidth;
}

long WINAPI DLLExport expGetHeight(LPRDATA rdPtr, long param1)
{
	return rdPtr->rHo.hoImgHeight;
}

// ----------------------------------------------------------
// Condition / Action / Expression jump table
// ----------------------------------------------------------
// Contains the address inside the extension of the different
// routines that handle the action, conditions and expressions.
// Located at the end of the source for convinience
// Must finish with a 0
//
long (WINAPI * ConditionJumps[])(LPRDATA rdPtr, long param1, long param2) = 
	{ 
	0};
	
short (WINAPI * ActionJumps[])(LPRDATA rdPtr, long param1, long param2) =
	{
	actSetZoomValue,	
	actSetPanorama,
	actSetPerspective,
	actSetSineWave,
	actSetCustom,
	actSetNumWaves,
	actSetOffset,
	actSetHorizontal,
	actSetVertical,
	actSetLeftTop,
	actSetRightBottom,
	actSetCustomValue,
	actSetWidth,
	actSetHeight,
	actSetResampleOn,
	actSetResampleOff,
	actSetSineOffset,
	actSetCustomOffset,
	0
	};

long (WINAPI * ExpressionJumps[])(LPRDATA rdPtr, long param) = 
	{
	expGetZoomValue,
	expGetOffset,
	expNumWaves,
	expGetCustom,
	expGetWidth,
	expGetHeight,
	0
	};