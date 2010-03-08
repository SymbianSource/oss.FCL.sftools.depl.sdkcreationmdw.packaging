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
* Description:  This class functions as controller of the application
*
*/


// INCLUDE FILES
#include <avkon.hrh>
#include "LBSReferenceAppUi.h"
#include "LBSReferenceContainer.h"



// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CLbsReferenceAppUi::ConstructL()
// Symbian OS two phased constructor
// ----------------------------------------------------------
//
void CLbsReferenceAppUi::ConstructL()
    {
    // Perform the base class construction
    BaseConstructL(EAknEnableSkin);

    //Create container
    iAppContainer = new (ELeave) CLbsReferenceContainer;
    iAppContainer->SetMopParent(this);
    iAppContainer->ConstructL( ClientRect() );

    // Allow the appview to receive keyboard input
    AddToStackL( iAppContainer );
    }

// ----------------------------------------------------
// CLbsReferenceAppUi::~CLbsReferenceAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CLbsReferenceAppUi::~CLbsReferenceAppUi()
    {
    // Cleanup
    if ( iAppContainer )
        {
        //Remove from stack
        RemoveFromStack( iAppContainer );

        //Delete the object
        delete iAppContainer;
        }
   }


// ----------------------------------------------------
// CLbsReferenceAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
//
// ----------------------------------------------------
//
TKeyResponse CLbsReferenceAppUi::HandleKeyEventL(
    const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
    {
    // The key event was not handled
    return EKeyWasNotConsumed;
    }

// ----------------------------------------------------
// CLbsReferenceAppUi::HandleCommandL(TInt aCommand)
//
// ----------------------------------------------------
//
void CLbsReferenceAppUi::HandleCommandL(TInt aCommand)
    {
    switch ( aCommand )
        {
        // Exit the application
        case EEikCmdExit:
        case EAknSoftkeyBack:
        case EAknSoftkeyExit:
            {
            //Exit
            Exit();
            break;
            }
        // Do nothing.
        default:
            break;
        }
    }

// End of File
