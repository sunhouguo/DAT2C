#include <sstream>
#include <iomanip>
#include <iostream>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

#include "../PublicSupport/Dat2cPublicAPI.h"
#include "RsaPublicKey.h"

namespace DigitalSignature {

const char rnd_seed[] = "string to make the random number generator think it has entropy";

using namespace std;

CRsaPublicKey::CRsaPublicKey(std::string keypath)
{
	key_ = NULL;

	LoadKey(keypath);
}

CRsaPublicKey::~CRsaPublicKey(void)
{
	FreeKey();
}

int CRsaPublicKey::getKeyLength()
{
	if (key_)
	{
		return EVP_PKEY_size(key_);
	}

	return -1;
}

//装载公钥
//************************************
// Method:    LoadKey
// FullName:  CRsaPublicKey::LoadKey
// Access:    private 
// Returns:   int
// Qualifier:
// Parameter: std::string keypath ECC证书文件路径
// Return:    0：成功 其它：失败
//************************************
int CRsaPublicKey::LoadKey(std::string keypath)
{
	RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
	ERR_load_crypto_strings();

	// 读入rsa证书
	BIO * bio = BIO_new_file(keypath.c_str(),"rb");
	if (!bio)
	{
		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	X509 * cert = PEM_read_bio_X509(bio,NULL,NULL,NULL);
	if (!cert) 
	{
		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	key_ = X509_get_pubkey(cert);
	if (!key_)
	{
		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	X509_free(cert);
	BIO_free(bio);

	return 0;
}

//************************************
// Method:    FreeKey
// FullName:  CRsaPublicKey::FreeKey
// Access:    private 
// Returns:   int
// Qualifier: 释放内存
// Return:    0：成功 其它：失败
//************************************
int CRsaPublicKey::FreeKey()
{
	return 0;
}

bool CRsaPublicKey::ValidKey()
{
	if (key_)
	{
		return true;
	}

	return false;
}

//************************************
// Method:    Authentic
// FullName:  Authentic
// Access:    public 
// Returns:   int
// Qualifier: 
// Parameter: const unsigned char * dgst 签名输入数据
// Parameter: int dgstlen 签名输入数据长度
// Parameter: const unsigned char * sig 签名
// Parameter: int siglen 签名长度
// Return:    0：认证正确 其它：失败
//************************************
int CRsaPublicKey::Authentic(unsigned char *dgst, int dgstlen,unsigned char *sig, int siglen)
{
	RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */

	if (!key_)
	{
		ostringstream ostr;
		ostr<<"can't load key"<<endl;
		cerr<<ostr.str();

		return -1;
	}

	int ret = RSA_verify(0,dgst,dgstlen,sig,siglen,key_->pkey.rsa);
	if (ret != 1) 
	{
		ostringstream ostr;
		ostr<<"RSA_verify: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();

		return -1;
	}

	return 0;
}

};//namespace DigitalSignature 
