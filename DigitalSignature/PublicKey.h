#pragma once

namespace DigitalSignature {

const short vaild_time_diff = 10; //检查时间戳合法性的接受时间范围，单位：秒

class CPublicKey
{
public:
	CPublicKey(void);
	virtual ~CPublicKey(void);

	virtual bool ValidKey() = 0;
	virtual int getKeyLength() = 0;

	virtual int ParseSignature(unsigned char * inBuf, int inL, int dsIndex,unsigned char * outBuf, int & outL); //解析签名
	virtual int CalcSecretDataLength(unsigned char * buf,bool bCalcByFrame);

private:
	virtual int Authentic(unsigned char *dgst, int dgstlen, unsigned char *sig, int siglen) = 0;

protected:
	time_t last_send_time_;
	time_t last_local_time_;
};

}; //namespace DigitalSignature

