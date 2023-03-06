#include "ShareMemory.h"
#include <sys/ipc.h>
#include <sys/shm.h>

/**
 * .
 * 
 * \brief				根据key创建和打开共享内存
 * \param key			共享内存key
 * \param size			为0表示共享内存已存在，不为0则创建
 */
ShareMemory::ShareMemory(int key, int size)
	:_shmAddr{ nullptr },
	_shmid{ 0 }
{
	if (size != 0 || ((_shmid = shmget(key, size, 0)) < 0))
	{
		size = size > 0 ? size : DEFAULT_SIZE;
		_shmid = shmget(key, size, IPC_CREAT | 0664);
		if (_shmid < 0)
			return;
	}
	_shmAddr = shmat(_shmid, NULL, 0);
	if (_shmAddr == (void*)-1)
	{
		_shmAddr = nullptr;
	}
}

/**
 * .
 * 
 * \brief				根据绝对路径生成共享内存
 * \param path			绝对路径
 * \param size			共享内存大小
 */
ShareMemory::ShareMemory(std::string path, int size)
	:_shmAddr{ nullptr },
	_shmid{ 0 }
{
	key_t key = ftok(path.c_str(), 'a');
	if (key == -1)
		key = ftok("/home", 'a');

	if (size != 0 || ((_shmid = shmget(key, size, 0)) < 0))
	{
		size = size > 0 ? size : DEFAULT_SIZE;
		_shmid = shmget(key, size, IPC_CREAT | 0664);
		if (_shmid < 0)
			return;
	}
	_shmAddr = shmat(_shmid, NULL, 0);
	if (_shmAddr == (void*)-1)
	{
		_shmAddr = nullptr;
	}
}

/**
 * .
 * 
 * \return 是否脱离成功
 */
bool ShareMemory::unmapshm()
{
	if (!_shmAddr)
		return false;
	bool ret = shmdt(_shmAddr) == 0;
	_shmAddr = nullptr;
	return ret;
}

/**
 * .
 * 
 * \return 是否删除成功
 */
bool ShareMemory::delshm()
{
	if (_shmid < 0)
		return false;
	bool ret = shmctl(_shmid, IPC_RMID, NULL) == 0;
	_shmAddr = nullptr;
	return ret;
}
