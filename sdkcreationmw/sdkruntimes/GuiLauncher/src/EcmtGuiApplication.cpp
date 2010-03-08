/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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


#include "EcmtGuiDocument.h"
#include "EcmtGuiApplication.h"
#include "EcmtGuiUid.h"

// Create an EcmtGui document, and return a pointer to it
CApaDocument* CEcmtGuiApplication::CreateDocumentL()
{
    return CEcmtGuiDocument::NewL(*this, KUidEcmtGuiApp);
}

// Return the UID for the EcmtGui application
TUid CEcmtGuiApplication::AppDllUid() const
{
    return KUidEcmtGuiApp;
}

