/*
 * Copyright 1995-2017 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_P11_ERR_H
# define HEADER_P11_ERR_H

# ifdef  __cplusplus
extern "C" {
# endif

/* BEGIN ERROR CODES */
/*
 * The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

int ERR_load_P11_strings(void);
void ERR_unload_P11_strings(void);
void ERR_P11_error(int function, int reason, char *file, int line);
# define P11err(f,r) ERR_P11_error((f),(r),__FILE__,__LINE__)
# define P11initerr(f,r) ERR_P11_init_error((f),(r),__FILE__,__LINE__)

/* Error codes for the P11 functions. */

/* Function codes. */
# define P11_F_PKCS11_CHANGE_PIN                          100
# define P11_F_PKCS11_CTX_LOAD                            101
# define P11_F_PKCS11_CTX_RELOAD                          102
# define P11_F_PKCS11_ECDH_DERIVE                         103
# define P11_F_PKCS11_GENERATE_RANDOM                     105
# define P11_F_PKCS11_INIT_PIN                            106
# define P11_F_PKCS11_LOGOUT                              107
# define P11_F_PKCS11_MECHANISM                           111
# define P11_F_PKCS11_SEED_RANDOM                         108
# define P11_F_PKCS11_STORE_KEY                           109
# define P11_F_PKCS11_VERIFY                              110

/* Reason codes. */
# define P11_R_KEYGEN_FAILED                              1030
# define P11_R_LOAD_MODULE_ERROR                          1025
# define P11_R_NOT_SUPPORTED                              1028
# define P11_R_NO_SESSION                                 1029
# define P11_R_UI_FAILED                                  1031
# define P11_R_UNSUPPORTED_PADDING_TYPE                   1026

# ifdef  __cplusplus
}
# endif
#endif
