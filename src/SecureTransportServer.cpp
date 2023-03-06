#include "SecureTransportServer.h"
#include <iostream>
#include <string>
#include <ctime>

#include "Hash.h"

/**
 * .
 * 
 * \param sock				将其获取，并将原参数清空
 */
SecureTransport::SecureTransport(std::unique_ptr<M_SOCKET::SocketService> sock)
	:_sock{ std::move(sock)},
	_aesKey{ nullptr },
	_rsa{ nullptr }
{
	srand((unsigned int)time(nullptr));
	sock.reset();
}

SecureTransport::SecureTransport(const SecureTransport&& src)
{
	_sock.reset(std::move(src._sock.get()));
	_aesKey.reset(std::move(src._aesKey.get()));
	_rsa.reset(std::move(src._rsa.get()));
}

SecureTransport& SecureTransport::operator=(const SecureTransport&& src)
{
	_sock.reset(std::move(src._sock.get()));
	_aesKey.reset(std::move(src._aesKey.get()));
	_rsa.reset(std::move(src._rsa.get()));
}

void SecureTransport::run()
{
	bool flag = true;
	while (flag)
	{
		_reqMsg.Clear();
		// 获取数据
		std::string msgStr = _sock->recvMsg();
		if (msgStr.empty())
			return;

		// 对数据反序列化
		if (!_reqMsg.ParsePartialFromString(msgStr))
		{
			std::cout << "客户端请求数据有误，直接断联" << std::endl;
			return;
		}

		// 分支判断请求
		switch (_reqMsg.cmdtype())
		{
		case MSG::REQUEST_AGREEMENT:
			if (!agreementKey())
				flag = false;
			break;
		case MSG::REQUEST_CHECK:
			if (!checkKey())
				flag = false;
			break;
		case MSG::REQUEST_LOGOUT:
			if (!logoutKey())
				flag = false;
			break;
		default:
			std::cout << "客户端请求数据有误，直接断联" << std::endl;
			return;
		}
		std::string out;
		if (_resMsg.SerializePartialToString(&out))
			_sock->sendMsg(out);
		else flag = false;
	}
	_sock.reset();
}

/**
 * .
 * 
 * \brief				密钥协商
 * \return				密钥协商是否成功
 */
bool SecureTransport::agreementKey()
{
	// id校验，确定发送端和接收端，查数据库
	if (_reqMsg.serverid() != "server123" && _reqMsg.clientid() != "client123")
	{
		std::cout << "服务器/客户端ID错误" << std::endl;
		return false;
	}
	// 签名校验，确定数据可靠性
	if (!checkSign())
		return false;

	// 生成对称密钥
	createAesKey();
	// 对称密钥加密
	_resMsg.set_rv(false);
	_resMsg.set_data(_rsa->publicEncrypt(std::string(_aesKey.get(), strlen(_aesKey.get()) + 1)));
	// 组织应答报文
	_resMsg.set_clientid(_reqMsg.clientid());
	_resMsg.set_serverid(_reqMsg.serverid());
	_resMsg.set_seckeyid(++keyId);
	// 密钥写入共享内存
	// 密钥持久化存储

	_resMsg.set_rv(true);
	return true;
}

/**
 * .
 *
 * \brief				密钥校验
 * \return				校验结果
 */
bool SecureTransport::checkKey()
{
	_resMsg.set_rv(false);
	std::string serverid = _reqMsg.serverid();
	std::string clientid = _reqMsg.clientid();
	std::string data = _reqMsg.data();
	_resMsg.set_clientid(clientid);
	_resMsg.set_serverid(serverid);
	try
	{
		_resMsg.set_seckeyid(std::stoi(data));
	}
	catch (const std::exception&)
	{
		// 客户端数据问题
		return false;
	}
	M_CRYPTO::Hash hash;
	hash.addData(serverid);
	hash.addData(clientid);
	hash.addData(data);
	std::string onceHash = hash.result();
	hash.addData(onceHash);
	if (_reqMsg.sign() == hash.result())
	{
		_resMsg.set_data(std::string("----校验成功----"));
		_resMsg.set_rv(true);
	}
	else _resMsg.set_data(std::string("----校验失败----"));
	return true;
}

/**
 * .
 * 
 * \brief				密钥注销
 * \return				注销结果
 */
bool SecureTransport::logoutKey()
{
	_resMsg.set_rv(false);
	_resMsg.set_clientid(_reqMsg.clientid());
	_resMsg.set_serverid(_reqMsg.serverid());
	std::string data = _reqMsg.data();
	try
	{
		_resMsg.set_seckeyid(std::stoi(_reqMsg.data()));
	}
	catch (const std::exception&)
	{
		// 客户端数据问题
		return false;
	}

	// 共享内存删除该密钥

	// 数据库标注


	_resMsg.set_data(std::string("----ok----"));
	return false;
}

/**
 * .
 * 
 * \brief				签名校验，确定数据可靠性
 * \return				校验结果
 */
bool SecureTransport::checkSign()
{
	std::string serverid = _reqMsg.serverid();
	std::string clientid = _reqMsg.clientid();
	std::string data = _reqMsg.data();
	// 签名校验，确定数据可靠性
	M_CRYPTO::Hash verifyHash;
	verifyHash.addData(serverid);
	verifyHash.addData(clientid);
	verifyHash.addData(data);
	std::string sourceData = verifyHash.result();
	if (sourceData.empty())
		return false;

	_rsa.reset(new M_CRYPTO::RsaCrypto{ _reqMsg.data() });
	return _rsa->rsaVerify(sourceData, _reqMsg.sign());
}

/**
 * .
 * 
 * \brief				通过随机数生成密钥
 */
void SecureTransport::createAesKey(M_CRYPTO::AesKeyLen keyLen)
{
	int len = 16;
	switch (keyLen)
	{
	case M_CRYPTO::AesKeyLen::PRIMARY_LEN:
		len = 16;
		break;
	case M_CRYPTO::AesKeyLen::INTERMEDIATE_LEN:
		len = 24;
		break;
	case M_CRYPTO::AesKeyLen::ADVANCE_LEN:
		len = 32;
		break;
	}
	_aesKey.reset(new char[len + 1]);
	char* key= _aesKey.get();
	for (size_t i = 0; i < len; i++)
	{
		key[i] = rand() % 93 + 33;
	}
	key[len] = '\0';
}
