#include "RsaCrypto.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <memory>
#include <iostream>

// 内部类
class M_CRYPTO::RsaCrypto::_RsaCrypto
{
public:
	_RsaCrypto(const string& publicPath, const string& privatePath, int bits = DEFAULT_KEY_BITS_LEN);
	_RsaCrypto(const string& publicKey);
	~_RsaCrypto();

	// 读取文件密钥字符串
	string readFile(bool isPrivate);

	// 获取公钥
	inline RSA* getPublicKey();

	// 获取密钥
	inline RSA* getPrivateKey();

	// 获取缓冲区
	inline unsigned char* getBuffer();

	// 重置缓冲区
	inline void resetBuffer(int len);

private:
	// 从文件中获取密钥
	void fromFileToKey();

	// 生成密钥对
	void createKey(int bits);

private:
	RSA* _public;
	RSA* _private;
	string _publicPath;
	string _privatePath;
	std::unique_ptr<unsigned char> buffer;
	int bufLen;
};

// ====================================内部类实现============================================

/**
 * .
 * 
 * \brief				其中一个为空代表公私钥已存在，只做打开操作
 * \param publicPath	公钥文件路径
 * \param privatePath	私钥文件路径
 * \param bits			密钥长度
 */
M_CRYPTO::RsaCrypto::_RsaCrypto::_RsaCrypto(const string& publicPath, const string& privatePath, int bits)
	:_public{ nullptr },
	_private{ nullptr },
	_publicPath{ publicPath },
	_privatePath{ privatePath },
	bufLen{ bits / 8 }
{
	resetBuffer(bits / 8);
	if ((!publicPath.empty() && !privatePath.empty()) || 
		(publicPath.empty() && privatePath.empty()))
		createKey(bits);
	else
		fromFileToKey();
}

/**
 * .
 * 
 * \brief				直接存储公钥
 * \param publicKey		公钥
 */
M_CRYPTO::RsaCrypto::_RsaCrypto::_RsaCrypto(const string& publicKey)
	:_public{ nullptr }, _private{ nullptr }
{
	if (publicKey.empty())
	{
		return;
	}
	BIO* bio = BIO_new_mem_buf(publicKey.c_str(), publicKey.size());
	if (!bio)
	{
		return;
	}
	PEM_read_bio_RSAPublicKey(bio, &_public, NULL, NULL);
	if (bio)
		BIO_free(bio);
	bio = nullptr;
}

/**
 * .
 * 
 * \brief				资源释放
 */
M_CRYPTO::RsaCrypto::_RsaCrypto::~_RsaCrypto()
{
	if (_public)
		RSA_free(_public);
	if (_private)
		RSA_free(_private);
	_public = nullptr;
	_private = nullptr;
}

/**
 * .
 * 
 * \brief				读取文件中的key，格式为base64
 * \param isPrivate		是否为私钥
 * \return				返回base54格式的key
 */
std::string M_CRYPTO::RsaCrypto::_RsaCrypto::readFile(bool isPrivate)
{
	std::ifstream in{ (isPrivate ? _privatePath : _publicPath) };
	if (!in.is_open())
	{
		return string();
	}
	std::stringstream key{};
	key << in.rdbuf();
	in.close();
	return key.str();
}

/**
 * .
 *
 * \brief				获取公钥
 * \return				返回公钥指针
 */
inline RSA* M_CRYPTO::RsaCrypto::_RsaCrypto::getPublicKey()
{
	return _public;
}

/**
 * .
 *
 * \brief				获取私钥
 * \return				返回私钥指针
 */
inline RSA* M_CRYPTO::RsaCrypto::_RsaCrypto::getPrivateKey()
{
	return _private;
}

/**
 * .
 * 
 * \brief				获取缓冲区
 * \return				返回缓冲区指针
 */
inline unsigned char* M_CRYPTO::RsaCrypto::_RsaCrypto::getBuffer()
{
	return buffer.get();
}

/**
 * .
 * 
 * \brief				重置缓冲区
 */
inline void M_CRYPTO::RsaCrypto::_RsaCrypto::resetBuffer(int len)
{
	buffer.reset(new unsigned char[len]);
	bufLen = len;
	memset(buffer.get(), 0, bufLen);
}

/**
 * .
 * 
 * \brief				从文件中获取已存在的key，如果读取文件失败，则创建公私钥
 */
void M_CRYPTO::RsaCrypto::_RsaCrypto::fromFileToKey()
{
	string path = _publicPath.empty() ? _privatePath : _publicPath;
	BIO* bio = BIO_new_file(path.data(), "r");
	if (!bio)
	{
		_publicPath = DEFAULT_PUBLIC_KEY_PATH;
		_privatePath = DEFAULT_PRIVATE_KEY_PATH;
		createKey(DEFAULT_KEY_BITS_LEN);
		return;
	}
	if (_publicPath.empty())
		PEM_read_bio_RSAPrivateKey(bio, &_private, NULL, NULL);
	else
		PEM_read_bio_RSAPublicKey(bio, &_public, NULL, NULL);
	if (bio)
		BIO_free(bio);
	bio = nullptr;
}

/**
 * .
 * 
 * \brief				创建公私钥
 */
void M_CRYPTO::RsaCrypto::_RsaCrypto::createKey(int bits)
{
	RSA* rsa = RSA_new();
	BIGNUM* e = BN_new();
	BN_set_word(e, 12345);
	RSA_generate_key_ex(rsa, bits, e, NULL);
	BN_free(e);
	// 提取公私钥
	_public = RSAPublicKey_dup(rsa);
	_private = RSAPrivateKey_dup(rsa);
	// 将公私钥写入文件
	BIO* priBio = BIO_new_file(_privatePath.data(), "w");
	BIO* pubBio = BIO_new_file(_publicPath.data(), "w");
	PEM_write_bio_RSAPrivateKey(priBio, rsa, NULL, NULL, 0, NULL, NULL);
	PEM_write_bio_RSAPublicKey(pubBio, rsa);
	if (priBio)
		BIO_free(priBio);
	if (pubBio)
		BIO_free(pubBio);
	if (rsa)
		RSA_free(rsa);
	priBio = nullptr;
	pubBio = nullptr;
	rsa = nullptr;
}

//=======================================加解密类实现=========================================

/**
 * .
 * 
 * \brief				重新生成密钥
 * \param keyBitLen		密钥长度
 * \param privatePath	私钥存储路径
 * \param publicPath	公钥存储路径
 */
M_CRYPTO::RsaCrypto::RsaCrypto(int keyBitLen, const string& privatePath, const string& publicPath)
	:_rsa{ new _RsaCrypto{ publicPath, privatePath, keyBitLen } }
{
}

/**
 * .
 * 
 * \brief				打开指定密钥
 * \param keyFile		密钥文件
 * \param isPrivate		是否为私钥
 */
M_CRYPTO::RsaCrypto::RsaCrypto(const string& keyFile, bool isPrivate)
{
	if (isPrivate)
		_rsa.reset(new _RsaCrypto{ string(), keyFile });
	else
		_rsa.reset(new _RsaCrypto{ keyFile, string() });
	// _rsa = isPrivate ? (new _RsaCrypto{ string(), keyFile }) : (new _RsaCrypto{ keyFile, string() });
}

/**
 * .
 * 
 * \brief				字符串中读取公钥
 * \param publicKey		公钥字符串
 */
M_CRYPTO::RsaCrypto::RsaCrypto(const string& publicKey)
{
	_rsa.reset(new _RsaCrypto{ publicKey });
}

/**
 * .
 * 
 * \brief				析构
 */
/*
M_CRYPTO::RsaCrypto::~RsaCrypto()
{
	if (_rsa)
		delete _rsa;
	_rsa = nullptr;
}
*/

/**
 * .
 * 
 * \brief				读取私钥文件
 * \return				返回私钥base64编码格式字符串
 */
std::string M_CRYPTO::RsaCrypto::getPrivateKey()
{
	return _rsa->readFile(true);
}

/**
 * .
 *
 * \brief				读取公钥文件
 * \return				返回公钥base64编码格式字符串
 */
std::string M_CRYPTO::RsaCrypto::getPublicKey()
{
	return _rsa->readFile(false);
}

/**
 * .
 * 
 * \brief				公钥加密
 * \param data			需加密数据，明文
 * \return				返回加密后的数据，密文
 */
std::string M_CRYPTO::RsaCrypto::publicEncrypt(const string& data)
{
	RSA* pubRsa = _rsa->getPublicKey();
	size_t len = RSA_size(pubRsa);
	if (data.size() <= 0 || data.size() > len - 11)
	{
		return string();
	}
	
	_rsa->resetBuffer(len);
	if (RSA_public_encrypt(data.size(),
		(unsigned char*)data.c_str(),
		_rsa->getBuffer(),
		pubRsa, RSA_PKCS1_PADDING) < 0)
	{
		return string();
	}
	string cipher((char*)_rsa->getBuffer(), len);
	return cipher;
}

/**
 * .
 * 
 * \brief				私钥解密
 * \param data			待解密数据，密文
 * \return				返回解密后的数据，明文
 */
std::string M_CRYPTO::RsaCrypto::privateDecrypt(const string& data)
{
	RSA* priRsa = _rsa->getPrivateKey();
	size_t len = RSA_size(priRsa);
	if (data.size() != len)
	{
		return string();
	}
	_rsa->resetBuffer(len);
	if ((len = RSA_private_decrypt(data.size(),
		(unsigned char*)data.c_str(),
		_rsa->getBuffer(),
		priRsa, RSA_PKCS1_PADDING)) < 0)
	{
		return string();
	}
	string plaintext{ (char*)_rsa->getBuffer(), len };
	return plaintext;
}

/**
 * .
 *
 * \brief				签名
 * \param signData		待签名数据
 * \param type			运算方式枚举值，存放于M_CRYPTO::HashType，默认sha224运算
 * \return				返回签名后的哈希值
 */
std::string M_CRYPTO::RsaCrypto::rsaSign(const string& signData, HashType type)
{
	if (_rsa->getPrivateKey() == nullptr)
		return std::string();

	int rsaType = NID_sha224;
	switch (type)
	{
	case M_CRYPTO::HashType::HASH_MD5:
		rsaType = NID_md5;
		break;
	case M_CRYPTO::HashType::HASH_SHA:
		rsaType = NID_sha1;
		break;
	case M_CRYPTO::HashType::HASH_SHA224:
		rsaType = NID_sha224;
		break;
	case M_CRYPTO::HashType::HASH_SHA256:
		rsaType = NID_sha256;
		break;
	case M_CRYPTO::HashType::HASH_SHA384:
		rsaType = NID_sha384;
		break;
	case M_CRYPTO::HashType::HASH_SHA512:
		rsaType = NID_sha512;
		break;
	}
	unsigned int siglen;
	_rsa->resetBuffer(1024);
	RSA_sign(rsaType, 
		(unsigned char*)signData.c_str(), 
		signData.size(), 
		_rsa->getBuffer(),
		&siglen, 
		_rsa->getPrivateKey());
	string sigret((char*)_rsa->getBuffer(), siglen);
	return sigret;
}

/**
 * .
 * 
 * \brief				校验签名
 * \param sourceData	源数据
 * \param signData		签名后的hash值
 * \param type			运算方式枚举值，存放于M_CRYPTO::HashType，默认sha224运算，必须与签名时一致
 * \return				校验是否成功
 */
bool M_CRYPTO::RsaCrypto::rsaVerify(const string& sourceData, const string& signData, HashType type)
{
	if (_rsa->getPublicKey() == nullptr)
		return false;

	int rsaType = NID_sha224;
	switch (type)
	{
	case M_CRYPTO::HashType::HASH_MD5:
		rsaType = NID_md5;
		break;
	case M_CRYPTO::HashType::HASH_SHA:
		rsaType = NID_sha1;
		break;
	case M_CRYPTO::HashType::HASH_SHA224:
		rsaType = NID_sha224;
		break;
	case M_CRYPTO::HashType::HASH_SHA256:
		rsaType = NID_sha256;
		break;
	case M_CRYPTO::HashType::HASH_SHA384:
		rsaType = NID_sha384;
		break;
	case M_CRYPTO::HashType::HASH_SHA512:
		rsaType = NID_sha512;
		break;
	}
	if (RSA_verify(rsaType,
		(unsigned char*)sourceData.data(),
		sourceData.size(),
		(unsigned char*)signData.data(),
		signData.size(),
		_rsa->getPublicKey()) == 1)
	{
		return true;
	}
	return false;
}
