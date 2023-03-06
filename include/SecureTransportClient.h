#ifndef OBJECT_SECURE_TRANSPORT_CLINENT_Y
#define OBJECT_SECURE_TRANSPORT_CLINENT_Y
#include <memory>

#include "msg.pb.h"
#include "RsaCrypto.h"
#include "AesCrypto.h"
#include "Hash.h"
#include "SocketService.h"

class SecureTransportClient
{
public:
	SecureTransportClient();
	void run();

private:
	bool showBasisMsg();
	// 密钥协商
	bool agreementKey();
	// 密钥校验
	bool checkKey();
	// 密钥注销
	bool logoutKey();
	// 生成报文
	bool generateMsg();
	// 解析报文
	bool parsingMsg(std::string& recvData);

private:
	MSG::RequestType _cmdType;
	MSG::RequestMsg _reqMsg;
	MSG::RespondMsg _resMsg;
	std::unique_ptr<char> _aesKey;
	std::unique_ptr<M_CRYPTO::RsaCrypto> _rsa;
	std::string _serverid;
	std::string _clientid;
	M_SOCKET::SocketService _sock;
	bool _isConToSer;
};

#endif // !OBJECT_SECURE_TRANSPORT_CLINENT_Y



