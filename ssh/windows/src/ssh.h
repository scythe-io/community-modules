/*++

 Copyright (c) SCYTHE, Inc. Use is subject to agreement.

 --*/

#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

__checkReturn __success(return == SCYTHE_NOERROR) INT init(__in PSCYTHE_CLIENT pClient, __in_opt PVOID pReserved);
__checkReturn __success(return == SCYTHE_NOERROR) ERROR_T run(__in PSCYTHE_MESSAGE pMessage, __in_opt PVOID Reserved);
__checkReturn __success(return == SCYTHE_NOERROR) ERROR_T getinfo(__inout PSCYTHE_MODULE_INFORMATION pInformation);
__checkReturn __success(return == SCYTHE_NOERROR) ERROR_T deinit(__in PVOID reserved);

#ifdef __cplusplus
}
#endif