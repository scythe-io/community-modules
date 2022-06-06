/*++

 Copyright (c) SCYTHE, Inc. Use is subject to agreement.

 --*/

#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

INT init(PSCYTHE_CLIENT pClient, PVOID pReserved);
ERROR_T run(PSCYTHE_MESSAGE pMessage, PVOID Reserved);
ERROR_T getinfo(PSCYTHE_MODULE_INFORMATION pInformation);
ERROR_T deinit(PVOID reserved);

#ifdef __cplusplus
}
#endif