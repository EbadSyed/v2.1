#ifndef TEST_CERTIFICATE_H
#define TEST_CERTIFICATE_H

extern const CertificateAuthority testAuthority;
extern const CertificateAuthority silverSpringAuthority;
extern const CertificateAuthority g2hAuthority;
extern const CertificateAuthority atmelAuthority;
extern const CertificateAuthority tiAuthority;
extern const CertificateAuthority exeginAuthority;
extern const CertificateAuthority tiChainRootAuthority;
extern const CertificateAuthority mcaChainRootAuthority;

#ifdef HAVE_TLS_DHE_RSA
extern const RsaPrivateKey privateKey0;
extern const RsaPrivateKey privateKey1;
#else
extern const uint8_t privateKey0[];
extern const uint8_t privateKey1[];
#endif

extern const uint8_t rawCertificate0[];
extern const uint16_t rawCertificateSize0;

extern const uint8_t rawCertificate1[];
extern const uint16_t rawCertificateSize1;

extern const uint8_t rawCertificate2[];
extern const uint16_t rawCertificateSize2;
extern const EccPrivateKey privateKey2;

extern const uint8_t rawCertificate3[];
extern const uint16_t rawCertificateSize3;
extern const uint8_t privateKey3[];

#endif // TEST_CERTIFICATE_H
