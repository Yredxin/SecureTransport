#include "test.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <memory>

using MyTest_Y::Test;
using namespace std;

void Test::RespondMsgTest()
{
	MSG::RespondMsg msg;
	msg.set_rv(true);
	msg.set_clientid("123123");
	msg.set_serverid("321321");
	msg.set_data("hello client");
	msg.set_seckeyid(123123);
	fstream out("res.dat", ios::out | ios::trunc | ios::binary);
	if (out.is_open())
		// msg.SerializePartialToString(&out);
		msg.SerializeToOstream(&out);
	else
		out << "err" << endl;
	out.close();
	MSG::RespondMsg msg2;
	fstream in{ "res.dat", ios::in | ios::binary };
	msg2.ParseFromIstream(&in);
	cout << "是否成功：" << msg2.rv() << endl
		<< "服务器ID：" << msg2.serverid() << endl
		<< "客户端ID：" << msg2.clientid() << endl
		<< "密钥ID：" << msg2.seckeyid() << endl
		<< "密钥：" << msg2.data() << endl;
	in.close();
}

void MyTest_Y::Test::RequestMsgTest()
{
	MSG::RequestMsg msg;
	msg.set_cmdtype(MSG::REQUEST_CHECK);
	msg.set_clientid("123123");
	msg.set_serverid("321321");
	msg.set_data("hello server");
	msg.set_sign("hahahaha");
	fstream out("req.dat", ios::out | ios::trunc | ios::binary);
	if (out.is_open())
		// msg.SerializePartialToString(&out);
		msg.SerializeToOstream(&out);
	else
		out << "err" << endl;

	out.close();
	MSG::RequestMsg msg2;
	fstream in{ "req.dat", ios::in | ios::binary };
	msg2.ParseFromIstream(&in);
	cout << "类型：" << msg2.cmdtype() << endl
		<< "服务器ID：" << msg2.serverid() << endl
		<< "客户端ID：" << msg2.clientid() << endl
		<< "校验：" << msg2.sign() << endl
		<< "密钥：" << msg2.data() << endl;
	in.close();
}

void MyTest_Y::Test::HashTest()
{
	M_CRYPTO::Hash myHash;
	string str("hello world");
	myHash.addData(str);
	cout << myHash.result();
}

void MyTest_Y::Test::RsaCryptoTest()
{
	cout << "==================密钥=================" << endl;
	M_CRYPTO::RsaCrypto rsa;
	string pri = rsa.getPrivateKey();
	string pub = rsa.getPublicKey();
	cout << "公钥:" << endl 
		<< pri << endl << endl
		<< "私钥:" << endl 
		<< pub << endl << endl;
	cout << "==================加解密=================" << endl;
	string xx = rsa.publicEncrypt("你好c++");
	cout << "密文大小:" << xx.size() << endl;
	cout << "密文:" << xx << endl;
	cout << "明文:" << rsa.privateDecrypt(xx) << endl;
	cout << "==================签名校验=================" << endl;
	string xxHash = rsa.rsaSign("你好c");
	cout << "校验码:" << xxHash << endl;
	cout << "校验结果:" << rsa.rsaVerify("你好c", xxHash) << endl;
	cout << "校验结果:" << rsa.rsaVerify("你好c++", xxHash) << endl;
}

void MyTest_Y::Test::AesCryptoTest()
{
	M_CRYPTO::AesCrypto aes;
	string enc = aes.encryption("你好AES\0");
	cout << "加密后：" << enc << endl << "大小:" << enc.size() << endl << endl;
	string dec = aes.decryption(enc);
	cout << "解密后：" << dec << endl << "大小:" << dec.size() << endl << endl;
}

void MyTest_Y::Test::SharedMemoryTest()
{
	ShareMemory shared{ "123" };
	void* ch = shared.getMapShm();
	memcpy(ch, "abcd\0", 5);
	printf("%s\n", ch);
	cout << shared.unmapshm() << endl;
	cout << shared.unmapshm() << endl;
	ShareMemory shared2{ 100 };
	cout << shared.delshm() << endl;
	cout << shared.delshm() << endl;
	
}

void MyTest_Y::Test::SocketText()
{
	M_SOCKET::SocketListener sock{ 8888 };
	std::shared_ptr<M_SOCKET::SocketService> ser;
	do
	{
		if (sock.getError() == M_SOCKET::STATUS_TYPE::S_TTIMEOUT)
			cout << "等待连接超时" << endl;
		ser = sock.acceptReq();
	} while (sock.getError() == M_SOCKET::STATUS_TYPE::S_TTIMEOUT);
	if (!(sock.getError() == M_SOCKET::STATUS_TYPE::S_SUCCESSFUL))
		return;
	cout << "连接成功" << endl;
	string msg;
	do
	{
		if (ser->getError() == M_SOCKET::STATUS_TYPE::S_TTIMEOUT)
			cout << "等待数据超时" << endl;
		msg = ser->recvMsg(3);
	} while (msg.empty() && ser->getError() == M_SOCKET::STATUS_TYPE::S_TTIMEOUT);

	if (!ser->sendMsg("hello world", 3))
	{
		cout << "send err" << endl;
	}
}

void MyTest_Y::Test::PoolTest()
{
	SecureTransportPool work;
	work.reset(10);
	for (size_t i = 0; i < 100; i++)
	{
		// SecureTransport task{nullptr};
		// work.addTask(task);
	}
	work.waitAllTasks();
}

void MyTest_Y::Test::clientTest()
{
	SecureTransportClient cli;
	cli.run();
}
