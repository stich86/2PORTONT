/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.org
 */
#include "tomcrypt.h"

/**
  @file crypt_cipher_descriptor.c
  Stores the cipher descriptor table, Tom St Denis
*/

struct ltc_cipher_descriptor cipher_descriptor[TAB_SIZE] = {
{ NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
 };

LTC_MUTEX_GLOBAL(ltc_cipher_mutex);


/* $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/src/misc/crypt/crypt_cipher_descriptor.c,v $ */
/* $Revision: 1.1 $ */
/* $Date: 2006/06/08 13:44:28 $ */
