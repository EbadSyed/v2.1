#!/bin/sh

# Translate given .pem files into .der, followed by conversion into formats
# that our TLS stacks expect.

set -e
BASENAME=`basename $0`

PrintUsage () {
  echo "Usage: $BASENAME ca-cert.pem ca-key.pem ca-pkcs8-key.pem device-cert.pem"
  exit 1
}

if [ $# -lt "4" ]
then
  echo "Error: too few arguments (have $#, need at least 6)"
  PrintUsage
fi

CACERT=$1
CAKEY=$2
CAPKCS8KEY=$3
DEVICECERT=$4

# Convert into format Silicon Labs TLS stack expects
case $( uname -s ) in
Linux)
  ./translate-silabs_linux --declare_ca new --file $CACERT > $CACERT.silabs.c
  ./translate-silabs_linux --declare --keyfile $CAPKCS8KEY > $CAPKCS8KEY.silabs.c
  ./translate-silabs_linux --declare --file $DEVICECERT > $DEVICECERT.silabs.c
  ;;
*)
  ./translate-silabs_osx --declare_ca new --file $CACERT > $CACERT.silabs.c
  ./translate-silabs_osx --declare --keyfile $CAPKCS8KEY > $CAPKCS8KEY.silabs.c
  ./translate-silabs_osx --declare --file $DEVICECERT > $DEVICECERT.silabs.c
  ;;
esac

# Convert CA and key into DER
openssl x509 -outform der -inform pem -in $CACERT -out $CACERT.der
openssl ec   -outform der -inform pem -in $CAKEY -out $CAKEY.der
openssl ec   -outform der -inform pem -in $CAPKCS8KEY -out $CAPKCS8KEY.der

# Convert device certificate into DER
openssl x509 -outform der -inform pem -in $DEVICECERT -out $DEVICECERT.der

# Convert into format mbed TLS stack expects
./translate-mbedtls.py --in $CACERT.der --out $CACERT.mbedtls.c
./translate-mbedtls.py --in $CAKEY.der --out $CAKEY.mbedtls.c
./translate-mbedtls.py --in $DEVICECERT.der --out $DEVICECERT.mbedtls.c

# Start writing the sample file

# Remove it if it exists
rm -rf sample-dotdot-certificates.c

echo "/*\
      \n * File: sample-dotdot-certificates.c\
      \n * Description: translated certificates for DTLS\
      \n * Inputs: $CACERT $CAKEY $CAPKCS8KEY $DEVICECERT\
      \n */\
      \n" >> sample-dotdot-certificates.c

echo "#include \"stack/core/ember-stack.h\"\
      \n#include \"app/thread/plugin/dtls-auth-params/certificate-parameters.h\"\
      \n" >> sample-dotdot-certificates.c

echo "//----------------------------------------------------------------\
      \n#ifdef EMBER_MBEDTLS_STACK\
      \n" >> sample-dotdot-certificates.c

echo "const char mbedtls_x509_ca_cert_der[] = {" >> sample-dotdot-certificates.c

cat $CACERT.mbedtls.c >> sample-dotdot-certificates.c

echo "\n};\
      \n\nconst size_t mbedtls_x509_ca_cert_der_len = sizeof(mbedtls_x509_ca_cert_der);\
      \n" >> sample-dotdot-certificates.c

echo "const char mbedtls_x509_server_cert_der[] = {" >> sample-dotdot-certificates.c

cat $DEVICECERT.mbedtls.c >> sample-dotdot-certificates.c

echo "\n};\
      \n\nconst size_t mbedtls_x509_server_cert_der_len = sizeof(mbedtls_x509_server_cert_der);\
      \n" >> sample-dotdot-certificates.c

echo "const char mbedtls_x509_server_key_der[] = {" >> sample-dotdot-certificates.c

cat $CAKEY.mbedtls.c >> sample-dotdot-certificates.c

echo "\n};\
      \n\nconst size_t mbedtls_x509_server_key_der_len = sizeof(mbedtls_x509_server_key_der);\
      \n" >> sample-dotdot-certificates.c

echo "const char mbedtls_x509_client_cert_der[] = {" >> sample-dotdot-certificates.c

cat $DEVICECERT.mbedtls.c >> sample-dotdot-certificates.c

echo "\n};\
      \n\nconst size_t mbedtls_x509_client_cert_der_len = sizeof(mbedtls_x509_client_cert_der);\
      \n" >> sample-dotdot-certificates.c

echo "const char mbedtls_x509_client_key_der[] = {" >> sample-dotdot-certificates.c

cat $CAKEY.mbedtls.c >> sample-dotdot-certificates.c

echo "\n};\
      \n\nconst size_t mbedtls_x509_client_key_der_len = sizeof(mbedtls_x509_client_key_der);\
      \n" >> sample-dotdot-certificates.c

echo "//----------------------------------------------------------------\
      \n\n#else // Below is ! EMBER_MBEDTLS_STACK\
      \n" >> sample-dotdot-certificates.c

echo "#include \"stack/ip/tls/tls.h\"\
      \n#include \"stack/ip/tls/certificate.h\"\
      \n" >> sample-dotdot-certificates.c

cat $CACERT.silabs.c >> sample-dotdot-certificates.c

cat $DEVICECERT.silabs.c >> sample-dotdot-certificates.c

cat $CAPKCS8KEY.silabs.c >> sample-dotdot-certificates.c

echo "\n//----------------------------------------------------------------\
      \nconst CertificateAuthority *emMyAuthorities[] = {\
      \n  NULL\
      \n};\
      \nconst uint8_t *emMyPrivateKey;\
      \nconst uint8_t *emMyRawCertificate;\
      \nuint16_t emMyRawCertificateLength;\
      \n\nuint8_t *emMyHostname = NULL;\
      \nuint16_t emMyHostnameLength = 0;\
      \n\n// No pre-shared key implementation on the application currently.\
      \nconst SharedKey *emMySharedKey = NULL;\
      \n//----------------------------------------------------------------\
      \n\n#endif // EMBER_MBEDTLS_STACK\n" >> sample-dotdot-certificates.c



# Cleanup
rm -rf $CACERT.silabs.c
rm -rf $CAPKCS8KEY.silabs.c
rm -rf $DEVICECERT.silabs.c

rm -rf $CACERT.der
rm -rf $CAKEY.der
rm -rf $CAPKCS8KEY.der
rm -rf $DEVICECERT.der
rm -rf $CACERT.mbedtls.c
rm -rf $CAKEY.mbedtls.c
rm -rf $DEVICECERT.mbedtls.c
