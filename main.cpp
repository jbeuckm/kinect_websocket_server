/*******************************************************************************
*                                                                              *
*   PrimeSense NITE 1.3 - Single Control Sample                                *
*   Copyright (C) 2010 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------
// General headers
#include <stdio.h>
// OpenNI headers
#include <XnOpenNI.h>
// NITE headers
#include <XnVSessionManager.h>
#include "XnVMultiProcessFlowClient.h"
#include <XnVWaveDetector.h>
#include <XnVPushDetector.h>
#include <XnVSwipeDetector.h>
#include <XnVCircleDetector.h>

// xml to initialize OpenNI
#define SAMPLE_XML_FILE "../../../Data/Sample-Tracking.xml"
#define SAMPLE_XML_FILE_LOCAL "Sample-Tracking.xml"

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE SessionProgress(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* UserCxt)
{
	printf("{\"type\":\"progress\",\"x\":%d,\"y\":%d,\"z\":%d, \"progress\":%f, \"focus\":\"%s\"}\n", (int)ptFocusPoint.X, (int)ptFocusPoint.Y, (int)ptFocusPoint.Z, fProgress,  strFocus);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStart(const XnPoint3D& ptFocusPoint, void* UserCxt)
{
	printf("{\"type\":\"start\", \"x\":%d,\"y\":%d,\"z\":%d}\n", (int)ptFocusPoint.X, (int)ptFocusPoint.Y, (int)ptFocusPoint.Z);
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnd(void* UserCxt)
{
	printf("{\"type\":\"end\"}\n");
}
// Callback for wave detection
void XN_CALLBACK_TYPE OnWaveCB(void* cxt)
{
	printf("{\"type\":\"wave\"}\n");
}
// Callback for push detection
void XN_CALLBACK_TYPE OnPushCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("{\"type\":\"push\", \"velocity\":%f, \"angle\":%f}\n", fVelocity, fAngle);
}


// Callback for swipe detection
void XN_CALLBACK_TYPE OnSwipeUpCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("{\"type\":\"swipe_up\",\"velocity\":%f,\"angle\":%f}\n", fVelocity, fAngle);
}
// Callback for swipe detection
void XN_CALLBACK_TYPE OnSwipeDownCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("{\"type\":\"swipe_down\",\"velocity\":%f,\"angle\":%f}\n", fVelocity, fAngle);
}
// Callback for swipe detection
void XN_CALLBACK_TYPE OnSwipeLeftCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("{\"type\":\"swipe_left\",\"velocity\":%f,\"angle\":%f}\n", fVelocity, fAngle);
}
// Callback for swipe detection
void XN_CALLBACK_TYPE OnSwipeRightCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("{\"type\":\"swipe_right\",\"velocity\":%f,\"angle\":%f}\n", fVelocity, fAngle);
}


// Callback for circle detection
void XN_CALLBACK_TYPE OnCircleCB(XnFloat fTimes, XnBool bConfident, const XnVCircle *pCircle, void* cxt)
{
	printf("{\"type\":\"circle\",\"times\":%f,\"confident\":%u}\n", fTimes, bConfident);
}



// callback for a new position of any hand
void XN_CALLBACK_TYPE OnPointUpdate(const XnVHandPointContext* pContext, void* cxt)
{
	printf("{\"type\":\"point\",\"id\":%d, \"x\":%d,\"y\":%d,\"z\":%d }\n", pContext->nID, (int)pContext->ptPosition.X, (int)pContext->ptPosition.Y, (int)pContext->ptPosition.Z);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

XnBool fileExists(const char *fn)
{
	XnBool exists;
	xnOSDoesFileExist(fn, &exists);
	return exists;
}


// this sample can run either as a regular sample, or as a client for multi-process (remote mode)
int main(int argc, char** argv)
{
	xn::Context context;
	xn::ScriptNode scriptNode;
	XnVSessionGenerator* pSessionGenerator;

    // Create context
    const char *fn = NULL;
    if      (fileExists(SAMPLE_XML_FILE)) fn = SAMPLE_XML_FILE;
    else if (fileExists(SAMPLE_XML_FILE_LOCAL)) fn = SAMPLE_XML_FILE_LOCAL;
    else {
        printf("Could not find '%s' nor '%s'. Aborting.\n" , SAMPLE_XML_FILE, SAMPLE_XML_FILE_LOCAL);
        return XN_STATUS_ERROR;
    }
    XnStatus rc = context.InitFromXmlFile(fn, scriptNode);
    if (rc != XN_STATUS_OK)
    {
        printf("Couldn't initialize: %s\n", xnGetStatusString(rc));
        return 1;
    }

    // Create the Session Manager
    pSessionGenerator = new XnVSessionManager();
    rc = ((XnVSessionManager*)pSessionGenerator)->Initialize(&context, "Click", "RaiseHand");
    if (rc != XN_STATUS_OK)
    {
        printf("Session Manager couldn't initialize: %s\n", xnGetStatusString(rc));
        delete pSessionGenerator;
        return 1;
    }
    

    // Initialization done. Start generating
    context.StartGeneratingAll();

	// Register session callbacks
	pSessionGenerator->RegisterSession(NULL, &SessionStart, &SessionEnd, &SessionProgress);

	// init & register wave control
	XnVWaveDetector wc;
	wc.RegisterWave(NULL, OnWaveCB);
	wc.RegisterPointUpdate(NULL, OnPointUpdate);
	pSessionGenerator->AddListener(&wc);

	// init & register push control
	XnVPushDetector pd;
	pd.RegisterPush(NULL, OnPushCB);
	pd.RegisterPointUpdate(NULL, OnPointUpdate);
	pSessionGenerator->AddListener(&pd);

	// init & register circle control
	XnVCircleDetector cd;
	cd.RegisterCircle(NULL, OnCircleCB);
	cd.RegisterPointUpdate(NULL, OnPointUpdate);
	pSessionGenerator->AddListener(&cd);

	// init & register swipe control
	XnVSwipeDetector sd;
	sd.RegisterSwipeUp(NULL, OnSwipeUpCB);
	sd.RegisterSwipeDown(NULL, OnSwipeDownCB);
	sd.RegisterSwipeLeft(NULL, OnSwipeLeftCB);
	sd.RegisterSwipeRight(NULL, OnSwipeRightCB);
	sd.RegisterPointUpdate(NULL, OnPointUpdate);
	pSessionGenerator->AddListener(&sd);

    setbuf(stdout, NULL);
    printf("start_data\n");

	// Main loop
	while (true)
	{
        context.WaitAnyUpdateAll();
        ((XnVSessionManager*)pSessionGenerator)->Update(&context);
	}

	delete pSessionGenerator;
	return 0;
}
