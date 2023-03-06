#ifndef OBJECT_POOL_Y
#include <memory>
#include <mutex>
#include <vector>
#include <list>
#include <thread>
#include <condition_variable>
#include "SecureTransportServer.h"

class SecureTransportPool
{
public:
	SecureTransportPool(int threadNum = 5);
	~SecureTransportPool();
	SecureTransportPool(const SecureTransportPool&) = delete;
	SecureTransportPool& operator=(const SecureTransportPool&) = delete;

public:
	void reset(int threadNum = 5);
	void addTask(std::unique_ptr<SecureTransport> task);
	void waitAllTasks();
	void removeAllTasks();
	void stop();
	void worker();
private:
	std::mutex _mutex;
	std::condition_variable _cond;
	std::list<std::shared_ptr<SecureTransport>> _taskList;
	std::vector< std::shared_ptr<std::thread>> _thread;
	bool _bRunning;
};

#endif // !OBJECT_POOL_Y



