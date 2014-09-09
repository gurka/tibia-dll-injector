#ifndef RSA_H_
#define RSA_H_

#include <gmp.h>

class RSA {
 public:
  RSA();
  ~RSA();

  void setPrivateKey(const char* p, const char* q);
  void decrypt(char* msg);

 private:
  typedef struct {
    mpz_t n;  // Modulus
    mpz_t e;  // Public Exponent
  } PublicKey;

  typedef struct {
    mpz_t n;  // Modulus
    mpz_t e;  // Public Exponent
    mpz_t d;  // Private Exponent
    mpz_t p;  // Starting prime p
    mpz_t q;  // Starting prime q
  } PrivateKey;

  PublicKey public_key;
  PrivateKey private_key;

  mpz_t n_;
  mpz_t d_;
};

#endif  // RSA_H_
