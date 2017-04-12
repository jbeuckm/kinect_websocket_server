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





#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;






// xml to initialize OpenNI
#define SAMPLE_XML_FILE "../../../Data/Sample-Tracking.xml"
#define SAMPLE_XML_FILE_LOCAL "Sample-Tracking.xml"

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE SessionProgress(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* UserCxt)
{
	printf("Session progress (%6.2f,%6.2f,%6.2f) - %6.2f [%s]\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z, fProgress,  strFocus);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStart(const XnPoint3D& ptFocusPoint, void* UserCxt)
{
	printf("Session started. Please wave (%6.2f,%6.2f,%6.2f)...\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z);
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnd(void* UserCxt)
{
	printf("Session ended. Please perform focus gesture to start session\n");
}
// Callback for wave detection
void XN_CALLBACK_TYPE OnWaveCB(void* cxt)
{
	printf("Wave!\n");
}
// Callback for push detection
void XN_CALLBACK_TYPE OnPushCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("Push! %f %f\n", fVelocity, fAngle);
}


// Callback for swipe detection
void XN_CALLBACK_TYPE OnSwipeUpCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("Swipe Up! %f %f\n", fVelocity, fAngle);
}
// Callback for swipe detection
void XN_CALLBACK_TYPE OnSwipeDownCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("Swipe Down! %f %f\n", fVelocity, fAngle);
}
// Callback for swipe detection
void XN_CALLBACK_TYPE OnSwipeLeftCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("Swipe Left! %f %f\n", fVelocity, fAngle);
}
// Callback for swipe detection
void XN_CALLBACK_TYPE OnSwipeRightCB(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("Swipe Right! %f %f\n", fVelocity, fAngle);
}


// Callback for circle detection
void XN_CALLBACK_TYPE OnCircleCB(XnFloat fTimes, XnBool bConfident, const XnVCircle *pCircle, void* cxt)
{
	printf("Circle! %f %d\n", fTimes, bConfident);
}



// callback for a new position of any hand
void XN_CALLBACK_TYPE OnPointUpdate(const XnVHandPointContext* pContext, void* cxt)
{
	printf("%d: (%f,%f,%f) [%f]\n", pContext->nID, pContext->ptPosition.X, pContext->ptPosition.Y, pContext->ptPosition.Z, pContext->fTime);
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
	XnBool bRemoting = FALSE;

	if (argc > 1)
	{
		// remote mode
		context.Init();
		printf("Running in 'Remoting' mode (Section name: %s)\n", argv[1]);
		bRemoting = TRUE;

		// Create multi-process client
		pSessionGenerator = new XnVMultiProcessFlowClient(argv[1]);

		XnStatus rc = ((XnVMultiProcessFlowClient*)pSessionGenerator)->Initialize();
		if (rc != XN_STATUS_OK)
		{
			printf("Initialize failed: %s\n", xnGetStatusString(rc));
			delete pSessionGenerator;
			return 1;
		}
	}
	else
	{
		// Local mode
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
	}

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

	// init & register push control
	XnVSwipeDetector sd;
	sd.RegisterSwipeUp(NULL, OnSwipeUpCB);
	sd.RegisterSwipeDown(NULL, OnSwipeDownCB);
	sd.RegisterSwipeLeft(NULL, OnSwipeLeftCB);
	sd.RegisterSwipeRight(NULL, OnSwipeRightCB);
	sd.RegisterPointUpdate(NULL, OnPointUpdate);
	pSessionGenerator->AddListener(&sd);

	printf("Please perform focus gesture to start session\n");
	printf("Hit any key to exit\n");

	// Main loop
	while (!xnOSWasKeyboardHit())
	{
		if (bRemoting)
		{
			((XnVMultiProcessFlowClient*)pSessionGenerator)->ReadState();
		}
		else
		{
			context.WaitAnyUpdateAll();
			((XnVSessionManager*)pSessionGenerator)->Update(&context);
		}
	}

	delete pSessionGenerator;

	return 0;
}
