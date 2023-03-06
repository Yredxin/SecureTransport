#ifndef OBJECT_HASH_Y
#define OBJECT_HASH_Y
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <memory>
#include <string>

namespace M_CRYPTO 
{
	using std::string;
	enum class HashType : char;
	class Hash;
};

/**
 * .
 * 
 * \brief				hash算法选择
 */
enum class M_CRYPTO::HashType : char
{
	HASH_MD5 = 0,
	HASH_SHA,
	HASH_SHA224,
	HASH_SHA256,
	HASH_SHA384,
	HASH_SHA512
};

class M_CRYPTO::Hash
{
	// 哈希选择
public:
	// hash初始化
	explicit Hash(HashType type = HashType::HASH_SHA224);

	// 销毁内部类
	// ~Hash() { if (_hash == nullptr) return; delete _hash; _hash = nullptr; };

	// 持续添加数据
	void addData(string& data);

	// 返回hash结果
	string result();
private:
	struct _Hash;
	// _Hash* _hash;
	std::shared_ptr<_Hash> _hash;
};

#endif // !OBJECT_HASH_Y

