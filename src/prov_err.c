/*
 * Copyright 1995-2017 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/err.h>
#include <stdio.h>

#include "prov_err.h"

#define PROV_LIB_NAME "pkcs11 provider"

/* BEGIN ERROR CODES */
#ifndef OPENSSL_NO_ERR

#define ERR_FUNC(func) ERR_PACK(0, func, 0)
#define ERR_REASON(reason) ERR_PACK(0, 0, reason)

static ERR_STRING_DATA PROV_str_functs[] = {
// {ERR_FUNC(PROV_F_CTX_CTRL_LOAD_CERT), "ctx_ctrl_load_cert"},
{ERR_FUNC(PROV_F_CTX_CTRL_SET_PIN), "ctx_ctrl_set_pin"},
// {ERR_FUNC(PROV_F_CTX_ENGINE_CTRL), "ctx_engine_ctrl"},
{ERR_FUNC(PROV_F_CTX_LOAD_CERT), "ctx_load_cert"},
// {ERR_FUNC(PROV_F_CTX_LOAD_KEY), "ctx_load_key"},
{ERR_FUNC(PROV_F_CTX_LOAD_PRIVKEY), "ctx_load_privkey"},
{ERR_FUNC(PROV_F_CTX_LOAD_PUBKEY), "ctx_load_pubkey"},
{ERR_FUNC(PROV_F_CTX_LOAD_OBJECT), "ctx_load_object"},
{ERR_FUNC(PROV_F_STOREMGMT), "p11_store_*"},
{ERR_FUNC(PROV_F_CTX_GET_PIN), "ctx_get_pin"},
{ERR_FUNC(PROV_CTX_LOGIN), "ctx_login"},
{0, NULL}};

static ERR_STRING_DATA PROV_str_reasons[] = {
{ERR_REASON(PROV_R_INVALID_ID), "invalid id"},
{ERR_REASON(PROV_R_INVALID_PARAMETER), "invalid parameter"},
{ERR_REASON(PROV_R_OBJECT_NOT_FOUND), "object not found"},
{ERR_REASON(PROV_R_UNKNOWN_COMMAND), "unknown command"},
{ERR_REASON(PROV_R_LOAD_OBJECT_CB), "error while loading object with object_cb on opened PKCS11 uri"},
{ERR_REASON(PROV_R_UNHANDLED_KEY_TYPE), "unhandled key type"},
{ERR_REASON(PROV_R_EVP_LOADER_ERR), "EVP loader returned error"},
{ERR_REASON(PROV_R_CANNOT_LOAD_PRIVKEY), "cannot load private key"},
{ERR_REASON(PROV_R_CANNOT_LOAD_PUBKEY), "cannot load public key"},
{ERR_REASON(PROV_R_CANNOT_LOAD_UNHANDLED_DATA_TYPE), "cannot load data of unhandled data type"},
{ERR_REASON(PROV_R_UNHANDLED_KEY_TYPE), "trying to load a key of unhandled key type"},
{ERR_REASON(PROV_R_CANNOT_PARSE_URI), "cannot parse pkcs11 uri"},
{ERR_REASON(PROV_R_MEMORY), "not enough memory"},
{ERR_REASON(PROV_R_URI_TYPE_UNKNOWN), "unknown type value in pkcs11 uri"},
{ERR_REASON(PROV_R_INPUT_FAILED), "pin input failed"},
{ERR_REASON(PROV_R_LOGIN_FAILED), "token login failed"},
{ERR_REASON(PROV_R_SLOT_AMBIGUOUS), "multiple slots found"},
{ERR_REASON(PROV_R_CANNOT_LOAD_CERT), "cannot load certificate"},
{0, NULL}};

#endif

#ifdef PROV_LIB_NAME
static ERR_STRING_DATA PROV_lib_name[] = {
{0, PROV_LIB_NAME},
{0, NULL}};
#endif

static int PROV_lib_error_code = 0;
static int PROV_error_init = 1;

int ERR_load_PROV_strings(void)
{
    if (PROV_lib_error_code == 0)
        PROV_lib_error_code = ERR_get_next_error_library();

    if (PROV_error_init)
    {
        PROV_error_init = 0;
#ifndef OPENSSL_NO_ERR
        ERR_load_strings(PROV_lib_error_code, PROV_str_functs);
        ERR_load_strings(PROV_lib_error_code, PROV_str_reasons);
#endif

#ifdef PROV_LIB_NAME
        PROV_lib_name->error = ERR_PACK(PROV_lib_error_code, 0, 0);
        ERR_load_strings(0, PROV_lib_name);
#endif
    }
    return 1;
}

void ERR_unload_PROV_strings(void)
{
    if (PROV_error_init == 0)
    {
#ifndef OPENSSL_NO_ERR
        ERR_unload_strings(PROV_lib_error_code, PROV_str_functs);
        ERR_unload_strings(PROV_lib_error_code, PROV_str_reasons);
#endif

#ifdef PROV_LIB_NAME
        ERR_unload_strings(0, PROV_lib_name);
#endif
        PROV_error_init = 1;
    }
}

void ERR_PROV_error(int function, int reason, char* file, int line)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    (void)function;
#endif

    if (PROV_lib_error_code == 0)
        PROV_lib_error_code = ERR_get_next_error_library();
    ERR_PUT_error(PROV_lib_error_code, function, reason, file, line);
}
