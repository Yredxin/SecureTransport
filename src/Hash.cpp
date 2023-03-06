#include "Hash.h"
#include <iostream>
#include <sstream>
using std::string;


struct M_CRYPTO::Hash::_Hash
{
	HashType _type;
	_Hash(HashType type) :_type{ type } {};

	// 初始化  
	inline int initMD5() { return MD5_Init(&_md5C); };
	inline int initSHA() { return SHA_Init(&_shaC); };
	inline int initSHA224() { return SHA224_Init(&_sha224C); };
	inline int initSHA256() { return SHA256_Init(&_sha256C); };
	inline int initSHA384() { return SHA384_Init(&_sha384C); };
	inline int initSHA512() { return SHA512_Init(&_sha512C); };

	// 数据添加
	inline int updateMD5(const string& data) { return MD5_Update(&_md5C, data.data(), data.size()); };
	inline int updateSHA(const string& data) { return SHA_Update(&_shaC, data.data(), data.size()); };
	inline int updateSHA224(const string& data) { return SHA224_Update(&_sha224C, data.data(), data.size()); };
	inline int updateSHA256(const string& data) { return SHA256_Update(&_sha256C, data.data(), data.size()); };
	inline int updateSHA384(const string& data) { return SHA384_Update(&_sha384C, data.data(), data.size()); };
	inline int updateSHA512(const string& data) { return SHA512_Update(&_sha512C, data.data(), data.size()); };

	// 计算哈希
	string finalMD5()
	{
		unsigned char result_hash[MD5_DIGEST_LENGTH] = { 0 };
		MD5_Final(result_hash, &_md5C);
		return resultHex(result_hash, MD5_DIGEST_LENGTH);
	};
	string finalSHA()
	{
		unsigned char result_hash[SHA_DIGEST_LENGTH] = { 0 };
		SHA_Final(result_hash, &_shaC);
		return resultHex(result_hash, SHA_DIGEST_LENGTH);
	};

	string finalSHA224()
	{
		unsigned char result_hash[SHA224_DIGEST_LENGTH] = { 0 };
		SHA224_Final(result_hash, &_sha224C);
		return resultHex(result_hash, SHA224_DIGEST_LENGTH);
	};
	string finalSHA256()
	{
		unsigned char result_hash[SHA256_DIGEST_LENGTH] = { 0 };
		SHA256_Final(result_hash, &_sha256C);
		return resultHex(result_hash, SHA256_DIGEST_LENGTH);
	};
	string finalSHA384()
	{
		unsigned char result_hash[SHA384_DIGEST_LENGTH] = { 0 };
		SHA384_Final(result_hash, &_sha384C);
		return resultHex(result_hash, SHA384_DIGEST_LENGTH);
	};
	string finalSHA512()
	{
		unsigned char result_hash[SHA512_DIGEST_LENGTH] = { 0 };
		SHA512_Final(result_hash, &_sha512C);
		return resultHex(result_hash, SHA512_DIGEST_LENGTH);
	};

private:
	string resultHex(const unsigned char* result_hash, const int len)
	{
		std::stringstream result;
		for (size_t i = 0; i < len; i++)
		{
			result << std::hex << static_cast<int>(result_hash[i]);
		}
		return result.str();
	};
	MD5_CTX _md5C;
	SHA_CTX _shaC;
	SHA256_CTX _sha224C;
	SHA256_CTX _sha256C;
	SHA512_CTX _sha384C;
	SHA512_CTX _sha512C;
};

/**
 * .
 * 
 * \brief				初始化hash
 * \param type			指定hash算法，存放于M_CRYPTO::HashType，默认sha224运算
 */
M_CRYPTO::Hash::Hash(HashType type)
	:_hash{ new _Hash{type} }
{
	switch (type)
	{
	case M_CRYPTO::HashType::HASH_MD5:
		_hash->initMD5();
		break;
	case M_CRYPTO::HashType::HASH_SHA:
		_hash->initSHA();
		break;
	case M_CRYPTO::HashType::HASH_SHA224:
		_hash->initSHA224();
		break;
	case M_CRYPTO::HashType::HASH_SHA256:
		_hash->initSHA256();
		break;
	case M_CRYPTO::HashType::HASH_SHA384:
		_hash->initSHA384();
		break;
	case M_CRYPTO::HashType::HASH_SHA512:
		_hash->initSHA512();
		break;
	}
}

/**
 * .
 * 
 * \brief				持续添加数据
 * \param data			需要添加的数据
 */
void M_CRYPTO::Hash::addData(string& data)
{
	switch (_hash->_type)
	{
	case M_CRYPTO::HashType::HASH_MD5:
		_hash->updateMD5(data);
		break;
	case M_CRYPTO::HashType::HASH_SHA:
		_hash->updateSHA(data);
		break;
	case M_CRYPTO::HashType::HASH_SHA224:
		_hash->updateSHA224(data);
		break;
	case M_CRYPTO::HashType::HASH_SHA256:
		_hash->updateSHA256(data);
		break;
	case M_CRYPTO::HashType::HASH_SHA384:
		_hash->updateSHA384(data);
		break;
	case M_CRYPTO::HashType::HASH_SHA512:
		_hash->updateSHA512(data);
		break;
	}
}

/**
 * .
 * 
 * \brief				计算hash结果
 * \return				返回hash结果
 */
string M_CRYPTO::Hash::result()
{
	switch (_hash->_type)
	{
	case M_CRYPTO::HashType::HASH_MD5:
		return _hash->finalMD5();
		break;
	case M_CRYPTO::HashType::HASH_SHA:
		return _hash->finalSHA();
		break;
	case M_CRYPTO::HashType::HASH_SHA224:
		return _hash->finalSHA224();
		break;
	case M_CRYPTO::HashType::HASH_SHA256:
		return _hash->finalSHA256();
		break;
	case M_CRYPTO::HashType::HASH_SHA384:
		return _hash->finalSHA384();
		break;
	case M_CRYPTO::HashType::HASH_SHA512:
		return _hash->finalSHA512();
		break;
	}
	return string();
}
