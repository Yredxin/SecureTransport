#ifndef OBJECT_SECURE_TRANSPORT_SERVER_Y
#define OBJECT_SECURE_TRANSPORT_SERVER_Y
#include <memory>
#include "msg.pb.h"
#include "SocketService.h"
#include "AesCrypto.h"
#include "RsaCrypto.h"

static int keyId = 0;

class SecureTransport
{
public:
	SecureTransport(std::unique_ptr<M_SOCKET::SocketService> sock);
	SecureTransport(const SecureTransport&& src);
	SecureTransport& operator=(const SecureTransport&& src);
	void run();

private:
	// 密钥协商
	bool agreementKey();
	// 密钥校验
	bool checkKey();
	// 密钥注销
	bool logoutKey();

private:
	bool checkSign();
	void createAesKey(M_CRYPTO::AesKeyLen keyLen = M_CRYPTO::AesKeyLen::PRIMARY_LEN);
private:
	std::unique_ptr<M_SOCKET::SocketService> _sock;
	MSG::RequestMsg _reqMsg;
	MSG::RespondMsg _resMsg;
	std::unique_ptr<char> _aesKey;
	std::unique_ptr<M_CRYPTO::RsaCrypto> _rsa;
};

#endif // !OBJECT_SECURE_TRANSPORT_SERVER_Y


