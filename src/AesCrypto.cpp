#include "AesCrypto.h"
#include <openssl/aes.h>
#include <iostream>

// 内部类
class M_CRYPTO::AesCrypto::_AesCrypto
{
public:
	_AesCrypto(const AesKeyLen keyLen, const string& key = "");
	string encryption(const string& data, bool isEnc);

private:
	string createKey();
	std::unique_ptr<unsigned char> createIVEC();

private:
	int _keyLen;
	AES_KEY _decKey;
	AES_KEY _encKey;
};

// ====================================内部类实现============================================

/**
 * .
 * 
 * \brief				初始化加解密密钥
 * \param keyLen		密钥长度
 * \param key			key 长度不等于 16或24或32 则自动创建
 */
M_CRYPTO::AesCrypto::_AesCrypto::_AesCrypto(const AesKeyLen keyLen, const string& key)
{
	switch (keyLen)
	{
	case M_CRYPTO::AesKeyLen::PRIMARY_LEN:
		_keyLen = 16;
		break;
	case M_CRYPTO::AesKeyLen::INTERMEDIATE_LEN:
		_keyLen = 24;
		break;
	case M_CRYPTO::AesKeyLen::ADVANCE_LEN:
		_keyLen = 32;
		break;
	}

	string k;
	if (key.size() != 16 || key.size() != 24 || key.size() != 32)
		k = createKey();
	else
		k = key;
	AES_set_encrypt_key((unsigned char*)k.c_str(), _keyLen * 8, &_encKey);
	AES_set_decrypt_key((unsigned char*)k.c_str(), _keyLen * 8, &_decKey);
}

/**
 * .
 * 
 * \brief				加解密
 * \param data			待加/解密的数据
 * \param isEnc			是否为加密
 * \return				返回加/解密后的字符串
 */
std::string M_CRYPTO::AesCrypto::_AesCrypto::encryption(const string& data, bool isEnc)
{
	int enc = isEnc ? AES_ENCRYPT : AES_DECRYPT;
	// 输出和输入数据长度一致，但是加密后的数据输出需要添加一个'\0' 且为16倍数
	size_t len;			
	if (data.size() % AES_BLOCK_SIZE == 0)
		len = data.size() + 1;
	else
		len = ((data.size() + 1) / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
	std::unique_ptr<unsigned char> out{ new unsigned char[len]};
	AES_KEY* key = isEnc ? &_encKey : &_decKey;
	// 生成初始化向量 ivec
	std::unique_ptr<unsigned char> ivec = createIVEC();
	AES_cbc_encrypt((unsigned char*)data.c_str(), out.get(), len, (isEnc ? &_encKey : &_decKey), ivec.get(), enc);
	return isEnc ? string((char*)out.get(), len) : string((char*)out.get());
}

/**
 * .
 * 
 * \brief				生成自动密钥
 * \return				返回密钥
 */
std::string M_CRYPTO::AesCrypto::_AesCrypto::createKey()
{
	// return std::move(string("1234567887654321", _keyLen));
	return string("1234567887654321", _keyLen);
}

/**
 * .
 * 
 * \brief				生成初始化向量
 * \return				返回初始化向量
 */
std::unique_ptr<unsigned char> M_CRYPTO::AesCrypto::_AesCrypto::createIVEC()
{
	std::unique_ptr<unsigned char> ivec{ new unsigned char[AES_BLOCK_SIZE] };
	for (size_t i = 0; i < AES_BLOCK_SIZE; i++)
	{
		ivec.get()[i] = (AES_BLOCK_SIZE - i) + 'a';
	}
	return ivec;
}

// ====================================类实现============================================

M_CRYPTO::AesCrypto::AesCrypto(const AesKeyLen keyLen, const string& key)
	:_aes{ new _AesCrypto{keyLen, key} }
{
}

/**
 * .
 * 
 * \brief				对数据进行加密
 * \param data			需要加密的数据
 * \return				加密后的数据
 */
std::string M_CRYPTO::AesCrypto::encryption(const string& data)
{
	return _aes->encryption(data, true);
}

/**
 * .
 *
 * \brief				对数据进行解密
 * \param data			需要解密的数据
 * \return				解密后的数据
 */
std::string M_CRYPTO::AesCrypto::decryption(const string& data)
{
	return _aes->encryption(data, false);
}
