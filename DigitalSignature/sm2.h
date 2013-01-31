/*
 * sm2.h
 *
 *  Created on: 2011-6-3
 *  Author: zweib@sgepri.sgcc
 */

#ifndef HEADER_SM2_H
#define HEADER_SM2_H

#ifdef  __cplusplus
extern "C" {
#endif

namespace DigitalSignature {

#define ECCref_MAX_BITS		256	//该常量定义按照目前国内支持的ECC最大模长而定
#define ECCref_MAX_LEN		((ECCref_MAX_BITS+7) / 8)

typedef struct ECCrefPublicKey_st
{
	unsigned int  bits;
	unsigned char x[ECCref_MAX_LEN];
	unsigned char y[ECCref_MAX_LEN];
} ECCrefPublicKey;

typedef struct ECCrefPrivateKey_st
{
	unsigned int  bits;
	unsigned char D[ECCref_MAX_LEN];
} ECCrefPrivateKey;

typedef struct ECCCipher_st
{
	unsigned char x[ECCref_MAX_LEN];
	unsigned char y[ECCref_MAX_LEN];
	unsigned char C[ECCref_MAX_LEN];
	unsigned char M[ECCref_MAX_LEN];
} ECCCipher;

typedef struct ECCSignature_st
{
	unsigned char r[ECCref_MAX_LEN];
	unsigned char s[ECCref_MAX_LEN];
} ECCSignature;

int SM2_Verify(unsigned char *pucDataInput, unsigned int uiInputLength,
		unsigned char *pucID, unsigned int uiIDLength,
		ECCrefPublicKey *pucPublicKey, ECCSignature *pucSignature);

int SM2_Encrypt(unsigned char *pucDataInput, unsigned int uiInputLength,
				ECCrefPublicKey *pucPublicKey, unsigned char *pucRandom,
				unsigned int uiRandomLength, ECCCipher *pucEncData);

int SM2_GenerateRandom(unsigned int uiLength, unsigned char *pucRandom);

void sm2_init();

void sm2_final();

};//namespace DigitalSignature

#ifdef  __cplusplus
}
#endif
#endif
