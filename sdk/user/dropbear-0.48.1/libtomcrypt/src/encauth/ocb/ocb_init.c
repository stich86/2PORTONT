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

/**
   @file ocb_init.c
   OCB implementation, initialize state, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef OCB_MODE

static const struct {
    int           len;
    unsigned char poly_div[MAXBLOCKSIZE], 
                  poly_mul[MAXBLOCKSIZE];
} polys[] = {
{
    8,
    { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B }
}, {
    16, 
    { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43 },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87 }
}
};

/**
  Initialize an OCB context.
  @param ocb     [out] The destination of the OCB state
  @param cipher  The index of the desired cipher
  @param key     The secret key
  @param keylen  The length of the secret key (octets)
  @param nonce   The session nonce (length of the block size of the cipher)
  @return CRYPT_OK if successful
*/
int ocb_init(ocb_state *ocb, int cipher, 
             const unsigned char *key, unsigned long keylen, const unsigned char *nonce)
{
   int poly, x, y, m, err;

   LTC_ARGCHK(ocb   != NULL);
   LTC_ARGCHK(key   != NULL);
   LTC_ARGCHK(nonce != NULL);

   /* valid cipher? */
   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }

   /* determine which polys to use */
   ocb->block_len = cipher_descriptor[cipher].block_length;
   for (poly = 0; poly < (int)(sizeof(polys)/sizeof(polys[0])); poly++) {
       if (polys[poly].len == ocb->block_len) { 
          break;
       }
   }
   if (polys[poly].len != ocb->block_len) {
      return CRYPT_INVALID_ARG;
   }   

   /* schedule the key */
   if ((err = cipher_descriptor[cipher].setup(key, keylen, 0, &ocb->key)) != CRYPT_OK) {
      return err;
   }
 
   /* find L = E[0] */
   zeromem(ocb->L, ocb->block_len);
   cipher_descriptor[cipher].ecb_encrypt(ocb->L, ocb->L, &ocb->key);

   /* find R = E[N xor L] */
   for (x = 0; x < ocb->block_len; x++) {
       ocb->R[x] = ocb->L[x] ^ nonce[x];
   }
   cipher_descriptor[cipher].ecb_encrypt(ocb->R, ocb->R, &ocb->key);

   /* find Ls[i] = L << i for i == 0..31 */
   XMEMCPY(ocb->Ls[0], ocb->L, ocb->block_len);
   for (x = 1; x < 32; x++) {
       m = ocb->Ls[x-1][0] >> 7;
       for (y = 0; y < ocb->block_len-1; y++) {
           ocb->Ls[x][y] = ((ocb->Ls[x-1][y] << 1) | (ocb->Ls[x-1][y+1] >> 7)) & 255;
       }
       ocb->Ls[x][ocb->block_len-1] = (ocb->Ls[x-1][ocb->block_len-1] << 1) & 255;

       if (m == 1) {
          for (y = 0; y < ocb->block_len; y++) {
              ocb->Ls[x][y] ^= polys[poly].poly_mul[y];
          }
       }
    }

    /* find Lr = L / x */
    m = ocb->L[ocb->block_len-1] & 1;

    /* shift right */
    for (x = ocb->block_len - 1; x > 0; x--) {
        ocb->Lr[x] = ((ocb->L[x] >> 1) | (ocb->L[x-1] << 7)) & 255;
    }
    ocb->Lr[0] = ocb->L[0] >> 1;

    if (m == 1) {
       for (x = 0; x < ocb->block_len; x++) {
           ocb->Lr[x] ^= polys[poly].poly_div[x];
       }
    }

    /* set Li, checksum */
    zeromem(ocb->Li,       ocb->block_len);
    zeromem(ocb->checksum, ocb->block_len);

    /* set other params */
    ocb->block_index = 1;
    ocb->cipher      = cipher;

    return CRYPT_OK;
}

#endif

/* $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/src/encauth/ocb/ocb_init.c,v $ */
/* $Revision: 1.1 $ */
/* $Date: 2006/06/08 13:40:01 $ */
