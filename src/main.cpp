#include <memory>
#include <iostream>
#include "SecureTransportPool.h"
#include "SecureTransportServer.h"
#include "SocketService.h"
#include "SocketListener.h"

int main(int agrc, char* argv[])
{
	SecureTransportPool pool;
	M_SOCKET::SocketListener ser{ 8888 };
	while (true)
	{
		std::unique_ptr<M_SOCKET::SocketService> cli = ser.acceptReq();
		if (ser.getError() == M_SOCKET::STATUS_TYPE::S_TTIMEOUT)
		{
			std::cout << "等待客户端连接" << std::endl;
			continue;
		}
		if (!cli)
		{
			break;
		}
		std::unique_ptr<SecureTransport> task{ new SecureTransport{std::move(cli) } };
		pool.addTask(std::move(task));
	}
	return 0;
}
