/* $Id: crypto_ecdsa.h,v 1.2 2012/03/28 00:38:10 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/crypto_ecdsa.h,v $
 *------------------------------------------------------------------
 * crypto_ecdsa.h  - Ported from IOS for Quack.
 *
 * Copyright (c) 2009 ~ 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */

#ifndef __CRYPTO_ECDSA_H_
#define __CRYPTO_ECDSA_H_

void crypto_ecdsa_parser_init(void);
int generate_signature(unsigned char *dataToSign,
                       unsigned int dataToSignLen,
                       unsigned char *privateKey,
                       unsigned int privateKeyLen,
                       unsigned char **signature,
                       unsigned int *signatureLen);

int verify_signature(unsigned char *publicKey, unsigned int publicKeyLen,
                      unsigned char *dataToSign, unsigned int dataToSignLen,
                      unsigned char *signature, unsigned int signatureLen);
int generate_message_digest(uchar *dataToDigest, int dataToDigestLen,
                            uchar *digestedData, unsigned int *digestedDataLen);
int generate_key_pair(uchar **pub_key, int *pub_key_len,
                      uchar **pvt_key, int *pvt_key_len);
void PrintBufAsCArray (char *label, unsigned char *buffer,
                       unsigned int bufferLen);
void PrintBuf (char *label, unsigned char *buf, unsigned int bufLen);
#endif /* __CRYPTO_ECDSA_H_ */


/******** History ******** 
$Log: crypto_ecdsa.h,v $
Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
