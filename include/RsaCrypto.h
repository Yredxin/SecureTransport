#ifndef OBJECT_RSACRYPTO_Y
#define OBJECT_RSACRYPTO_Y
#include <string>
#include <memory>
#include "Hash.h"

constexpr char* DEFAULT_PRIVATE_KEY_PATH	= "private.pen";
constexpr char* DEFAULT_PUBLIC_KEY_PATH		= "public.pen";
constexpr int DEFAULT_KEY_BITS_LEN			= 1024;

/*
extern "C"
{
#include <openssl/applink.c>
};
*/
namespace M_CRYPTO 
{
	using std::string;
	class RsaCrypto;
}
class M_CRYPTO::RsaCrypto
{
public:
	// 重新生成密钥
	explicit RsaCrypto(int keyBitLen = DEFAULT_KEY_BITS_LEN, const string& privatePath= DEFAULT_PRIVATE_KEY_PATH, const string& publicPath = DEFAULT_PUBLIC_KEY_PATH);

	// 文件中打开已保存密钥
	explicit RsaCrypto(const string& keyFile, bool isPrivate);

	// 字符串中打开公钥
	explicit RsaCrypto(const string& publicKey);

	// 获取公私钥
	string getPrivateKey();
	string getPublicKey();

	// 公钥加密
	string publicEncrypt(const string& data);

	// 私钥解密
	string privateDecrypt(const string& data);

	// 签名
	string rsaSign(const string& signData, HashType type = HashType::HASH_SHA224);

	// 校验签名
	bool rsaVerify(const string& sourceData, const string& signData,HashType type = HashType::HASH_SHA224);

	// ~RsaCrypto();

private:
	class _RsaCrypto;
	// _RsaCrypto* _rsa;
	std::shared_ptr<_RsaCrypto> _rsa;
};

#endif // !OBJECT_RSACRYPTO_Y

