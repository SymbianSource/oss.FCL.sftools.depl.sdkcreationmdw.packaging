/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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


// INCLUDE FILES
#include "ChoiceListExApp.h"
#include "ChoiceListExDocument.h"


// ============================ MEMBER FUNCTIONS ===============================

// UID for the application;
// this should correspond to the uid defined in the mmp file
static const TUid KUidChoiceListExApp = {0xA000027D};

// -----------------------------------------------------------------------------
// CChoiceListExApp::CreateDocumentL()
// Creates CApaDocument object
// -----------------------------------------------------------------------------
//
CApaDocument* CChoiceListExApp::CreateDocumentL()
    {  
    // Create an ChoiceListEx document, and return a pointer to it
    CApaDocument* document = CChoiceListExDocument::NewL( *this );
    return document;
    }

// -----------------------------------------------------------------------------
// CChoiceListExApp::AppDllUid()
// Returns application UID
// -----------------------------------------------------------------------------
//
TUid CChoiceListExApp::AppDllUid() const
    {
    // Return the UID for the ChoiceListEx application
    return KUidChoiceListExApp;
    }

