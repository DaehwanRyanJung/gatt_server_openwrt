/*
 * Digest Authentification
 */

#include "DigestAuth.h"
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <random>
#include <algorithm>

#include "NtcLogger.h"

// calculate SHA256 hash
static std::string calSHA256Hash(const std::string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// calculate md5 hash
static std::string calMD5Hash(const std::string str)
{
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, str.c_str(), str.size());
    MD5_Final(hash, &md5);
    std::stringstream ss;
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// calculate HA1 hash on Digest Auth
//
// HA1 = SHA256/MD5(username:realm:password)
//
// algorithm: SHA256 or MD5 (default: SHA256)
static std::string getDigestHA1(const std::string &algorithm, const std::string &username, const std::string &realm, const std::string &password)
{
    std::string digestStr = username + ":" + realm + ":" + password;
    if (algorithm == "MD5") {
        return calMD5Hash(digestStr);
    }
    return calSHA256Hash(digestStr);
}

// calculate HA2 hash on Digest Auth
//
// HA2 = SHA256/MD5(method:digestURI)
//
// algorithm: SHA256 or MD5 (default: SHA256)
static std::string getDigestHA2(const std::string &algorithm, const std::string &method, const std::string &uri)
{
    std::string digestStr = method + ":" + uri;
    if (algorithm == "MD5") {
        return calMD5Hash(digestStr);
    }
    return calSHA256Hash(digestStr);
}

// calculate response hash on Digest Auth
//
// response = SHA256/MD5(HA1:nonce:HA2)
//
// algorithm: SHA256 or MD5 (default: SHA256)
static std::string getDigestResponse(const std::string &algorithm, const std::string &ha1, const std::string &nonce, const std::string &ha2)
{
    std::string digestStr = ha1 + ":" + nonce + ":" + ha2;
    if (algorithm == "MD5") {
        return calMD5Hash(digestStr);
    }
    return calSHA256Hash(digestStr);
}

// generate random AlphaNumeric string
//
// length: random AlphaNumeric string length to generate
std::string genRandomAlphaNumStr(size_t length)
{
    std::string possibleChars("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    size_t possibleCharsLen = possibleChars.length();
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> dist(0, possibleCharsLen-1);

    unsigned char key;
    std::string ret;
    ret.reserve(length);
    for (size_t i=0; i < length; i++) {
        if (RAND_bytes(&key, sizeof(key)) > 0) {
            ret += possibleChars[key%possibleCharsLen];
        } else {
            ret+= possibleChars[dist(engine)];
        }
    }
    return ret;
}

// check whether response hash is valid
bool verifyDigestHash(const std::string &username, const std::string &password,
                      const std::string &realm, const std::string &method,
                      const std::string &uri, const std::string &nonce,
                      const std::string &respHash)
{
    std::string calHA1 = getDigestHA1("SHA256", username, realm, password);
    std::string calHA2 = getDigestHA2("SHA256", method, uri);
    std::string calHash = getDigestResponse("SHA256", calHA1, nonce, calHA2);

    log(LOG_INFO, "calHash=[%s]", calHash.c_str());
    return respHash == calHash;
}
