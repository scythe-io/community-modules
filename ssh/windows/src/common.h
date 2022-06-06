/*++

 Copyright (c) SCYTHE, Inc. Use is subject to agreement.

 --*/
#pragma once

#include <windows.h>

typedef unsigned int ERROR_T;

#define SCYTHE_NOERROR 0                        // The operation completed successfully.
#define SCYTHE_ERROR_INVALID_PARAMETER 87       // The parameter is incorrect.
#define SCYTHE_ERROR_OUTOFMEMORY 14             // Not enough storage is available to complete this operation.
#define SCYTHE_ERROR_INTERNAL 1359              // An internal error occurred.
#define SCYTHE_ERROR_DATA_CHECKSUM_ERROR 323    // A data integrity checksum error occurred.Data in the file stream is corrupt.
#define SCYTHE_ERROR_ALREADY_RUNNING 183        // Already exists.
#define SCYTHE_ERROR_NOT_ALLOWED 5              // Access is denied.
#define SCYTHE_ERROR_NOT_FOUND 1168             // Element not found.
#define SCYTHE_ERROR_FILE_TOO_LARGE 8312        // The specified object is too large.

#pragma pack(push, 1)
typedef struct _SCYTHE_MESSAGE
{
    OVERLAPPED  Overlapped;
    UINT32      Flags;
    GUID        MessageId;
    UINT32      DestinationModuleType;
    GUID        OriginModuleId;
    GUID        DestinationModuleId;
    UINT64      Context;
    UINT64      MessageSize;
    BYTE        Message[1];
} SCYTHE_MESSAGE, *PSCYTHE_MESSAGE;
#pragma pack(pop)

typedef ERROR_T (* POST_MESSAGE)(
    __in_opt GUID                         MessageId,
    __in_opt UINT32                       Flags,
    __in_opt UINT64                       Context,
    __in     UINT32                       ModuleType,
    __in_opt LPCGUID                      pOriginModuleId,
    __in_opt LPCGUID                      pDestinationModuleId,
    __in_opt UINT64                       MessageSize,
    __in_opt LPCBYTE                      Message);

typedef PVOID PSCYTHE_SERVER_PUBLICKEY;
typedef PVOID PSCYTHE_CLIENT_PRIVATEKEY;

typedef ERROR_T (* SIGN_AND_ENCRYPT_MESSAGE)(
    __in PSCYTHE_MESSAGE                  pMessage,
    __in PSCYTHE_SERVER_PUBLICKEY         pEncryptionKey,
    __in PSCYTHE_CLIENT_PRIVATEKEY        pSigningKey,
    __out_bcount(*pcbCipherSize) LPVOID * ppvCipher,
    __out LPDWORD                         pcbCipherSize);

typedef ERROR_T (* DECRYPT_AND_VERIFY_MESSAGE)(
    __inout_bcount(cbCipherSize) PVOID    pvCipher,
    __in DWORD                            cbCipherSize,
    __in PSCYTHE_CLIENT_PRIVATEKEY        pDecryptionKey,
    __in PSCYTHE_SERVER_PUBLICKEY         pVerifyKey,
    __out PSCYTHE_MESSAGE *               ppMessage);


typedef VOID (* GET_ENCRYPTION_KEYS)(
    __out PSCYTHE_SERVER_PUBLICKEY *      ppEncryptionKey,
    __out PSCYTHE_SERVER_PUBLICKEY *      ppVerificationKey,
    __out PSCYTHE_CLIENT_PRIVATEKEY *     ppDecryptionKey);

typedef VOID (* FREE_ENCRYPTION_DATA)(
    __in_opt PVOID                        pvMessage);

#pragma pack(push, 1)
typedef struct _SCYTHE_CLIENT
{
    DWORD                       cbSize;
    DWORD                       Flags;
    POST_MESSAGE                PostToMessagingSubsystem;
    PVOID                       Reserved1[2];
    GUID                        CampaignId;
    PVOID                       Reserved2[6];
    SIGN_AND_ENCRYPT_MESSAGE    SignAndEncryptMessage;
    DECRYPT_AND_VERIFY_MESSAGE  DecryptAndVerifyMessage;
    FREE_ENCRYPTION_DATA        FreeEncryptionData;
    GET_ENCRYPTION_KEYS         GetEncryptionKeys;
} SCYTHE_CLIENT, *PSCYTHE_CLIENT;
#pragma pack(pop)

typedef enum _SCYTHE_MODULE_TYPE
{
    CATEGORY_NONE = 0,
    CATEGORY_INPUT_COMMUNICATION = 1,
    CATEGORY_OUTPUT_COMMUNICATION = 2,
    CATEGORY_WORKER = 4
} SCYTHE_MODULE_TYPE;

#pragma pack(push, 1)
typedef struct _SCYTHE_MODULE_VERSION
{
    WORD	wMajor;
    WORD	wMinor;
} SCYTHE_MODULE_VERSION, *PSCYTHE_MODULE_VERSION;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _SCYTHE_MODULE_INFORMATION
{
    DWORD                           cbSize;
    DWORD                           Flags;
    DWORD                           ModuleType;
    SCYTHE_MODULE_VERSION           ModuleVersion;
    GUID                            ModuleId;
    PVOID                           Reserved1;
    PVOID                           Reserved2;
} SCYTHE_MODULE_INFORMATION, *PSCYTHE_MODULE_INFORMATION;
#pragma pack(pop)

#ifdef SHOW_LOGS
typedef ULONG(*DBGPRINT_FN) (
    _In_ PCHAR Format,
    ...);

#define INITIALIZE_DBGPRINT(x)  \
        {\
            HMODULE NTDLL = GetModuleHandleW(L"NTDLL"); \
            if (NULL != NTDLL) \
            { \
                DbgPrint = (DBGPRINT_FN)GetProcAddress(NTDLL, "DbgPrint"); \
            } \
        }

#define DBGPRINT(x, ...) if (DbgPrint) { DbgPrint(x, __VA_ARGS__); }

#define DECLARE_DBGPRINT(x) \
        DBGPRINT_FN DbgPrint = NULL;
#else
#define DECLARE_DBGPRINT(x) 
#define DBGPRINT(x, ...)
#define INITIALIZE_DBGPRINT(x) ((void)0)

#endif

