#include "SecureTransportClient.h"
#include <iostream>
#include <stdlib.h>

SecureTransportClient::SecureTransportClient()
	:_serverid{ "server123" },
	_clientid{ "client123" },
	_isConToSer{ true }
{
	_sock.connectToHost("192.168.128.130", 8888);
	if (_sock.getError() == M_SOCKET::STATUS_TYPE::S_TTIMEOUT)
	{
		std::cout << "连接服务器超时" << std::endl;
		_isConToSer = false;
	}
}

/**
 * .
 * 
 * \brief				程序运行
 */
void SecureTransportClient::run()
{
	while (_isConToSer && showBasisMsg())
	{
		_reqMsg.Clear();
		_resMsg.Clear();
		switch (_cmdType)
		{
		case MSG::REQUEST_AGREEMENT:
			if (!agreementKey())
				return;
			break;
		case MSG::REQUEST_CHECK:
			if (!checkKey())
				return;
			break;
		case MSG::REQUEST_LOGOUT:
			if (!logoutKey())
				return;
			break;
		}
	}
}

/**
 * .
 * 
 * \brief				显示选项信息
 */
bool SecureTransportClient::showBasisMsg()
{
	system("clear");
	char choose = ' ';
	bool isChoose = false;
	bool isQuit = false;
	bool isClear = true;
	while (isClear)
	{
		isClear = false;
		std::cout << "欢迎使用安全传输密钥生成测试系统，请内容进行选择：" << std::endl
			<< "1. 密钥协商" << std::endl
			<< "2. 密钥校验" << std::endl
			<< "3. 密钥注销" << std::endl
			<< "q. 退出系统" << std::endl
			<< "c. 清理屏幕" << std::endl
			<< "请按选项输入(1/2/3/q/Q/c/C):" << std::flush;
		do
		{
			std::cin >> choose;
			isChoose = true;
			switch (choose)
			{
			case '1':
				_cmdType = MSG::RequestType::REQUEST_AGREEMENT;
				break;
			case '2':
				_cmdType = MSG::RequestType::REQUEST_CHECK;
				break;
			case '3':
				_cmdType = MSG::RequestType::REQUEST_LOGOUT;
				break;
			case 'q':
			case 'Q':
				isQuit = true;
				break;
			case 'c':
			case 'C':
				isClear = true;
				break;
			default:
				isChoose = false;
				break;
			}
		} while (!isChoose);
	}
	return !isQuit;
}

/**
 * .
 * 
 * \brief				密钥协商
 * \return				协商结果
 */
bool SecureTransportClient::agreementKey()
{
	// 生成公私钥
	_rsa.reset(new M_CRYPTO::RsaCrypto{ 1024 });

	// 组织请求报文
	if (!generateMsg())
	{
		std::cout << "请求报文生成失败" << std::endl;
		return false;
	}

	// 报文序列化
	std::string sendData;
	if (_reqMsg.SerializePartialToString(&sendData))
	{
		std::cout << "序列化请求失败" << std::endl;
		return false;
	}

	// 发送报文
	if (_sock.sendMsg(sendData))
	{
		std::cout << "发送报文失败" << std::endl;
		if (_sock.getError() == M_SOCKET::STATUS_TYPE::S_TTIMEOUT)
			std::cout << "发送数据超时" << std::endl;
		return false;
	}

	// 等待应答
	std::string resData = _sock.recvMsg();
	if (resData.empty())
	{
		std::cout << "等待应答失败" << std::endl;
		if (_sock.getError() == M_SOCKET::STATUS_TYPE::S_TTIMEOUT)
			std::cout << "获取应答数据超时" << std::endl;
		return false;
	}

	// 解析应答报文
	if (!parsingMsg(resData))
		return false;
	if (_resMsg.data().size() != 16 || _resMsg.data().size() != 24 || _resMsg.data().size() != 32)
	{
		std::cout << "数据异常" << std::endl;
		return false;
	}

	// 存储对称密钥
	_aesKey.reset(new char[_resMsg.data().size()] + 1);
	strncpy(_aesKey.get(), _resMsg.data().c_str(), _resMsg.data().size());
	_aesKey.get()[_resMsg.data().size()] = '\0';

	// 对称密钥存入共享内存
	std::cout << _aesKey.get() << std::endl;
}

/**
 * .
 *
 * \brief				密钥校验
 * \return				校验结果
 */
bool SecureTransportClient::checkKey()
{
}

/**
 * .
 *
 * \brief				密钥注销
 * \return				注销结果
 */
bool SecureTransportClient::logoutKey()
{
}

/**
 * .
 * 
 * \brief				生成报文
 * \return 
 */
bool SecureTransportClient::generateMsg()
{
	std::string data = _rsa->getPublicKey();
	if (data.empty())
		return false;
	_reqMsg.set_cmdtype(_cmdType);
	_reqMsg.set_data(data);
	_reqMsg.set_serverid(_serverid);
	_reqMsg.set_clientid(_clientid);
	M_CRYPTO::Hash hash;
	hash.addData(_serverid);
	hash.addData(_clientid);
	hash.addData(data);
	std::string onceHash = hash.result();
	if (onceHash.empty())
		return false;
	std::string sign = _rsa->rsaSign(onceHash);
	if (sign.empty())
		return false;
	_reqMsg.set_sign(sign);
}

bool SecureTransportClient::parsingMsg(std::string& recvData)
{
	if (_resMsg.ParsePartialFromString(recvData))
	{
		std::cout << "解析应答失败" << std::endl;
		return false;
	}
	if (!_resMsg.rv())
	{
		std::cout << "交互失败" << std::endl;
		return false;
	}
	if (_resMsg.clientid() != _clientid &&
		_resMsg.serverid() != _serverid &&
		_resMsg.data().empty())
	{
		std::cout << "数据异常" << std::endl;
		return false;
	}
	return true;
}
