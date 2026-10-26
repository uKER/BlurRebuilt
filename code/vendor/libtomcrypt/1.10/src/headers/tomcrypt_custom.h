#ifndef TOMCRYPT_CUSTOM_H_
#define TOMCRYPT_CUSTOM_H_

/* macros for various libc functions you can change for embedded targets */
#ifndef XMALLOC
#define XMALLOC  malloc
#endif
#ifndef XREALLOC
#define XREALLOC realloc
#endif
#ifndef XCALLOC
#define XCALLOC  calloc
#endif
#ifndef XFREE
#define XFREE    free
#endif

#ifndef XMEMSET
#define XMEMSET  memset
#endif
#ifndef XMEMCPY
#define XMEMCPY  memcpy
#endif
#ifndef XMEMCMP
#define XMEMCMP  memcmp
#endif

#ifndef XCLOCK
#define XCLOCK   clock
#endif
#ifndef XCLOCKS_PER_SEC
#define XCLOCKS_PER_SEC CLOCKS_PER_SEC
#endif

#ifndef XQSORT
#define XQSORT qsort
#endif

/* Use small code where possible */
/* #define LTC_SMALL_CODE */

/* disable all file related functions */
 #define LTC_NO_FILE 

/* disable all forms of ASM */
/* #define LTC_NO_ASM */

/* disable FAST mode */
/* #define LTC_NO_FAST */

/* disable BSWAP on x86 */
/* #define LTC_NO_BSWAP */

/* ---> Symmetric Block Ciphers <--- */
#ifndef LTC_NO_CIPHERS

#define DES
#define RIJNDAEL

#endif /* LTC_NO_CIPHERS */


/* ---> Block Cipher Modes of Operation <--- */
#ifndef LTC_NO_MODES

#define CFB
#define OFB
#define ECB
#define CBC
#define CTR

#endif /* LTC_NO_MODES */

/* ---> One-Way Hash Functions <--- */
#ifndef LTC_NO_HASHES 

#define SHA512
#define SHA384
#define SHA256
#define SHA224
#define TIGER
#define SHA1
#define MD5

#endif /* LTC_NO_HASHES */

/* ---> MAC functions <--- */
#ifndef LTC_NO_MACS

#define HMAC

#if defined(PELICAN) && !defined(RIJNDAEL)
   #error Pelican-MAC requires RIJNDAEL
#endif

/* ---> Encrypt + Authenticate Modes <--- */

#define OCB_MODE
#define CCM_MODE
//#define GCM_MODE

/* Use 64KiB tables */
#ifndef LTC_NO_TABLES
   #define GCM_TABLES 
#endif

#endif /* LTC_NO_MACS */

/* --> Pseudo Random Number Generators <--- */
#ifndef LTC_NO_PRNGS

/* Yarrow */
#define YARROW
/* which descriptor of AES to use?  */
/* 0 = rijndael_enc 1 = aes_enc, 2 = rijndael [full], 3 = aes [full] */
#define YARROW_AES 3

/* Fortuna PRNG */
#define FORTUNA
/* reseed every N calls to the read function */
#define FORTUNA_WD    10
/* number of pools (4..32) can save a bit of ram by lowering the count */
#define FORTUNA_POOLS 32

#if defined(YARROW) && !defined(CTR)
   #error YARROW requires CTR chaining mode to be defined!
#endif

/* a PRNG that simply reads from an available system source */
#define SPRNG

/* the *nix style /dev/random device */
#define DEVRANDOM
/* try /dev/urandom before trying /dev/random */
#define TRY_URANDOM_FIRST

#endif /* LTC_NO_PRNGS */

/* ---> math provider? <--- */
#ifndef LTC_NO_MATH

/* LibTomMath */
#ifndef LTM_DESC
#define LTM_DESC
#endif

#endif /* LTC_NO_MATH */

/* ---> Public Key Crypto <--- */
#ifndef LTC_NO_PK

/* Digital Signature Algorithm */
//#define MDSA

/* Include RSA support */
#define MRSA

/* ECC */
#define MECC

#endif /* LTC_NO_PK */

/* PKCS #1 (RSA) and #5 (Password Handling) stuff */
#ifndef LTC_NO_PKCS

#endif /* LTC_NO_PKCS */

#ifdef MECC
/* Supported ECC Key Sizes */
#ifndef LTC_NO_CURVES
   #define ECC192
   #define ECC224
   #define ECC256
   #define ECC384
   #define ECC521
#endif
#endif

#if defined(MECC) || defined(MRSA) || defined(MDSA) || defined(MKATJA)
   /* Include the MPI functionality?  (required by the PK algorithms) */
   #define MPI
#endif

#ifdef MRSA
   #define PKCS_1
#endif   

#define LTC_DER

#if defined(LTC_DER) && !defined(MPI) 
   #error ASN.1 DER requires MPI functionality
#endif

#if (defined(MDSA) || defined(MRSA) || defined(MECC) || defined(MKATJA)) && !defined(LTC_DER)
   #error PK requires ASN.1 DER functionality, make sure LTC_DER is enabled
#endif

/* THREAD management */

#undef LTC_PTHREAD

#ifdef LTC_PTHREAD

#include <pthread.h>

#define LTC_MUTEX_GLOBAL(x)   pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
#define LTC_MUTEX_PROTO(x)    extern pthread_mutex_t x;
#define LTC_MUTEX_TYPE(x)     pthread_mutex_t x;
#define LTC_MUTEX_INIT(x)     pthread_mutex_init(x, NULL);
#define LTC_MUTEX_LOCK(x)     pthread_mutex_lock(x);
#define LTC_MUTEX_UNLOCK(x)   pthread_mutex_unlock(x);

#else

/* default no functions */
#define LTC_MUTEX_GLOBAL(x)
#define LTC_MUTEX_PROTO(x)
#define LTC_MUTEX_TYPE(x)
#define LTC_MUTEX_INIT(x)
#define LTC_MUTEX_LOCK(x)
#define LTC_MUTEX_UNLOCK(x)

#endif

#endif



/* $Source: /cvs/libtom/libtomcrypt/src/headers/tomcrypt_custom.h,v $ */
/* $Revision: 1.39 $ */
/* $Date: 2006/01/29 15:53:39 $ */
