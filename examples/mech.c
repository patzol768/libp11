
/* libp11 example code: mech.c
 *
 * This examply simply connects to your smart card
 * and lists the supported PKCS11 mechanisms.
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#if !defined(_WIN32) || defined(__CYGWIN__)
#include <termios.h>
#endif
#include <libp11.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define STR(x) #x

struct mechanism_name
{
    CK_MECHANISM_TYPE type;
    char* name;
};

typedef struct mechanism_name MECHANISM_NAME;

static const MECHANISM_NAME mech_names[] = {
{CKM_RSA_PKCS_KEY_PAIR_GEN, STR(CKM_RSA_PKCS_KEY_PAIR_GEN)},
{CKM_RSA_PKCS, STR(CKM_RSA_PKCS)},
{CKM_RSA_9796, STR(CKM_RSA_9796)},
{CKM_RSA_X_509, STR(CKM_RSA_X_509)},
{CKM_MD2_RSA_PKCS, STR(CKM_MD2_RSA_PKCS)},
{CKM_MD5_RSA_PKCS, STR(CKM_MD5_RSA_PKCS)},
{CKM_SHA1_RSA_PKCS, STR(CKM_SHA1_RSA_PKCS)},
{CKM_RIPEMD128_RSA_PKCS, STR(CKM_RIPEMD128_RSA_PKCS)},
{CKM_RIPEMD160_RSA_PKCS, STR(CKM_RIPEMD160_RSA_PKCS)},
{CKM_RSA_PKCS_OAEP, STR(CKM_RSA_PKCS_OAEP)},
{CKM_RSA_X9_31_KEY_PAIR_GEN, STR(CKM_RSA_X9_31_KEY_PAIR_GEN)},
{CKM_RSA_X9_31, STR(CKM_RSA_X9_31)},
{CKM_SHA1_RSA_X9_31, STR(CKM_SHA1_RSA_X9_31)},
{CKM_RSA_PKCS_PSS, STR(CKM_RSA_PKCS_PSS)},
{CKM_SHA1_RSA_PKCS_PSS, STR(CKM_SHA1_RSA_PKCS_PSS)},
{CKM_DSA_KEY_PAIR_GEN, STR(CKM_DSA_KEY_PAIR_GEN)},
{CKM_DSA, STR(CKM_DSA)},
{CKM_DSA_SHA1, STR(CKM_DSA_SHA1)},
{CKM_DSA_SHA224, STR(CKM_DSA_SHA224)},
{CKM_DSA_SHA256, STR(CKM_DSA_SHA256)},
{CKM_DSA_SHA384, STR(CKM_DSA_SHA384)},
{CKM_DSA_SHA512, STR(CKM_DSA_SHA512)},
{CKM_DH_PKCS_KEY_PAIR_GEN, STR(CKM_DH_PKCS_KEY_PAIR_GEN)},
{CKM_DH_PKCS_DERIVE, STR(CKM_DH_PKCS_DERIVE)},
{CKM_X9_42_DH_KEY_PAIR_GEN, STR(CKM_X9_42_DH_KEY_PAIR_GEN)},
{CKM_X9_42_DH_DERIVE, STR(CKM_X9_42_DH_DERIVE)},
{CKM_X9_42_DH_HYBRID_DERIVE, STR(CKM_X9_42_DH_HYBRID_DERIVE)},
{CKM_X9_42_MQV_DERIVE, STR(CKM_X9_42_MQV_DERIVE)},
{CKM_SHA256_RSA_PKCS, STR(CKM_SHA256_RSA_PKCS)},
{CKM_SHA384_RSA_PKCS, STR(CKM_SHA384_RSA_PKCS)},
{CKM_SHA512_RSA_PKCS, STR(CKM_SHA512_RSA_PKCS)},
{CKM_SHA256_RSA_PKCS_PSS, STR(CKM_SHA256_RSA_PKCS_PSS)},
{CKM_SHA384_RSA_PKCS_PSS, STR(CKM_SHA384_RSA_PKCS_PSS)},
{CKM_SHA512_RSA_PKCS_PSS, STR(CKM_SHA512_RSA_PKCS_PSS)},
{CKM_SHA224_RSA_PKCS, STR(CKM_SHA224_RSA_PKCS)},
{CKM_SHA224_RSA_PKCS_PSS, STR(CKM_SHA224_RSA_PKCS_PSS)},
{CKM_RC2_KEY_GEN, STR(CKM_RC2_KEY_GEN)},
{CKM_RC2_ECB, STR(CKM_RC2_ECB)},
{CKM_RC2_CBC, STR(CKM_RC2_CBC)},
{CKM_RC2_MAC, STR(CKM_RC2_MAC)},
{CKM_RC2_MAC_GENERAL, STR(CKM_RC2_MAC_GENERAL)},
{CKM_RC2_CBC_PAD, STR(CKM_RC2_CBC_PAD)},
{CKM_RC4_KEY_GEN, STR(CKM_RC4_KEY_GEN)},
{CKM_RC4, STR(CKM_RC4)},
{CKM_DES_KEY_GEN, STR(CKM_DES_KEY_GEN)},
{CKM_DES_ECB, STR(CKM_DES_ECB)},
{CKM_DES_CBC, STR(CKM_DES_CBC)},
{CKM_DES_MAC, STR(CKM_DES_MAC)},
{CKM_DES_MAC_GENERAL, STR(CKM_DES_MAC_GENERAL)},
{CKM_DES_CBC_PAD, STR(CKM_DES_CBC_PAD)},
{CKM_DES2_KEY_GEN, STR(CKM_DES2_KEY_GEN)},
{CKM_DES3_KEY_GEN, STR(CKM_DES3_KEY_GEN)},
{CKM_DES3_ECB, STR(CKM_DES3_ECB)},
{CKM_DES3_CBC, STR(CKM_DES3_CBC)},
{CKM_DES3_MAC, STR(CKM_DES3_MAC)},
{CKM_DES3_MAC_GENERAL, STR(CKM_DES3_MAC_GENERAL)},
{CKM_DES3_CBC_PAD, STR(CKM_DES3_CBC_PAD)},
{CKM_DES3_CMAC, STR(CKM_DES3_CMAC)},
{CKM_CDMF_KEY_GEN, STR(CKM_CDMF_KEY_GEN)},
{CKM_CDMF_ECB, STR(CKM_CDMF_ECB)},
{CKM_CDMF_CBC, STR(CKM_CDMF_CBC)},
{CKM_CDMF_MAC, STR(CKM_CDMF_MAC)},
{CKM_CDMF_MAC_GENERAL, STR(CKM_CDMF_MAC_GENERAL)},
{CKM_CDMF_CBC_PAD, STR(CKM_CDMF_CBC_PAD)},
{CKM_MD2, STR(CKM_MD2)},
{CKM_MD2_HMAC, STR(CKM_MD2_HMAC)},
{CKM_MD2_HMAC_GENERAL, STR(CKM_MD2_HMAC_GENERAL)},
{CKM_MD5, STR(CKM_MD5)},
{CKM_MD5_HMAC, STR(CKM_MD5_HMAC)},
{CKM_MD5_HMAC_GENERAL, STR(CKM_MD5_HMAC_GENERAL)},
{CKM_SHA_1, STR(CKM_SHA_1)},
{CKM_SHA_1_HMAC, STR(CKM_SHA_1_HMAC)},
{CKM_SHA_1_HMAC_GENERAL, STR(CKM_SHA_1_HMAC_GENERAL)},
{CKM_RIPEMD128, STR(CKM_RIPEMD128)},
{CKM_RIPEMD128_HMAC, STR(CKM_RIPEMD128_HMAC)},
{CKM_RIPEMD128_HMAC_GENERAL, STR(CKM_RIPEMD128_HMAC_GENERAL)},
{CKM_RIPEMD160, STR(CKM_RIPEMD160)},
{CKM_RIPEMD160_HMAC, STR(CKM_RIPEMD160_HMAC)},
{CKM_RIPEMD160_HMAC_GENERAL, STR(CKM_RIPEMD160_HMAC_GENERAL)},
{CKM_SHA256, STR(CKM_SHA256)},
{CKM_SHA256_HMAC, STR(CKM_SHA256_HMAC)},
{CKM_SHA256_HMAC_GENERAL, STR(CKM_SHA256_HMAC_GENERAL)},
{CKM_SHA224, STR(CKM_SHA224)},
{CKM_SHA224_HMAC, STR(CKM_SHA224_HMAC)},
{CKM_SHA224_HMAC_GENERAL, STR(CKM_SHA224_HMAC_GENERAL)},
{CKM_SHA384, STR(CKM_SHA384)},
{CKM_SHA384_HMAC, STR(CKM_SHA384_HMAC)},
{CKM_SHA384_HMAC_GENERAL, STR(CKM_SHA384_HMAC_GENERAL)},
{CKM_SHA512, STR(CKM_SHA512)},
{CKM_SHA512_HMAC, STR(CKM_SHA512_HMAC)},
{CKM_SHA512_HMAC_GENERAL, STR(CKM_SHA512_HMAC_GENERAL)},
{CKM_SHA3_256, STR(CKM_SHA3_256)},
{CKM_SHA3_256_HMAC, STR(CKM_SHA3_256_HMAC)},
{CKM_SHA3_256_HMAC_GENERAL, STR(CKM_SHA3_256_HMAC_GENERAL)},
{CKM_SHA3_256_KEY_GEN, STR(CKM_SHA3_256_KEY_GEN)},
{CKM_SHA3_224, STR(CKM_SHA3_224)},
{CKM_SHA3_224_HMAC, STR(CKM_SHA3_224_HMAC)},
{CKM_SHA3_224_HMAC_GENERAL, STR(CKM_SHA3_224_HMAC_GENERAL)},
{CKM_SHA3_224_KEY_GEN, STR(CKM_SHA3_224_KEY_GEN)},
{CKM_SHA3_384, STR(CKM_SHA3_384)},
{CKM_SHA3_384_HMAC, STR(CKM_SHA3_384_HMAC)},
{CKM_SHA3_384_HMAC_GENERAL, STR(CKM_SHA3_384_HMAC_GENERAL)},
{CKM_SHA3_384_KEY_GEN, STR(CKM_SHA3_384_KEY_GEN)},
{CKM_SHA3_512, STR(CKM_SHA3_512)},
{CKM_SHA3_512_HMAC, STR(CKM_SHA3_512_HMAC)},
{CKM_SHA3_512_HMAC_GENERAL, STR(CKM_SHA3_512_HMAC_GENERAL)},
{CKM_SHA3_512_KEY_GEN, STR(CKM_SHA3_512_KEY_GEN)},
{CKM_CAST_KEY_GEN, STR(CKM_CAST_KEY_GEN)},
{CKM_CAST_ECB, STR(CKM_CAST_ECB)},
{CKM_CAST_CBC, STR(CKM_CAST_CBC)},
{CKM_CAST_MAC, STR(CKM_CAST_MAC)},
{CKM_CAST_MAC_GENERAL, STR(CKM_CAST_MAC_GENERAL)},
{CKM_CAST_CBC_PAD, STR(CKM_CAST_CBC_PAD)},
{CKM_CAST3_KEY_GEN, STR(CKM_CAST3_KEY_GEN)},
{CKM_CAST3_ECB, STR(CKM_CAST3_ECB)},
{CKM_CAST3_CBC, STR(CKM_CAST3_CBC)},
{CKM_CAST3_MAC, STR(CKM_CAST3_MAC)},
{CKM_CAST3_MAC_GENERAL, STR(CKM_CAST3_MAC_GENERAL)},
{CKM_CAST3_CBC_PAD, STR(CKM_CAST3_CBC_PAD)},
{CKM_CAST5_KEY_GEN, STR(CKM_CAST5_KEY_GEN)},
{CKM_CAST128_KEY_GEN, STR(CKM_CAST128_KEY_GEN)},
{CKM_CAST5_ECB, STR(CKM_CAST5_ECB)},
{CKM_CAST128_ECB, STR(CKM_CAST128_ECB)},
{CKM_CAST5_CBC, STR(CKM_CAST5_CBC)},
{CKM_CAST128_CBC, STR(CKM_CAST128_CBC)},
{CKM_CAST5_MAC, STR(CKM_CAST5_MAC)},
{CKM_CAST128_MAC, STR(CKM_CAST128_MAC)},
{CKM_CAST5_MAC_GENERAL, STR(CKM_CAST5_MAC_GENERAL)},
{CKM_CAST128_MAC_GENERAL, STR(CKM_CAST128_MAC_GENERAL)},
{CKM_CAST5_CBC_PAD, STR(CKM_CAST5_CBC_PAD)},
{CKM_CAST128_CBC_PAD, STR(CKM_CAST128_CBC_PAD)},
{CKM_RC5_KEY_GEN, STR(CKM_RC5_KEY_GEN)},
{CKM_RC5_ECB, STR(CKM_RC5_ECB)},
{CKM_RC5_CBC, STR(CKM_RC5_CBC)},
{CKM_RC5_MAC, STR(CKM_RC5_MAC)},
{CKM_RC5_MAC_GENERAL, STR(CKM_RC5_MAC_GENERAL)},
{CKM_RC5_CBC_PAD, STR(CKM_RC5_CBC_PAD)},
{CKM_IDEA_KEY_GEN, STR(CKM_IDEA_KEY_GEN)},
{CKM_IDEA_ECB, STR(CKM_IDEA_ECB)},
{CKM_IDEA_CBC, STR(CKM_IDEA_CBC)},
{CKM_IDEA_MAC, STR(CKM_IDEA_MAC)},
{CKM_IDEA_MAC_GENERAL, STR(CKM_IDEA_MAC_GENERAL)},
{CKM_IDEA_CBC_PAD, STR(CKM_IDEA_CBC_PAD)},
{CKM_GENERIC_SECRET_KEY_GEN, STR(CKM_GENERIC_SECRET_KEY_GEN)},
{CKM_CONCATENATE_BASE_AND_KEY, STR(CKM_CONCATENATE_BASE_AND_KEY)},
{CKM_CONCATENATE_BASE_AND_DATA, STR(CKM_CONCATENATE_BASE_AND_DATA)},
{CKM_CONCATENATE_DATA_AND_BASE, STR(CKM_CONCATENATE_DATA_AND_BASE)},
{CKM_XOR_BASE_AND_DATA, STR(CKM_XOR_BASE_AND_DATA)},
{CKM_EXTRACT_KEY_FROM_KEY, STR(CKM_EXTRACT_KEY_FROM_KEY)},
{CKM_SSL3_PRE_MASTER_KEY_GEN, STR(CKM_SSL3_PRE_MASTER_KEY_GEN)},
{CKM_SSL3_MASTER_KEY_DERIVE, STR(CKM_SSL3_MASTER_KEY_DERIVE)},
{CKM_SSL3_KEY_AND_MAC_DERIVE, STR(CKM_SSL3_KEY_AND_MAC_DERIVE)},
{CKM_SSL3_MASTER_KEY_DERIVE_DH, STR(CKM_SSL3_MASTER_KEY_DERIVE_DH)},
{CKM_TLS_PRE_MASTER_KEY_GEN, STR(CKM_TLS_PRE_MASTER_KEY_GEN)},
{CKM_TLS_MASTER_KEY_DERIVE, STR(CKM_TLS_MASTER_KEY_DERIVE)},
{CKM_TLS_KEY_AND_MAC_DERIVE, STR(CKM_TLS_KEY_AND_MAC_DERIVE)},
{CKM_TLS_MASTER_KEY_DERIVE_DH, STR(CKM_TLS_MASTER_KEY_DERIVE_DH)},
{CKM_SSL3_MD5_MAC, STR(CKM_SSL3_MD5_MAC)},
{CKM_SSL3_SHA1_MAC, STR(CKM_SSL3_SHA1_MAC)},
{CKM_MD5_KEY_DERIVATION, STR(CKM_MD5_KEY_DERIVATION)},
{CKM_MD2_KEY_DERIVATION, STR(CKM_MD2_KEY_DERIVATION)},
{CKM_SHA1_KEY_DERIVATION, STR(CKM_SHA1_KEY_DERIVATION)},
{CKM_PBE_MD2_DES_CBC, STR(CKM_PBE_MD2_DES_CBC)},
{CKM_PBE_MD5_DES_CBC, STR(CKM_PBE_MD5_DES_CBC)},
{CKM_PBE_MD5_CAST_CBC, STR(CKM_PBE_MD5_CAST_CBC)},
{CKM_PBE_MD5_CAST3_CBC, STR(CKM_PBE_MD5_CAST3_CBC)},
{CKM_PBE_MD5_CAST5_CBC, STR(CKM_PBE_MD5_CAST5_CBC)},
{CKM_PBE_MD5_CAST128_CBC, STR(CKM_PBE_MD5_CAST128_CBC)},
{CKM_PBE_SHA1_CAST5_CBC, STR(CKM_PBE_SHA1_CAST5_CBC)},
{CKM_PBE_SHA1_CAST128_CBC, STR(CKM_PBE_SHA1_CAST128_CBC)},
{CKM_PBE_SHA1_RC4_128, STR(CKM_PBE_SHA1_RC4_128)},
{CKM_PBE_SHA1_RC4_40, STR(CKM_PBE_SHA1_RC4_40)},
{CKM_PBE_SHA1_DES3_EDE_CBC, STR(CKM_PBE_SHA1_DES3_EDE_CBC)},
{CKM_PBE_SHA1_DES2_EDE_CBC, STR(CKM_PBE_SHA1_DES2_EDE_CBC)},
{CKM_PBE_SHA1_RC2_128_CBC, STR(CKM_PBE_SHA1_RC2_128_CBC)},
{CKM_PBE_SHA1_RC2_40_CBC, STR(CKM_PBE_SHA1_RC2_40_CBC)},
{CKM_PKCS5_PBKD2, STR(CKM_PKCS5_PBKD2)},
{CKM_PBA_SHA1_WITH_SHA1_HMAC, STR(CKM_PBA_SHA1_WITH_SHA1_HMAC)},
{CKM_KEY_WRAP_LYNKS, STR(CKM_KEY_WRAP_LYNKS)},
{CKM_KEY_WRAP_SET_OAEP, STR(CKM_KEY_WRAP_SET_OAEP)},
{CKM_SKIPJACK_KEY_GEN, STR(CKM_SKIPJACK_KEY_GEN)},
{CKM_SKIPJACK_ECB64, STR(CKM_SKIPJACK_ECB64)},
{CKM_SKIPJACK_CBC64, STR(CKM_SKIPJACK_CBC64)},
{CKM_SKIPJACK_OFB64, STR(CKM_SKIPJACK_OFB64)},
{CKM_SKIPJACK_CFB64, STR(CKM_SKIPJACK_CFB64)},
{CKM_SKIPJACK_CFB32, STR(CKM_SKIPJACK_CFB32)},
{CKM_SKIPJACK_CFB16, STR(CKM_SKIPJACK_CFB16)},
{CKM_SKIPJACK_CFB8, STR(CKM_SKIPJACK_CFB8)},
{CKM_SKIPJACK_WRAP, STR(CKM_SKIPJACK_WRAP)},
{CKM_SKIPJACK_PRIVATE_WRAP, STR(CKM_SKIPJACK_PRIVATE_WRAP)},
{CKM_SKIPJACK_RELAYX, STR(CKM_SKIPJACK_RELAYX)},
{CKM_KEA_KEY_PAIR_GEN, STR(CKM_KEA_KEY_PAIR_GEN)},
{CKM_KEA_KEY_DERIVE, STR(CKM_KEA_KEY_DERIVE)},
{CKM_FORTEZZA_TIMESTAMP, STR(CKM_FORTEZZA_TIMESTAMP)},
{CKM_BATON_KEY_GEN, STR(CKM_BATON_KEY_GEN)},
{CKM_BATON_ECB128, STR(CKM_BATON_ECB128)},
{CKM_BATON_ECB96, STR(CKM_BATON_ECB96)},
{CKM_BATON_CBC128, STR(CKM_BATON_CBC128)},
{CKM_BATON_COUNTER, STR(CKM_BATON_COUNTER)},
{CKM_BATON_SHUFFLE, STR(CKM_BATON_SHUFFLE)},
{CKM_BATON_WRAP, STR(CKM_BATON_WRAP)},
{CKM_ECDSA_KEY_PAIR_GEN, STR(CKM_ECDSA_KEY_PAIR_GEN)},
{CKM_EC_KEY_PAIR_GEN, STR(CKM_EC_KEY_PAIR_GEN)},
{CKM_ECDSA, STR(CKM_ECDSA)},
{CKM_ECDSA_SHA1, STR(CKM_ECDSA_SHA1)},
{CKM_ECDSA_SHA224, STR(CKM_ECDSA_SHA224)},
{CKM_ECDSA_SHA256, STR(CKM_ECDSA_SHA256)},
{CKM_ECDSA_SHA384, STR(CKM_ECDSA_SHA384)},
{CKM_ECDSA_SHA512, STR(CKM_ECDSA_SHA512)},
{CKM_ECDH1_DERIVE, STR(CKM_ECDH1_DERIVE)},
{CKM_ECDH1_COFACTOR_DERIVE, STR(CKM_ECDH1_COFACTOR_DERIVE)},
{CKM_ECMQV_DERIVE, STR(CKM_ECMQV_DERIVE)},
{CKM_JUNIPER_KEY_GEN, STR(CKM_JUNIPER_KEY_GEN)},
{CKM_JUNIPER_ECB128, STR(CKM_JUNIPER_ECB128)},
{CKM_JUNIPER_CBC128, STR(CKM_JUNIPER_CBC128)},
{CKM_JUNIPER_COUNTER, STR(CKM_JUNIPER_COUNTER)},
{CKM_JUNIPER_SHUFFLE, STR(CKM_JUNIPER_SHUFFLE)},
{CKM_JUNIPER_WRAP, STR(CKM_JUNIPER_WRAP)},
{CKM_FASTHASH, STR(CKM_FASTHASH)},
{CKM_AES_KEY_GEN, STR(CKM_AES_KEY_GEN)},
{CKM_AES_ECB, STR(CKM_AES_ECB)},
{CKM_AES_CBC, STR(CKM_AES_CBC)},
{CKM_AES_MAC, STR(CKM_AES_MAC)},
{CKM_AES_MAC_GENERAL, STR(CKM_AES_MAC_GENERAL)},
{CKM_AES_CBC_PAD, STR(CKM_AES_CBC_PAD)},
{CKM_AES_CTR, STR(CKM_AES_CTR)},
{CKM_AES_GCM, STR(CKM_AES_GCM)},
{CKM_AES_CCM, STR(CKM_AES_CCM)},
{CKM_AES_CTS, STR(CKM_AES_CTS)},
{CKM_AES_CMAC, STR(CKM_AES_CMAC)},
{CKM_BLOWFISH_KEY_GEN, STR(CKM_BLOWFISH_KEY_GEN)},
{CKM_BLOWFISH_CBC, STR(CKM_BLOWFISH_CBC)},
{CKM_TWOFISH_KEY_GEN, STR(CKM_TWOFISH_KEY_GEN)},
{CKM_TWOFISH_CBC, STR(CKM_TWOFISH_CBC)},
{CKM_DES_ECB_ENCRYPT_DATA, STR(CKM_DES_ECB_ENCRYPT_DATA)},
{CKM_DES_CBC_ENCRYPT_DATA, STR(CKM_DES_CBC_ENCRYPT_DATA)},
{CKM_DES3_ECB_ENCRYPT_DATA, STR(CKM_DES3_ECB_ENCRYPT_DATA)},
{CKM_DES3_CBC_ENCRYPT_DATA, STR(CKM_DES3_CBC_ENCRYPT_DATA)},
{CKM_AES_ECB_ENCRYPT_DATA, STR(CKM_AES_ECB_ENCRYPT_DATA)},
{CKM_AES_CBC_ENCRYPT_DATA, STR(CKM_AES_CBC_ENCRYPT_DATA)},
{CKM_GOSTR3410_KEY_PAIR_GEN, STR(CKM_GOSTR3410_KEY_PAIR_GEN)},
{CKM_GOSTR3410, STR(CKM_GOSTR3410)},
{CKM_GOSTR3410_WITH_GOSTR3411, STR(CKM_GOSTR3410_WITH_GOSTR3411)},
{CKM_GOSTR3410_KEY_WRAP, STR(CKM_GOSTR3410_KEY_WRAP)},
{CKM_GOSTR3410_DERIVE, STR(CKM_GOSTR3410_DERIVE)},
{CKM_GOSTR3410_512_KEY_PAIR_GEN, STR(CKM_GOSTR3410_512_KEY_PAIR_GEN)},
{CKM_GOSTR3410_512, STR(CKM_GOSTR3410_512)},
{CKM_GOSTR3410_12_DERIVE, STR(CKM_GOSTR3410_12_DERIVE)},
{CKM_GOSTR3410_WITH_GOSTR3411_12_256, STR(CKM_GOSTR3410_WITH_GOSTR3411_12_256)},
{CKM_GOSTR3410_WITH_GOSTR3411_12_512, STR(CKM_GOSTR3410_WITH_GOSTR3411_12_512)},
{CKM_GOSTR3411, STR(CKM_GOSTR3411)},
{CKM_GOSTR3411_HMAC, STR(CKM_GOSTR3411_HMAC)},
{CKM_GOSTR3411_12_256, STR(CKM_GOSTR3411_12_256)},
{CKM_GOSTR3411_12_512, STR(CKM_GOSTR3411_12_512)},
{CKM_GOSTR3411_12_256_HMAC, STR(CKM_GOSTR3411_12_256_HMAC)},
{CKM_GOSTR3411_12_512_HMAC, STR(CKM_GOSTR3411_12_512_HMAC)},
{CKM_GOST28147_KEY_GEN, STR(CKM_GOST28147_KEY_GEN)},
{CKM_GOST28147_ECB, STR(CKM_GOST28147_ECB)},
{CKM_GOST28147, STR(CKM_GOST28147)},
{CKM_GOST28147_MAC, STR(CKM_GOST28147_MAC)},
{CKM_GOST28147_KEY_WRAP, STR(CKM_GOST28147_KEY_WRAP)},
{CKM_DSA_PARAMETER_GEN, STR(CKM_DSA_PARAMETER_GEN)},
{CKM_DH_PKCS_PARAMETER_GEN, STR(CKM_DH_PKCS_PARAMETER_GEN)},
{CKM_X9_42_DH_PARAMETER_GEN, STR(CKM_X9_42_DH_PARAMETER_GEN)},
{CKM_AES_KEY_WRAP, STR(CKM_AES_KEY_WRAP)},
{CKM_VENDOR_DEFINED, STR(CKM_VENDOR_DEFINED)},
{0xFFFFFFFF, "UNKNOWN / VENDOR_DEFINED"}};

char* find_mechanism_name(CK_MECHANISM_TYPE type)
{
    const MECHANISM_NAME* p = mech_names;
    while (p->type != type && p->type != 0xFFFFFFFF)
        p++;

    return p->name;
}

int main(int argc, char* argv[])
{
    PKCS11_CTX* ctx;
    PKCS11_SLOT *slots, *slot;

    PKCS11_MECHANISM* mechanisms;
    unsigned long mechcountp;

    int rc;
    unsigned int nslots;

    if (argc < 2)
    {
        fprintf(stderr, "usage: mech /usr/lib/opensc-pkcs11.so\n");
        return 1;
    }

    ctx = PKCS11_CTX_new();

    /* load pkcs #11 module */
    rc = PKCS11_CTX_load(ctx, argv[1]);
    if (rc)
    {
        fprintf(stderr, "loading pkcs11 engine failed: %s\n",
                ERR_reason_error_string(ERR_get_error()));
        rc = 1;
        goto nolib;
    }

    /* get information on all slots */
    rc = PKCS11_enumerate_slots(ctx, &slots, &nslots);
    if (rc < 0)
    {
        fprintf(stderr, "no slots available\n");
        rc = 2;
        goto noslots;
    }

    /* get first slot with a token */
    slot = PKCS11_find_token(ctx, slots, nslots);
    if (slot == NULL || slot->token == NULL)
    {
        fprintf(stderr, "no token available\n");
        rc = 3;
        goto notoken;
    }
    printf("Slot manufacturer........: %s\n", slot->manufacturer);
    printf("Slot description.........: %s\n", slot->description);
    printf("Slot token label.........: %s\n", slot->token->label);
    printf("Slot token manufacturer..: %s\n", slot->token->manufacturer);
    printf("Slot token model.........: %s\n", slot->token->model);
    printf("Slot token serialnr......: %s\n", slot->token->serialnr);

    /* read mechanisms of the slot */
    rc = PKCS11_enumerate_slot_mechanisms(ctx, PKCS11_get_slotid_from_slot(slot), &mechanisms, &mechcountp);
    if (rc)
    {
        fprintf(stderr, "mechanism list not available for this slot\n");
        rc = 13;
        goto failed;
    }

    printf("Number of mechanisms.....: %lu\n", mechcountp);

#define FLAG_PRINT(flag)                       \
    if (mechanisms[i].info.flags & CKF_##flag) \
    {                                          \
        if (printedflags)                      \
            printf(", ");                      \
        printf("CKF_" #flag);                  \
        printedflags++;                        \
    }

    for (unsigned int i = 0; i < mechcountp; i++)
    {
        int printedflags = 0;

        printf("%3d. Mechanism...........: %s\n", i, find_mechanism_name(mechanisms[i].type));
        printf("     Flags...............: ");
        FLAG_PRINT(HW);
        FLAG_PRINT(ENCRYPT);
        FLAG_PRINT(DECRYPT);
        FLAG_PRINT(DIGEST);
        FLAG_PRINT(SIGN);
        FLAG_PRINT(SIGN_RECOVER);
        FLAG_PRINT(VERIFY);
        FLAG_PRINT(VERIFY_RECOVER);
        FLAG_PRINT(GENERATE);
        FLAG_PRINT(GENERATE_KEY_PAIR);
        FLAG_PRINT(WRAP);
        FLAG_PRINT(UNWRAP);
        FLAG_PRINT(DERIVE);
        FLAG_PRINT(EXTENSION);

        FLAG_PRINT(EC_F_P);
        FLAG_PRINT(EC_F_2M);
        FLAG_PRINT(EC_ECPARAMETERS);
        FLAG_PRINT(EC_NAMEDCURVE);
        FLAG_PRINT(EC_UNCOMPRESS);
        FLAG_PRINT(EC_COMPRESS);
        printf("\n");

        printf("     Max key size........: %lu\n", mechanisms[i].info.ulMaxKeySize);
        printf("     Min key size........: %lu\n", mechanisms[i].info.ulMinKeySize);
    }

#undef FLAG_PRINT

    rc = 0;

failed:
    if (rc)
        ERR_print_errors_fp(stderr);

notoken:
    PKCS11_release_all_slots(ctx, slots, nslots);

noslots:
    PKCS11_CTX_unload(ctx);

nolib:
    PKCS11_CTX_free(ctx);

    if (rc)
        printf("mechanism listing failed.\n");
    else
        printf("mechanism listing successful.\n");
    return rc;
}

/* vim: set noexpandtab: */
