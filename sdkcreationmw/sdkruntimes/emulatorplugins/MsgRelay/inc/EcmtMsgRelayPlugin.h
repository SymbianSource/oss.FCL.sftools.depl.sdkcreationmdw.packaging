/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#ifndef SERVICEMANAGERPLUGIN_H
#define SERVICEMANAGERPLUGIN_H

#include <e32base.h>
#include "EcmtPlugin.h"
#include "EcmtMessagingIf.h"

class CMessageRelayAppDll;

class CMsgRelayPlugin : public CBase, public MEcmtPlugin
	{
	public:
		~CMsgRelayPlugin( );
		static MEcmtPlugin* NewL( );
		void BindMessagingL( MEcmtMessaging* aMessaging );
		
	private:
		CMsgRelayPlugin();
		void ConstructL();
		
	private:
		TUid 				iUid;
		TUid 				iTargetUid;
		CMessageRelayAppDll* iMessageRelayAppDll;
		
	};
	
#endif

