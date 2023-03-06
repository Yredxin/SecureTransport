#include "SecureTransportPool.h"
#include <iostream>
#include <chrono>

SecureTransportPool::SecureTransportPool(int threadNum) :_bRunning{ true }
{
	if (threadNum < 5)
		threadNum = 5;
	for (size_t i = 0; i < threadNum; i++)
	{
		// std::shared_ptr<std::thread> td = std::make_shared<std::thread>(std::bind(&SecureTransportPool::worker, this));
		std::shared_ptr<std::thread> td{ new std::thread{std::bind(&SecureTransportPool::worker, this) } };
		_thread.push_back(td);
	}
}

SecureTransportPool::~SecureTransportPool()
{
	stop();
	removeAllTasks();
}

/**
 * .
 * 
 * \brief				初始化线程数
 * \param threadNum		线程数量
 */
void SecureTransportPool::reset(int threadNum)
{
	stop();
	if (threadNum < 5)
		threadNum = 5;
	_bRunning = true;
	for (size_t i = 0; i < threadNum; i++)
	{
		// std::shared_ptr<std::thread> td = std::make_shared<std::thread>(std::bind(&SecureTransportPool::worker, this));
		std::shared_ptr<std::thread> td{ new std::thread{std::bind(&SecureTransportPool::worker, this) } };
		_thread.push_back(td);
	}
}

/**
 * .
 * 
 * \brief				添加任务
 * \param test			任务
 */
void SecureTransportPool::addTask(std::unique_ptr<SecureTransport> task)
{
	std::shared_ptr<SecureTransport> workTask = std::move(task);
	{
		std::unique_lock<std::mutex> guard(_mutex);
		_taskList.push_back(workTask);
	}
	_cond.notify_one();
}

/**
 * .
 * 
 * \brief				等待所有任务结束
 */
void SecureTransportPool::waitAllTasks()
{
	while (true)
	{
		{
			std::unique_lock<std::mutex> guard(_mutex);
			if (_taskList.empty())
				return;
		}
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

/**
 * .
 * 
 * \brief				删除所有任务
 */
void SecureTransportPool::removeAllTasks()
{
	std::unique_lock<std::mutex> guard(_mutex);
	for (auto &task : _taskList)
		task.reset();
	_taskList.clear();
}

/**
 * .
 * 
 * \brief				等待所有线程结束工作
 */
void SecureTransportPool::stop()
{
	_bRunning = false;
	_cond.notify_all();

	for (auto& iter : _thread)
		if (iter->joinable())
			iter->join();
}

/**
 * .
 * 
 * \brief				任务处理
 */
void SecureTransportPool::worker()
{
	std::shared_ptr<SecureTransport> task;
	std::cout << "工作线程：" << std::this_thread::get_id() << "开始工作!!!" << std::endl;
	while (true)
	{
		{
			std::unique_lock<std::mutex> guard(_mutex);
			while (_taskList.empty())
			{
				if (!_bRunning)
					break;

				// 没有任务，将线程挂起，并释放guard锁
				// 获取任务，将加锁
				_cond.wait(guard);
			}
			if (!_bRunning)
				break;

			task = _taskList.front();
			_taskList.pop_front();
		}
		if (task == nullptr)
			continue;

		task->run();
		task.reset();
	}
	std::cout << "工作线程：" << std::this_thread::get_id() << "结束工作!!!" << std::endl;
}
