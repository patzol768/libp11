#include <stdio.h>
#include <stdlib.h>

#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/params.h>

#include "libp11-int.h"
#include "prov_keymgmt.h"
#include "prov_parse.h"

#define STRING_BUF_LEN 32
#define MAX_PIN_LENGTH 32
#define MAX_URI_LENGTH 256

extern EVP_PKEY* ctx_load_privkey(PROVIDER_CTX* ctx, const char* s_key_id, OSSL_PASSPHRASE_CALLBACK* pw_cb, void* pw_cbarg);

/******************************************************************************/

static OSSL_FUNC_keymgmt_gen_init_fn keymgmt_gen_init_ec;
static OSSL_FUNC_keymgmt_gen_init_fn keymgmt_gen_init_rsa;
static OSSL_FUNC_keymgmt_gen_set_params_fn keymgmt_gen_set_params;
static OSSL_FUNC_keymgmt_gen_settable_params_fn keymgmt_gen_settable_params;
static OSSL_FUNC_keymgmt_new_fn keymgmt_new;
static OSSL_FUNC_keymgmt_gen_fn keymgmt_gen;
static OSSL_FUNC_keymgmt_gen_cleanup_fn keymgmt_gen_cleanup;
static OSSL_FUNC_keymgmt_load_fn keymgmt_load;
static OSSL_FUNC_keymgmt_free_fn keymgmt_free;
static OSSL_FUNC_keymgmt_get_params_fn keymgmt_get_params;
static OSSL_FUNC_keymgmt_gettable_params_fn keymgmt_gettable_params;
static OSSL_FUNC_keymgmt_has_fn keymgmt_has;

/******************************************************************************/

static const OSSL_DISPATCH p11_keymgmt_tbl_rsa[] = {
{OSSL_FUNC_KEYMGMT_FREE, (void (*)(void))keymgmt_free},
{OSSL_FUNC_KEYMGMT_GEN_INIT, (void (*)(void))keymgmt_gen_init_rsa},
{OSSL_FUNC_KEYMGMT_GEN_SET_PARAMS, (void (*)(void))keymgmt_gen_set_params},
{OSSL_FUNC_KEYMGMT_GEN_SETTABLE_PARAMS, (void (*)(void))keymgmt_gen_settable_params},
{OSSL_FUNC_KEYMGMT_NEW, (void (*)(void))keymgmt_new},
{OSSL_FUNC_KEYMGMT_GEN, (void (*)(void))keymgmt_gen},
{OSSL_FUNC_KEYMGMT_GEN_CLEANUP, (void (*)(void))keymgmt_gen_cleanup},
{OSSL_FUNC_KEYMGMT_LOAD, (void (*)(void))keymgmt_load},
{OSSL_FUNC_KEYMGMT_GET_PARAMS, (void (*)(void))keymgmt_get_params},
{OSSL_FUNC_KEYMGMT_GETTABLE_PARAMS, (void (*)(void))keymgmt_gettable_params},
{OSSL_FUNC_KEYMGMT_HAS, (void (*)(void))keymgmt_has},
{0, NULL}};

static const OSSL_DISPATCH p11_keymgmt_tbl_ec[] = {
{OSSL_FUNC_KEYMGMT_FREE, (void (*)(void))keymgmt_free},
{OSSL_FUNC_KEYMGMT_GEN_INIT, (void (*)(void))keymgmt_gen_init_ec},
{OSSL_FUNC_KEYMGMT_GEN_SET_PARAMS, (void (*)(void))keymgmt_gen_set_params},
{OSSL_FUNC_KEYMGMT_GEN_SETTABLE_PARAMS, (void (*)(void))keymgmt_gen_settable_params},
{OSSL_FUNC_KEYMGMT_NEW, (void (*)(void))keymgmt_new},
{OSSL_FUNC_KEYMGMT_GEN, (void (*)(void))keymgmt_gen},
{OSSL_FUNC_KEYMGMT_GEN_CLEANUP, (void (*)(void))keymgmt_gen_cleanup},
{OSSL_FUNC_KEYMGMT_LOAD, (void (*)(void))keymgmt_load},
{OSSL_FUNC_KEYMGMT_GET_PARAMS, (void (*)(void))keymgmt_get_params},
{OSSL_FUNC_KEYMGMT_GETTABLE_PARAMS, (void (*)(void))keymgmt_gettable_params},
{OSSL_FUNC_KEYMGMT_HAS, (void (*)(void))keymgmt_has},
{0, NULL}};

static const OSSL_ALGORITHM p11_dispatch_keymgmt[] = {
{"RSA:rsaEncryption", "provider=pkcs11", p11_keymgmt_tbl_rsa, "PKCS#11 RSA implementation"},
/*{"RSA-PSS:RSASSA-PSS", "provider=pkcs11", p11_keymgmt_tbl, "PKCS#11 RSA-PSS implementation"},*/
{"EC:id-ecPublicKey", "provider=pkcs11", p11_keymgmt_tbl_ec, "PKCS#11 EC implementation"},
{NULL, NULL, NULL, NULL}};

const OSSL_ALGORITHM* p11_get_ops_keymgmt(void* provctx, int* no_store)
{
    (void)no_store;

    PROVIDER_CTX* ctx = provctx;

    ctx_log(ctx, 3, "%s\n", __FUNCTION__);

    return !ctx->b_keymgmt_disabled ? p11_dispatch_keymgmt : NULL;
}

/******************************************************************************/
#define FREE(x)          \
    if (x)               \
    {                    \
        OPENSSL_free(x); \
    }

static void __free_p11_keymgmtctx(P11_KEYMGMT_CTX* ctx)
{
    if (ctx)
    {
        FREE(ctx->point_conversion)
        FREE(ctx->group_name)
        FREE(ctx->encoding)
        FREE(ctx->uri);
        FREE(ctx)
    }
}

static P11_KEYMGMT_CTX* __new_p11_keymgmtctx()
{
    P11_KEYMGMT_CTX* ctx = OPENSSL_zalloc(sizeof(*ctx));

    if (!ctx)
    {
        goto err;
    }

    ctx->encoding = OPENSSL_zalloc(STRING_BUF_LEN);
    ctx->group_name = OPENSSL_zalloc(STRING_BUF_LEN);
    ctx->point_conversion = OPENSSL_zalloc(STRING_BUF_LEN);
    ctx->uri = OPENSSL_zalloc(MAX_URI_LENGTH + 1);
    ctx->label = OPENSSL_zalloc(STRING_BUF_LEN);
    ctx->id = OPENSSL_zalloc(STRING_BUF_LEN);
    ctx->id_len = 0;

    if (!ctx->encoding || !ctx->group_name || !ctx->point_conversion || !ctx->label || !ctx->id)
    {
        goto err;
    }

    return ctx;

err:
    __free_p11_keymgmtctx(ctx);

    return NULL;
}

/******************************************************************************/

static void* keymgmt_gen_init(void* provctx, int selection, const OSSL_PARAM params[], long algorithm)
{
    P11_KEYMGMT_CTX* ctx = NULL;

    ctx = __new_p11_keymgmtctx();
    if (!ctx)
    {
        goto err;
    }

    ctx->provctx = provctx;
    ctx->slot = ctx->provctx->slot;
    ctx->algorithm = algorithm;
    ctx->nbits = 2048;
    // ctx->type = RSA; // RSA, RSA-PSS, EC, ...

    ctx_log(ctx->provctx, 3, "%s\n", __FUNCTION__);
    ctx_log(ctx->provctx, 2, "selection=%04x\n", selection);

    if ((selection & OSSL_KEYMGMT_SELECT_KEYPAIR) == 0)
        goto err;

    if (!ctx->slot)
    {
        /* Look for a slot */
        ctx->slot = PKCS11_find_token(ctx->provctx->pkcs11_ctx, ctx->provctx->slot_list, ctx->provctx->slot_count);
        if (ctx->slot == NULL || ctx->slot->token == NULL)
        {
            goto err;
        }
    }

    /* We need RW session, since openssl pumps the key from outside */
    if (PKCS11_open_session(ctx->slot, 1))
    {
        printf("Could not open session\n");
        goto err;
    }

    if (!keymgmt_gen_set_params(ctx, params))
        goto err;

    return ctx;

err:
    if (ctx)
    {
        __free_p11_keymgmtctx(ctx);
    }

    return NULL;
}

static void* keymgmt_gen_init_rsa(void* provctx, int selection, const OSSL_PARAM params[])
{
    return keymgmt_gen_init(provctx, selection, params, CKK_RSA);
}

static void* keymgmt_gen_init_ec(void* provctx, int selection, const OSSL_PARAM params[])
{
    return keymgmt_gen_init(provctx, selection, params, CKK_EC);
}

#define ALLOC(ptr, len, max)       \
    {                              \
        if (ptr)                   \
        {                          \
            OPENSSL_free(ptr);     \
        }                          \
        ptr = OPENSSL_zalloc(max); \
        len = max;                 \
    }

static int process_uri(P11_KEYMGMT_CTX* ctx)
{
    PKCS11_TOKEN* temp_token = NULL;
    int rv = 0;

    if (!ctx)
        goto err;

    if (!ctx->uri)
    {
        time_t now;
        char* p;
        int i;
        char skip;
        static const char* hexdigits = "01234567890ABCDEFabcdef";
        time(&now);

        ctx->uri = OPENSSL_zalloc(64);
        if (!ctx->uri)
            goto err;

        strcpy(ctx->uri, "pkcs11:token=libp11;id=");
        p = ctx->uri + strlen(ctx->uri);
        for (skip = 1, i = 56; i >= 0; i -= 8)
        {
            char c = (now >> i) & 0xff;
            if (skip)
            {
                if (c == 0)
                {
                    continue;
                }

                skip = 0;
            }

            *p++ = '%';
            *p++ = hexdigits[(c >> 4) & 0xf];
            *p++ = hexdigits[c & 0xf];
            skip = 0;
        }
        *p = 0;

        ctx_log(ctx->provctx, 0, "%s pkcs11 uri not defined, using arbitrary [%s]\n", __FUNCTION__, ctx->uri);
    }

    ALLOC(ctx->id, ctx->id_len, STRING_BUF_LEN + 1)
    ALLOC(ctx->provctx->pin, ctx->provctx->pin_length, MAX_PIN_LENGTH + 1)
    if (!ctx->id || !ctx->provctx->pin)
    {
        goto err;
    }

    if (parse_pkcs11_uri(ctx->provctx, ctx->uri, &temp_token, (unsigned char**)&ctx->id, &ctx->id_len, &ctx->provctx->pin, &ctx->provctx->pin_length, &ctx->label))
    {
        /* only id, label and pin are used from the uri, other parsed parts dropped */

        if (!ctx->label && temp_token->label)
        {
            OPENSSL_free(ctx->label);
            ctx->label = temp_token->label;
            temp_token->label = NULL;
        }
    }
    else
    {
        goto err;
    }

    rv = 1;

err:
    if (temp_token)
    {
        pkcs11_destroy_token(temp_token);
        OPENSSL_free(temp_token);
    }

    return rv;
}

static void* keymgmt_new(void* provctx)
{
    PROVIDER_CTX* ctx = provctx;

    ctx_log(ctx, 3, "%s\n", __FUNCTION__);

    return NULL;
}

static void* keymgmt_gen(void* genctx, OSSL_CALLBACK* cb, void* cbarg)
{
    P11_KEYMGMT_CTX* ctx = genctx;
    EVP_PKEY* pkey;
    int rv;

    /* TODO: handle following parameters */
    (void)cb;
    (void)cbarg;

    ctx_log(ctx->provctx, 3, "%s\n", __FUNCTION__);

    /* get ID and label from uri. Only here, since OpenSSL calls set_params multiple times. */
    process_uri(ctx);

    /* ID is binary here */
    switch (ctx->algorithm)
    {
        case CKK_RSA:
            rv = pkcs11_generate_key(PRIVSLOT(ctx->slot), ctx->algorithm, ctx->nbits, ctx->label, (unsigned char*)ctx->id, ctx->id_len);
            break;

        case CKK_EC:
            rv = pkcs11_generate_ec_key(PRIVSLOT(ctx->slot), ctx->group_name, ctx->label, (unsigned char*)ctx->id, ctx->id_len);
            break;

        default:
            rv = 1;
    }

    if (rv)
    {
        return NULL;
    }

    /* ID is PKCS11 URI here. */
    pkey = ctx_load_privkey(ctx->provctx, ctx->uri, NULL, NULL);

    return pkey;
}

static void keymgmt_gen_cleanup(void* genctx)
{
    P11_KEYMGMT_CTX* ctx = genctx;

    ctx_log(ctx->provctx, 3, "%s\n", __FUNCTION__);

    __free_p11_keymgmtctx(ctx);
}

static void keymgmt_free(void* keydata)
{
    /* TODO: implement or remove from available function list */
    (void)keydata;

    // fprintf(stderr, "%s\n", __FUNCTION__);
}

static int keymgmt_has(const void* keydata, int selection)
{
    /* TODO: implement or remove from available function list */
    (void)keydata;
    (void)selection;

    // fprintf(stderr, "%s\n", __FUNCTION__);

    return 0;
}

static int keymgmt_get_params(void* keydata, OSSL_PARAM params[])
{
    /* TODO: implement or remove from available function list */
    (void)keydata;
    (void)params;

    // fprintf(stderr, "%s\n", __FUNCTION__);

    return 0;
}

static const OSSL_PARAM* keymgmt_gettable_params(void* provctx)
{
    PROVIDER_CTX* ctx = provctx;

    ctx_log(ctx, 3, "%s\n", __FUNCTION__);

    return NULL;
}

#define P11_PKEY_PARAM_PIN_CODE "pass"
#define P11_PKEY_PARAM_URI "uri"

static const OSSL_PARAM keymgmt_gen_settable_params_tbl_rsa[] = {
// RSA
OSSL_PARAM_size_t(OSSL_PKEY_PARAM_RSA_BITS, NULL),
// OSSL_PARAM_size_t(OSSL_PKEY_PARAM_RSA_PRIMES, NULL),
// OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_E, NULL, 0),

// RSA-PSS
// OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_RSA_DIGEST, NULL, 0),
// OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_RSA_DIGEST_PROPS, NULL, 0),
// OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_RSA_MASKGENFUNC, NULL, 0),
// OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_RSA_MGF1_DIGEST, NULL, 0),
// OSSL_PARAM_int(OSSL_PKEY_PARAM_RSA_PSS_SALTLEN, NULL),

// common
OSSL_PARAM_utf8_string(P11_PKEY_PARAM_PIN_CODE, NULL, 0),
OSSL_PARAM_utf8_string(P11_PKEY_PARAM_URI, NULL, 0),
OSSL_PARAM_END};

static const OSSL_PARAM keymgmt_gen_settable_params_tbl_ec[] = {
// EC
OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, NULL, 0),
OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_EC_ENCODING, NULL, 0),
OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_EC_POINT_CONVERSION_FORMAT, NULL, 0),

// common
OSSL_PARAM_utf8_string(P11_PKEY_PARAM_PIN_CODE, NULL, 0),
OSSL_PARAM_utf8_string(P11_PKEY_PARAM_URI, NULL, 0),
OSSL_PARAM_END};

static const OSSL_PARAM* keymgmt_gen_settable_params(void* genctx, void* provctx)
{
    (void)provctx;
    P11_KEYMGMT_CTX* ctx = genctx;

    ctx_log(ctx->provctx, 3, "%s\n", __FUNCTION__);

    switch (ctx->algorithm)
    {
        case CKK_RSA:
            return keymgmt_gen_settable_params_tbl_rsa;
            break;

        case CKK_EC:
            return keymgmt_gen_settable_params_tbl_ec;
            break;

        default:
            break;
    }

    return NULL;
}

static int keymgmt_gen_set_params(void* genctx, const OSSL_PARAM params[])
{
    P11_KEYMGMT_CTX* ctx = genctx;
    const OSSL_PARAM* p;

    ctx_log(ctx->provctx, 3, "%s\n", __FUNCTION__);

    if (params == NULL)
        return 1;

    if ((p = OSSL_PARAM_locate_const(params, P11_PKEY_PARAM_URI)) != NULL)
    {
        if (!OSSL_PARAM_get_utf8_string(p, &ctx->uri, MAX_URI_LENGTH))
        {
            goto err;
        }

        ctx_log(ctx->provctx, 2, "param: P11_PKEY_PARAM_URI=%s\n", ctx->uri);
    }

    if ((p = OSSL_PARAM_locate_const(params, P11_PKEY_PARAM_PIN_CODE)) != NULL)
    {
        if (!OSSL_PARAM_get_utf8_string(p, &ctx->provctx->pin, MAX_PIN_LENGTH))
        {
            goto err;
        }

        ctx_log(ctx->provctx, 2, "param: P11_PKEY_PARAM_PIN_CODE=%s\n", ctx->provctx->pin);
    }

    if (ctx->algorithm == CKK_RSA)
    {
        if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_RSA_BITS)) != NULL)
        {
            if (!OSSL_PARAM_get_size_t(p, &ctx->nbits))
                goto err;

            ctx_log(ctx->provctx, 2, "param: OSSL_PKEY_PARAM_RSA_BITS=%ld\n", ctx->nbits);

            if (ctx->nbits < RSA_MIN_MODULUS_BITS)
            {
                ctx_log(ctx->provctx, 0, "RSA key too small\n");
                goto err;
            }
        }
    }

    if (ctx->algorithm == CKK_EC)
    {
        if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_GROUP_NAME)) != NULL)
        {
            if (!OSSL_PARAM_get_utf8_string(p, &ctx->group_name, STRING_BUF_LEN))
                goto err;
        }

        if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_EC_ENCODING)) != NULL)
        {
            if (!OSSL_PARAM_get_utf8_string(p, &ctx->encoding, STRING_BUF_LEN))
                goto err;
        }

        if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_EC_POINT_CONVERSION_FORMAT)) != NULL)
        {
            if (!OSSL_PARAM_get_utf8_string(p, &ctx->point_conversion, STRING_BUF_LEN))
                goto err;
        }

        ctx_log(ctx->provctx, 2, "%s group:[%s] encoding:[%s] point_conversion:[%s] pin:[%s]\n",
                __FUNCTION__,
                ctx->group_name,
                ctx->encoding,
                ctx->point_conversion,
                ctx->provctx->pin);
    }

    if (ctx->slot->token->loginRequired && !ctx->provctx->pin)
    {
        ctx_log(ctx->provctx, 0, "%s login required, but no pin present\n", __FUNCTION__);
        goto err;
    }

    if (PKCS11_login(ctx->slot, 0, ctx->provctx->pin))
    {
        goto err;
    }

    return 1;

err:
    return 0;
}

static void* keymgmt_load(const void* reference, size_t reference_sz)
{
    EVP_PKEY* pkey;

    /* *reference is what we are passing in object_cb() as PARAM_REFERENCE */
    if (!reference || reference_sz != sizeof(pkey))
        return NULL;

    /* the contents of the reference is the address to our object */
    pkey = *(EVP_PKEY**)reference;
    /* we grabbed it, so we detach it */
    *(EVP_PKEY**)reference = NULL;

    return pkey;
}
