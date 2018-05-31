// Silicon Labs TLS stack

#ifndef CERTIFICATE_PARAMETERS_H
#define CERTIFICATE_PARAMETERS_H

extern const CertificateAuthority testAuthority;
extern const CertificateAuthority exeginAuthority;
extern const CertificateAuthority newAuthority; // Custom defined

extern const uint8_t privateKey0[];
extern const uint8_t privateKey1[];
extern const uint8_t newPrivate[]; // Custom defined

extern const uint8_t rawCertificate0[];
extern const uint16_t rawCertificateSize0;

extern const uint8_t rawCertificate1[];
extern const uint16_t rawCertificateSize1;

extern const uint8_t newCertificate[]; // Custom defined
extern const uint16_t newCertificateSize; // Custom defined

// mbed TLS stack

// Server
extern const char mbedtls_x509_ca_cert_der[];
extern const size_t mbedtls_x509_ca_cert_der_len;

extern const char mbedtls_x509_server_cert_der[];
extern const size_t mbedtls_x509_server_cert_der_len;

extern const char mbedtls_x509_server_key_der[];
extern const size_t mbedtls_x509_server_key_der_len;

// Client
extern const char mbedtls_x509_client_cert_der[];
extern const size_t mbedtls_x509_client_cert_der_len;

extern const char mbedtls_x509_client_key_der[];
extern const size_t mbedtls_x509_client_key_der_len;

#endif // CERTIFICATE_PARAMETERS_H
