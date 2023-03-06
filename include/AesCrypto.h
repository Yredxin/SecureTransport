#ifndef OBJECT_AES_CRYPTO_Y
#define OBJECT_AES_CRYPTO_Y
#include <string>
#include <memory>

namespace M_CRYPTO 
{
	using std::string;
	enum class AesKeyLen :char;
	class AesCrypto;
}

/**
 * .
 * 
 * \brief				设置密钥长度
 * \param				PRIMARY_LEN			===> 密钥长度 16byte
 * \param				INTERMEDIATE_LEN	===> 密钥长度 24byte
 * \param				ADVANCE_LEN			===> 密钥长度 32byte
 */
enum class M_CRYPTO::AesKeyLen :char 
{
	PRIMARY_LEN			= 16,
	INTERMEDIATE_LEN	= 24,
	ADVANCE_LEN			= 32
};

class M_CRYPTO::AesCrypto
{
public:
	// 初始化，如果未传递key，或者key存在问题，则自动重新生成
	explicit AesCrypto(const AesKeyLen keyLen = AesKeyLen::PRIMARY_LEN, const string& key = "");

	// 加密
	string encryption(const string& data);

	// 解密
	string decryption(const string& data);

private:
	class _AesCrypto;
	std::shared_ptr<_AesCrypto> _aes;
	/*
	struct _AesCryptoDeleter final
	{
		constexpr _AesCryptoDeleter() noexcept = default;
		void operator()(_AesCrypto* p) const;
	};
	std::unique_ptr<_AesCrypto, _AesCryptoDeleter> _aes;
	*/
};

#endif // !OBJECT_AES_CRYPTO_Y

