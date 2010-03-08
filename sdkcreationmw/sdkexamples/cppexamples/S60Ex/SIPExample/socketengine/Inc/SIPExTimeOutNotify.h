/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef MSIPEXTIMEOUTNOTIFY_H
#define MSIPEXTIMEOUTNOTIFY_H

// CLASS DECLARATIONS
/**
* Interface for notifing timer's timeout.
*/
class MSIPExTimeOutNotify 
    {
    public: 
        virtual void TimerExpired() = 0;
    };

#endif // MSIPEXTIMEOUTNOTIFY_H

// End of file