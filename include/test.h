#ifndef PROJECT_TEST_Y
#define PROJECT_TEST_Y

#include "msg.pb.h"
#include "Hash.h"
#include "RsaCrypto.h"
#include "AesCrypto.h"
#include "ShareMemory.h"
#include "SocketListener.h"
#include "SocketService.h"
#include "SecureTransportServer.h"
#include "SecureTransportPool.h"
#include "SecureTransportClient.h"

namespace MyTest_Y {
	class Test;
}

class MyTest_Y::Test
{
public:
	static void RespondMsgTest();
	static void RequestMsgTest();
	static void HashTest();
	static void RsaCryptoTest();
	static void AesCryptoTest();
	static void SharedMemoryTest();
	static void SocketText();
	static void PoolTest();
	static void clientTest();

private:
	Test() = delete;
	~Test() = delete;
	Test(const Test&) = delete;
	Test& operator=(const Test&) = delete;
	Test(const Test&&) = delete;
	Test& operator=(const Test&&) = delete;
};

#endif // PROJECT_TEST_Y
