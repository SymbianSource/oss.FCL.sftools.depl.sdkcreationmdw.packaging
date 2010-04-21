// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// RawIP MCPR Factory
// 
//

/**
 @file
 @internalComponent
*/

#ifndef SYMBIAN_WINSCOKMCPR_FACTORY_H
#define SYMBIAN_WINSCOKMCPR_FACTORY_H

#include <comms-infras/ss_metaconnprov.h>

class CWinsockMetaConnectionProviderFactory : public ESock::CMetaConnectionProviderFactoryBase
	{
public:
    enum { iUid = 0x10281e00 };
	static CWinsockMetaConnectionProviderFactory* NewL(TAny* aParentContainer);

protected:
    CWinsockMetaConnectionProviderFactory(TUid aFactoryId, ESock::CMetaConnectionFactoryContainer& aParentContainer);
	ESock::ACommsFactoryNodeId* DoCreateObjectL(ESock::TFactoryQueryBase& aQuery);
	};

#endif
//SYMBIAN_RAWIPMCPR_FACTORY_H
