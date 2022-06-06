/*++

 Copyright (c) SCYTHE, Inc. Use is subject to agreement.

 --*/
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
typedef struct _GUID {
    unsigned int  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
#pragma pack(pop)

#define VOID void
#define ULONG_PTR DWORD
typedef char CHAR;
typedef unsigned char BYTE;
typedef BYTE* PBYTE;
typedef unsigned short WORD;
typedef uint32_t DWORD;
typedef void* HANDLE;
typedef void* PVOID;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef const GUID* LPCGUID;
typedef void* LPVOID;
typedef DWORD* LPDWORD;
typedef int INT;
typedef const BYTE* LPCBYTE;

#pragma pack(push, 1)
typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        } DUMMYSTRUCTNAME;
        PVOID Pointer;
    } DUMMYUNIONNAME;

    HANDLE  hEvent;
} OVERLAPPED, * LPOVERLAPPED;
#pragma pack(pop)

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
    GUID                         MessageId,
    UINT32                       Flags,
    UINT64                       Context,
    UINT32                       ModuleType,
    LPCGUID                      pOriginModuleId,
    LPCGUID                      pDestinationModuleId,
    UINT64                       MessageSize,
    LPCBYTE                      Message);

typedef PVOID PSCYTHE_SERVER_PUBLICKEY;
typedef PVOID PSCYTHE_CLIENT_PRIVATEKEY;

typedef ERROR_T (* SIGN_AND_ENCRYPT_MESSAGE)(
    PSCYTHE_MESSAGE                  pMessage,
    PSCYTHE_SERVER_PUBLICKEY         pEncryptionKey,
    PSCYTHE_CLIENT_PRIVATEKEY        pSigningKey,
    LPVOID *                         ppvCipher,
    LPDWORD                          pcbCipherSize);

typedef ERROR_T (* DECRYPT_AND_VERIFY_MESSAGE)(
    PVOID                            pvCipher,
    DWORD                            cbCipherSize,
    PSCYTHE_CLIENT_PRIVATEKEY        pDecryptionKey,
    PSCYTHE_SERVER_PUBLICKEY         pVerifyKey,
    PSCYTHE_MESSAGE *               ppMessage);


typedef VOID (* GET_ENCRYPTION_KEYS)(
    PSCYTHE_SERVER_PUBLICKEY *      ppEncryptionKey,
    PSCYTHE_SERVER_PUBLICKEY *      ppVerificationKey,
    PSCYTHE_CLIENT_PRIVATEKEY *     ppDecryptionKey);

typedef VOID (* FREE_ENCRYPTION_DATA)(
    PVOID                        pvMessage);

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
