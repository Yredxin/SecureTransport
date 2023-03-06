#ifndef OBJECT_SHARE_MEMORY_Y
#define OBJECT_SHARE_MEMORY_Y
#include <string>
constexpr int DEFAULT_SIZE = 1024;

class ShareMemory
{
public:
	ShareMemory(int key, int size = 0);
	ShareMemory(std::string path, int size = 0);

	// 获取共享内存
	/**
	 * .
	 * 
	 * \return				成功返回内存首地址，失败返回nullptr
	 */
	inline void* getMapShm() { return _shmAddr; };

	// 脱离共享内存
	bool unmapshm();

	// 删除共享内存
	bool delshm();

private:
	int _shmid;
	void* _shmAddr;
};

#endif // !OBJECT_SHARE_MEMORY_Y

