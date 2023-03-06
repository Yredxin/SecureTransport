#include "SocketService.h"
#include <cstring>

#ifdef _WIN32
#include <WS2tcpip.h>
#else
#define closesocket(s) close(s)
#define INVALID_HANDLE_VALUE (-1)
#endif // !_WIN32

/**
 * .
 *
 * \brief				用户初始化使用
 */
M_SOCKET::SocketService::SocketService()
	:_fd{ (SOCKET_TYPE)INVALID_HANDLE_VALUE },
	_flag{ true },
	_isClient{ true },
	_err{ STATUS_TYPE::S_SUCCESSFUL },
	_peerAddr{ nullptr },
	_peerAddrLen{ 0 }
{
#ifdef _WIN32
	// 指定版本号
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int nErrorID = ::WSAStartup(wVersionRequested, &wsaData);
	if (nErrorID != 0)
		return;
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wHighVersion) != 2)
	{
		::WSACleanup();
		return;
	}
#endif
	_fd = socket(AF_INET, SOCK_STREAM, 0);
}

/**
 * .
 *
 * \brief				服务器通信套接字使用
 * \param fd			通信套接字
 */
M_SOCKET::SocketService::SocketService(SOCKET_TYPE fd, void* addr, int addrLen)
	:_fd{ fd },
	_flag{ false },
	_isClient{ false },
	_err{ STATUS_TYPE::S_SUCCESSFUL },
	_peerAddrLen{ addrLen }
{
	_peerAddr.reset(new char[addrLen]);
	memcpy(_peerAddr.get(), addr, _peerAddrLen);
}

/**
 * .
 *
 * \brief				关闭套接字
 */
M_SOCKET::SocketService::~SocketService()
{
#ifdef _WIN32
	if (_flag)
		::WSACleanup();
#endif
	if (_fd != (SOCKET_TYPE)INVALID_HANDLE_VALUE)
		closesocket(_fd);
}

bool M_SOCKET::SocketService::resetSocket()
{
	if (_fd != (SOCKET_TYPE)INVALID_HANDLE_VALUE)
		closesocket(_fd);
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == (SOCKET_TYPE)INVALID_HANDLE_VALUE)
		_err = STATUS_TYPE::S_SOCKETERR;
	else
		_err = STATUS_TYPE::S_SUCCESSFUL;
	return (_fd != (SOCKET_TYPE)INVALID_HANDLE_VALUE);
}

/**
 * .
 *
 * \brief				连接到服务器
 * \param host			服务器地址
 * \param port			服务器端口号
 * \param waitTime		超时等待时间，默认1s
 * \return				成功连接：true，连接失败：false通过getError获取失败值
 */
bool M_SOCKET::SocketService::connectToHost(const std::string& host, const int port, int waitTime)
{
	if (!_isClient)
		_err = STATUS_TYPE::S_NOTCLIENT;
	if (_err != STATUS_TYPE::S_SUCCESSFUL && _err != STATUS_TYPE::S_TTIMEOUT)
		return false;

	if (waitTime > 0)
	{
		if (!SocketService::setNonBlock(_fd))
		{
			_err = STATUS_TYPE::S_OTHRE;
			return false;
		}
	}

	bool flag = false;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef _WIN32
	inet_pton(AF_INET, host.data(), &addr.sin_addr.S_un.S_addr);
#else
	inet_pton(AF_INET, host.data(), &addr.sin_addr.s_addr);
#endif // _WIN32

	int ret = connect(_fd, (sockaddr*)&addr, sizeof(addr));
	switch (ret)
	{
	case -1:
		//if (GetSocketError == EINPROGRESS)
		if (waitTime > 0)
			flag = connectTimeout(waitTime);
		else return ((_err = STATUS_TYPE::S_OTHRE) == STATUS_TYPE::S_SUCCESSFUL);
		break;
	default:
		flag = true;
		break;
	}

	if (waitTime > 0)
	{
		if (!SocketService::setBlock(_fd))
		{
			_err = STATUS_TYPE::S_OTHRE;
			return false;
		}
	}
	_err = STATUS_TYPE::S_SUCCESSFUL;
	return flag;
}

/**
 * .
 *
 * \brief				读取数据
 * \param waitTime		超时等待时间
 * \return				成功：读取的数据，失败：空字符串
 */
std::string M_SOCKET::SocketService::recvMsg(int waitTime)
{
	if (_err != STATUS_TYPE::S_SUCCESSFUL && _err != STATUS_TYPE::S_TTIMEOUT)
		return std::string();

	if (waitTime > 0)
	{
		fd_set readset;
		FD_ZERO(&readset);
		FD_SET(_fd, &readset);
		timeval timeout = { waitTime, 0 };
		int ret = 0;
		do
		{
			ret = select(_fd + 1, &readset, NULL, NULL, &timeout);
		} while (ret < 0 && GetSocketError == EINTR);
		switch (ret)
		{
		case 0:
			_err = STATUS_TYPE::S_TTIMEOUT;
			return std::string();
		case -1:
			return std::string();
		}
	}
	std::string msgHead = readN(sizeof(int));
	if (msgHead.size() < sizeof(int))
		return std::string();
	int msgLen;
	memcpy((void*)&msgLen, msgHead.c_str(), sizeof(int));
	return readN(ntohl(msgLen));
}

/**
 * .
 *
 * \brief				发送数据
 * \param data			需要发送的数据
 * \param waitTime		超时等待时间
 * \return				成功：true，失败：false通过getError获取失败原因
 */
bool M_SOCKET::SocketService::sendMsg(const std::string& data, int waitTime)
{
	if (_err != STATUS_TYPE::S_SUCCESSFUL && _err != STATUS_TYPE::S_TTIMEOUT)
		return false;

	if (waitTime > 0)
	{
		fd_set writeset;
		FD_ZERO(&writeset);
		FD_SET(_fd, &writeset);
		timeval timeout = { waitTime, 0 };
		int ret = 0;
		do
		{
			ret = select(_fd + 1, NULL, &writeset, NULL, &timeout);
		} while (ret < 0 && GetSocketError == EINTR);
		switch (ret)
		{
		case 0:
			_err = STATUS_TYPE::S_TTIMEOUT;
			return false;
		case -1:
			return false;
		}
	}

	int dataLen = htonl(data.size());
	std::string sendData{ (char*)&dataLen, sizeof(dataLen) };
	return writeN(sendData + data);
}

/**
 * .
 *
 * \brief				设置文件描述符非阻塞
 * \param fd			需要设置的文件描述符
 * \return					是否设置成功
 */
bool M_SOCKET::SocketService::setNonBlock(int fd)
{
	int ret = 0;
#ifdef _WIN32
	u_long argp = 1;
	ret = ioctlsocket(fd, FIONBIO, &argp);
#else
	int flag = fcntl(fd, F_GETFL);
	if (flag == -1)
		return false;
	ret = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
#endif // _WIN32
	return ret >= 0;
}

bool M_SOCKET::SocketService::setBlock(int fd)
{
	int ret = 0;
#ifdef _WIN32
	u_long argp = 0;
	ret = ioctlsocket(fd, FIONBIO, &argp);
#else
	int flag = fcntl(fd, F_GETFL);
	if (flag == -1)
		return false;
	ret = fcntl(fd, F_SETFL, flag & (~O_NONBLOCK));
#endif // _WIN32
	return ret >= 0;
}

/**
 * .
 *
 * \brief				获取对端ip
 * \return				返回点分十进制字符串
 */
std::string M_SOCKET::SocketService::getPeerIP()
{
	return std::string();
}

/**
 * .
 *
 * \brief				获取对端端口
 * \return				返回对端端口信息
 */
int M_SOCKET::SocketService::getPeerPort()
{
	return 0;
}

/**
 * .
 *
 * \brief				读取N个字符
 * \param count			待读取字符数
 * \return				成功：读取的字符，失败：空字符，对端关闭，返回以接收到的字符串
 */
std::string M_SOCKET::SocketService::readN(int count)
{
	int tmpLen = count;
	int getN = 0;
	std::unique_ptr<char> buf{ new char[count] };
	char* tmpBuf = buf.get();
	while (tmpLen > 0)
	{
		if ((getN = recv(_fd, tmpBuf, tmpLen, 0)) < 0)
		{
			if (GetSocketError == EINTR)
			{
				continue;
			}
			return std::string();
		}
		if (getN == 0)
		{
			_err = STATUS_TYPE::S_PEELCLOSE;
			return std::string(buf.get(), count - tmpLen);
		}
		tmpBuf += getN;
		tmpLen -= getN;
	}
	_err = STATUS_TYPE::S_SUCCESSFUL;
	return std::string(buf.get(), count);
}

/**
 * .
 *
 * \brief				写入数据
 * \param data			待写入的数据
 * \return				成功：true，失败false
 */
int M_SOCKET::SocketService::writeN(const std::string& data)
{
	int tmpLen = data.size();
	const char* tmpBuf = data.data();
	int getN = 0;
	while (tmpLen > 0)
	{
		if (((getN = send(_fd, tmpBuf, tmpLen, 0)) < 0))
		{
			if (GetSocketError == EINTR)
			{
				continue;
			}
			return -1;
		}
		if (getN == 0)
		{
			_err = STATUS_TYPE::S_PEELCLOSE;
			return data.size() - tmpLen;
		}
		tmpBuf += getN;
		tmpLen -= getN;
	}
	_err = STATUS_TYPE::S_SUCCESSFUL;
	return data.size();
}

/**
 * .
 *
 * \brief				处理连接等待
 * \param waitTime		等待时间
 * \return				返回连接是否成功
 */
bool M_SOCKET::SocketService::connectTimeout(int waitTime)
{
	fd_set writeset;
	FD_ZERO(&writeset);
	FD_SET(_fd, &writeset);
	timeval timeout = { waitTime, 0 };
	int ret = 0;
	do
	{
		ret = select(_fd + 1, nullptr, &writeset, nullptr, &timeout);
	} while (ret < 0 && GetSocketError == EINTR);
	switch (ret)
	{
	case 0:
		_err = STATUS_TYPE::S_TTIMEOUT;
		return false;
	case -1:
		_err = STATUS_TYPE::S_OTHRE;
		return false;
	}
	int err;
	socklen_t errLen = sizeof(err);
	int sockopt = getsockopt(_fd, SOL_SOCKET, SO_ERROR, (char*)&err, &errLen);
	switch (sockopt)
	{
	case -1:
		_err = STATUS_TYPE::S_OTHRE;
		return false;
	case 0:
		return true;
	default:
		errno = err;
		_err = STATUS_TYPE::S_OTHRE;
		return false;
	}
	return false;
}
