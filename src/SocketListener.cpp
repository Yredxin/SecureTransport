#include "SocketListener.h"
#include <regex>
#ifdef _WIN32
#else
#define closesocket(s) close(s)
#define INVALID_HANDLE_VALUE (-1)
#endif // !_WIN32

/**
 * .
 * 
 * \brief				保存状态且初始化
 * \param port			开放端口
 * \param host			本地主机
 */
M_SOCKET::SocketListener::SocketListener(const int port)
	:_fd{ (SOCKET_TYPE)INVALID_HANDLE_VALUE },
	_port{ port },
	_err{ STATUS_TYPE::S_SUCCESSFUL }
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
	if (_port <= 0 || _port > 65535)
	{
		_err = STATUS_TYPE::S_PARAMERR;
		return;
	}
	resetSocket(port);
}

/**
 * .
 * 
 * \brief				释放掉端口
 */
M_SOCKET::SocketListener::~SocketListener()
{
#ifdef _WIN32
	::WSACleanup();
#endif
	if (_fd != (SOCKET_TYPE)INVALID_HANDLE_VALUE)
		closesocket(_fd);
}

/**
 * .
 * 
 * \brief				重置连接及初始化
 * \param port			端口号
 * \param host			本地ip
 * \return				成功：true，失败：false
 */
bool M_SOCKET::SocketListener::resetSocket(const int port)
{
	if (_fd != (SOCKET_TYPE)INVALID_HANDLE_VALUE)
		closesocket(_fd);
	if (port < 0 || port > 65535)
	{
		_err = STATUS_TYPE::S_PARAMERR;
		return false;
	}
	if (port != 0)
		_port = port;
	// 创建套接字
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	// 绑定端口
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
		return false;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
#ifdef _WIN32
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#else
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif // _WIN32
	if (bind(_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
	{
		_err = STATUS_TYPE::S_OTHRE;
		return false;
	}
	// 设置监听
	listen(_fd, 128);
	if (_fd == (SOCKET_TYPE)INVALID_HANDLE_VALUE)
		_err = STATUS_TYPE::S_SOCKETERR;
	else
		_err = STATUS_TYPE::S_SUCCESSFUL;
	return (_fd != (SOCKET_TYPE)INVALID_HANDLE_VALUE);
}

/**
 * .
 * 
 * \brief				等待客户端连接
 * \return				成功：返回服务套接字指针，失败：nullptr
 */
std::unique_ptr<M_SOCKET::SocketService> M_SOCKET::SocketListener::acceptReq(int waitTime)
{
	if (_err != STATUS_TYPE::S_SUCCESSFUL && _err != STATUS_TYPE::S_TTIMEOUT)
		return nullptr;
	
	if (waitTime > 0)
	{
		if (!SocketService::setNonBlock(_fd))
		{
			_err = STATUS_TYPE::S_OTHRE;
			return nullptr;
		}
	}

	int ret = 0;
	std::unique_ptr<SocketService> cs;
	bool flags = false;
	fd_set readset;
	FD_ZERO(&readset);
	FD_SET(_fd, &readset);
	struct timeval timeout = { waitTime, 0 };
	do
	{
		ret = select(_fd + 1, &readset, nullptr, nullptr, &timeout);
	} while (ret < 0 && GetSocketError == EINTR);

	switch (ret)
	{
	case 0:
		_err = STATUS_TYPE::S_TTIMEOUT;
		break;
	case -1:
		_err = STATUS_TYPE::S_OTHRE;
		break;
	default:
		flags = true;
		break;
	}
	
	if (flags)
	{
		sockaddr_in addr;
#ifdef _WIN32
		int addrLen = sizeof(addr);
#else
		socklen_t addrLen = sizeof(addr);
#endif // _WIN32

		SOCKET_TYPE lfd = accept(_fd, (sockaddr*)&addr, &addrLen);
		if (lfd >= 0)
		{
			cs.reset(new SocketService{ lfd, &addr, addrLen });
		}
	}

	if (waitTime > 0)
	{
		if (!SocketService::setBlock(_fd))
		{
			_err = STATUS_TYPE::S_OTHRE;
			return cs;
		}
	}
	if (cs)
		_err = STATUS_TYPE::S_SUCCESSFUL;
	return cs;
}
