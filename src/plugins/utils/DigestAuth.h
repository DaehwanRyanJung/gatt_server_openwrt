#ifndef DIGESTAUTH_H_10280704012024
#define DIGESTAUTH_H_10280704012024
/*
 * Digest Authentication
 */

#include <string>

// generate random hex string
std::string genRandomAlphaNumStr(size_t len);

// check whether response hash is valid
bool verifyDigestHash(const std::string &username, const std::string &password,
                      const std::string &realm, const std::string &method,
                      const std::string &uri, const std::string &nonce,
                      const std::string &respHash);

#endif // DIGESTAUTH_H_10280704012024
