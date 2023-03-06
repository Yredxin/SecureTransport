#ifndef OBJECT_SOCKET_LISTENER_Y
#define OBJECT_SOCKET_LISTENER_Y
#include <string>
#include <memory>
#include "SocketService.h"

#ifdef _WIN32
#include <WinSock2.h>
typedef SOCKET SOCKET_TYPE;
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
typedef int SOCKET_TYPE;
#endif

namespace M_SOCKET
{
	class SocketListener;
}

class M_SOCKET::SocketListener
{
public:
	SocketListener(const int port);
	~SocketListener();
	bool resetSocket(const int port = 0);
	std::unique_ptr<SocketService> acceptReq(int waitTime = DEFAULT_SEC);
	inline STATUS_TYPE getError() { return _err; };

private:
	SOCKET_TYPE _fd;
	int _port;
	STATUS_TYPE _err;
};

#endif // !OBJECT_SOCKET_LISTENER_Y

