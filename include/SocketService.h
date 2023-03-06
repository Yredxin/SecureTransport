#ifndef OBJECT_SOCKET_SERVICE_Y
#define OBJECT_SOCKET_SERVICE_Y
#include <string>
#include <memory>

#ifdef _WIN32
#include <WinSock2.h>
typedef SOCKET SOCKET_TYPE;
#pragma comment(lib, "ws2_32.lib")
#define GetSocketError WSAGetLastError()
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
typedef int SOCKET_TYPE;
#define GetSocketError errno
#endif

namespace M_SOCKET
{
	constexpr int DEFAULT_SEC = 5;
	class SocketService;
	enum class STATUS_TYPE :char
	{
		S_SUCCESSFUL,				// 成功	
		S_TTIMEOUT,					// 超时
		S_PEELCLOSE,				// 对端关闭
		S_SOCKETERR,				// 套接字出现问题，需要重置
		S_PARAMERR,					// 参数错误
		S_NOTCLIENT,				// 非客户端调用
		S_OTHRE,					// 使用系统函数查看，perror
	};
}

class M_SOCKET::SocketService
{
public:
	SocketService();
	SocketService(SOCKET_TYPE fd, void *addr, int addrLen);
	~SocketService();
	bool resetSocket();
	bool connectToHost(const std::string& host, const int port, int waitTime = DEFAULT_SEC);
	std::string recvMsg(int waitTime = DEFAULT_SEC);
	bool sendMsg(const std::string& data, int waitTime = DEFAULT_SEC);
	inline STATUS_TYPE getError() { return _err; };
	static bool setNonBlock(int fd);
	static bool setBlock(int fd);
	std::string getPeerIP();
	int getPeerPort();
private:
	std::string readN(int count);
	int writeN(const std::string& data);
	bool connectTimeout(int waitTime);

private:
	SOCKET_TYPE _fd;
	STATUS_TYPE _err;
	bool _flag;
	std::unique_ptr<char> _peerAddr;
	int _peerAddrLen;
	bool _isClient;
};
#endif // !OBJECT_SOCKET_SERVICE_Y

