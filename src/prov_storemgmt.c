/*
 * Copyright (c) 2022 Zoltan Patocs
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_object.h>
#include <openssl/params.h>
#include <openssl/store.h>

#include "prov_storemgmt.h"

// -------------------------------------------------------------------------------------------------

static const char* keytype_rsa = "RSA";
static const char* keytype_rsa_pss = "RSA-PSS";
static const char* keytype_ec = "EC";

typedef EVP_PKEY*(EVP_LOADER)(PROVIDER_CTX*, const char*, OSSL_PASSPHRASE_CALLBACK*, void*);

// -------------------------------------------------------------------------------------------------

static OSSL_FUNC_store_open_fn p11_store_open;
static OSSL_FUNC_store_load_fn p11_store_load;
static OSSL_FUNC_store_eof_fn p11_store_eof;
static OSSL_FUNC_store_close_fn p11_store_close;
static OSSL_FUNC_store_settable_ctx_params_fn p11_store_settable_ctx_params;
static OSSL_FUNC_store_set_ctx_params_fn p11_store_set_ctx_params;

// -------------------------------------------------------------------------------------------------

extern void pkcs11_object_free(PKCS11_OBJECT_private* obj); /* Free an object */

extern EVP_PKEY* ctx_load_privkey(PROVIDER_CTX* ctx, const char* s_key_id, OSSL_PASSPHRASE_CALLBACK* pw_cb, void* pw_cbarg);
extern EVP_PKEY* ctx_load_pubkey(PROVIDER_CTX* ctx, const char* s_key_id, OSSL_PASSPHRASE_CALLBACK* pw_cb, void* pw_cbarg);

// -------------------------------------------------------------------------------------------------

static P11_STORE_CTX* __new_p11_storectx()
{
    return calloc(1, sizeof(P11_STORE_CTX));
}

static void __free_p11_storectx(P11_STORE_CTX* ptr)
{
    if (ptr)
    {
        free(ptr->uri);
        free(ptr->label);
        if (ptr->object)
        {
            pkcs11_object_free(ptr->object);
        }

        memset(ptr, 0, sizeof(P11_STORE_CTX));

        free(ptr);
    }
}

// -------------------------------------------------------------------------------------------------

static const OSSL_DISPATCH p11_storemgmt_tbl[] = {
{OSSL_FUNC_STORE_OPEN, (void (*)(void))p11_store_open},
// {OSSL_FUNC_STORE_ATTACH, (void (*)(void))p11_store_attach},
{OSSL_FUNC_STORE_SETTABLE_CTX_PARAMS, (void (*)(void))p11_store_settable_ctx_params},
{OSSL_FUNC_STORE_SET_CTX_PARAMS, (void (*)(void))p11_store_set_ctx_params},
{OSSL_FUNC_STORE_LOAD, (void (*)(void))p11_store_load},
{OSSL_FUNC_STORE_EOF, (void (*)(void))p11_store_eof},
{OSSL_FUNC_STORE_CLOSE, (void (*)(void))p11_store_close},
// {OSSL_FUNC_STORE_EXPORT_OBJECT, (void (*)(void))p11_store_export_object},
{0, NULL}};

static const OSSL_ALGORITHM p11_dispatch_storemgmt[] = {
{"pkcs11", "provider=pkcs11", p11_storemgmt_tbl, NULL},
{NULL, NULL, NULL, NULL}};

const OSSL_ALGORITHM* p11_get_ops_storemgmt(void* provctx, int* no_store)
{
    (void)no_store;

    PROVIDER_CTX* ctx = provctx;

    ctx_log(ctx, 3, "%s\n", __FUNCTION__);

    return !ctx->b_storemgmt_disabled ? p11_dispatch_storemgmt : NULL;
}

// -------------------------------------------------------------------------------------------------

static void* p11_store_open(void* provctx, const char* uri)
{
    P11_STORE_CTX* storectx;

    if (!(storectx = __new_p11_storectx()))
    {
        return NULL;
    }

    storectx->provctx = provctx;

    if (provctx == NULL || uri == NULL || strncmp("pkcs11://", uri, 9))
        goto err;

    storectx->uri = strdup(uri + 9); /* strlen("pkcs11://") = 9 */

    // No real check, since we don't know the object type, only the uri

    return storectx;

err:
    __free_p11_storectx(storectx);
    return NULL;
}

static int p11_store_close(void* storectx)
{
    assert(storectx != NULL);

    __free_p11_storectx((P11_STORE_CTX*)storectx);

    return 0;
}

// -------------------------------------------------------------------------------------------------

static EVP_PKEY* p11_store_load_evp(void* storectx, OSSL_CALLBACK* object_cb, void* object_cbarg,
                                    OSSL_PASSPHRASE_CALLBACK* pw_cb, void* pw_cbarg,
                                    EVP_LOADER* loader)
{
    P11_STORE_CTX* ctx = storectx;
    EVP_PKEY* key;
    EVP_PKEY* return_key;

    OSSL_PARAM params[4];
    int object_type = OSSL_OBJECT_PKEY;
    const char* keytype; // RSA, RSA-PSS, EC

    if (!(key = loader(ctx->provctx, ctx->uri, pw_cb, pw_cbarg)))
    {
        ctx_log(ctx->provctx, 0, "EVP loader returned null\n");
        goto err;
    }

    int pkey_type = EVP_PKEY_get_base_id(key);
    switch (pkey_type)
    {
        case EVP_PKEY_RSA:
            keytype = keytype_rsa;
            break;

        case EVP_PKEY_RSA_PSS:
            keytype = keytype_rsa_pss;
            break;

        case EVP_PKEY_EC:
            keytype = keytype_ec;
            break;

        default:
            ctx_log(ctx->provctx, 0, "keytype unhandled (%d)\n", pkey_type);
            goto err;
    }

    return_key = key;

    params[0] = OSSL_PARAM_construct_int(OSSL_OBJECT_PARAM_TYPE, &object_type);
    params[1] = OSSL_PARAM_construct_utf8_string(OSSL_OBJECT_PARAM_DATA_TYPE, (char*)keytype, 0);
    /* The address of the key becomes the octet string */
    params[2] = OSSL_PARAM_construct_octet_string(OSSL_OBJECT_PARAM_REFERENCE, &key, sizeof(key));
    params[3] = OSSL_PARAM_construct_end();

    if (!object_cb(params, object_cbarg))
    {
        ctx_log(ctx->provctx, 0, "object_cb returned error\n");
        goto err;
    }

    return return_key;

err:
    if (key)
    {
        EVP_PKEY_free(key);
    }

    return NULL;
}

static int p11_store_load(void* storectx,
                          OSSL_CALLBACK* object_cb, void* object_cbarg,
                          OSSL_PASSPHRASE_CALLBACK* pw_cb, void* pw_cbarg)
{
    P11_STORE_CTX* ctx = storectx;
    struct ossl_load_result_data_st* load_data = object_cbarg;
    OSSL_STORE_INFO* info;

    assert(storectx != NULL);

    if ((info = calloc(1, sizeof(OSSL_STORE_INFO))) == NULL)
    {
        return 0;
    }

    ctx->state = P11_STORE_CTX_STATE_LOADING;
    info->type = ctx->type;

    switch (ctx->type)
    {
        case OSSL_STORE_INFO_PKEY:
        {
            if (ctx->privkey)
            {
                EVP_PKEY_free(ctx->privkey);
                ctx->privkey = NULL;
            }

            if (!(ctx->privkey = p11_store_load_evp(ctx, object_cb, object_cbarg, pw_cb, pw_cbarg, ctx_load_privkey)))
            {
                ctx_log(ctx->provctx, 0, "ctx_load_privkey returned null\n");
                goto err;
            }

            info->_.pkey = ctx->privkey;
            EVP_PKEY_up_ref(ctx->privkey);
        }

        break;

        case OSSL_STORE_INFO_PUBKEY:
        {
            if (ctx->pubkey)
            {
                EVP_PKEY_free(ctx->pubkey);
                ctx->pubkey = NULL;
            }

            if (!(ctx->pubkey = p11_store_load_evp(ctx, object_cb, object_cbarg, pw_cb, pw_cbarg, ctx_load_pubkey)))
            {
                ctx_log(ctx->provctx, 0, "ctx_load_pubkey returned null\n");
                goto err;
            }

            info->_.pubkey = ctx->pubkey;
            EVP_PKEY_up_ref(ctx->pubkey);
        }
        break;

        default:
            ctx_log(ctx->provctx, 0, "unhandled type: %d\n", ctx->type);
            goto err;
    }

    load_data->v = info;
    ctx->state = P11_STORE_CTX_STATE_LOADED;

    return 1;

err:
    free(info);
    ctx->state = P11_STORE_CTX_STATE_ERROR;
    return 0;
}

static int p11_store_eof(void* storectx)
{
    P11_STORE_CTX* ctx;

    assert(storectx != NULL);

    ctx = storectx;

    return (ctx->state == P11_STORE_CTX_STATE_LOADED || ctx->state == P11_STORE_CTX_STATE_ERROR);
}

static const OSSL_PARAM* p11_store_settable_ctx_params(void* provctx)
{
    static const OSSL_PARAM p11_store_settable_params_tbl[] = {
    OSSL_PARAM_END};

    (void)(provctx);

    return p11_store_settable_params_tbl;
}

static int p11_store_set_ctx_params(void* storectx, const OSSL_PARAM params[])
{
    P11_STORE_CTX* ctx;
    const OSSL_PARAM* p;

    assert(storectx != NULL);

    ctx = storectx;

    if ((p = OSSL_PARAM_locate_const(params, "expect")) != NULL)
    {
        assert(p->data_type == OSSL_PARAM_INTEGER);

        ctx->type = *(int*)p->data;

        ctx_log(ctx->provctx, 1, "expected type: %s\n", OSSL_STORE_INFO_type_string(ctx->type));
    }

    return 1;
}
