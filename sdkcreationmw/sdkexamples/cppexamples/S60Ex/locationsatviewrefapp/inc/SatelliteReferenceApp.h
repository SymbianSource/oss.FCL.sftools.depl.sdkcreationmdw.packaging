/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef SATELLITE_REFERENCE_APP_H
#define SATELLITE_REFERENCE_APP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidSatelliteReferenceApp = { 0xA000018A };

// CLASS DECLARATION

/**
* SatelliteReference application class.
* Provides factory to create concrete document object.
* 
*/
class CSatelliteReferenceApp : public CAknApplication
    {
    
    public: // Functions from base classes
    private:

        /**
        * From CApaApplication, creates document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID.
        * @return The value of KUidSatelliteReferenceApp.
        */
        TUid AppDllUid() const;
    };

#endif

// End of File

