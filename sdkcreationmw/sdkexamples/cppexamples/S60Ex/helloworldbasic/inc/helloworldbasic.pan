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


#ifndef __HELLOWORLDBASIC_PAN__
#define __HELLOWORLDBASIC_PAN__

/** HelloWorldBasic application panic codes */
enum THelloWorldBasicPanics
    {
    EHelloWorldBasicUi = 1
    // add further panics here
    };

inline void Panic(THelloWorldBasicPanics aReason)
    {
    _LIT(applicationName,"HelloWorldBasic");
    User::Panic(applicationName, aReason);
    }

#endif // __HELLOWORLDBASIC_PAN__
