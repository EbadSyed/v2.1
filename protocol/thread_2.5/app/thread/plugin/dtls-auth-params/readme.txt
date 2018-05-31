-------------------------------------------------------------------------------
Creating custom X.509 certificates for use with the DTLS authentication (dtls-auth-params) plugin:
-------------------------------------------------------------------------------

The following examples are intended for a Linux or any other Unix (Mac OS X, etc.)
environment that has the openssl libraries installed.  Please seek documentation
to create .pem files if you're running a different environment.

-------------------------------------------------------------------------------
To create your own CA authority:

1. Generate a CA private key using an EC curve:

  openssl ecparam -genkey -name secp256r1 -out ca-key.pem

2. Create a CA certificate using the above private key:

  openssl req -x509 -new -SHA256 -nodes -key ca-key.pem -days 3650 -out ca-cert.pem

3. Create a certificate signing request:

  openssl req -new -SHA256 -key ca-key.pem -nodes -out ca-csr.pem

4. You can verify that the CSR has the details that you specified, by entering:

  openssl req -in ca-csr.pem -noout -text

5. For the purposes of the DTLS plugin, please also generate an unencrypted
   pkcs8 format key as follows:

  openssl pkcs8 -topk8 -nocrypt -in ca-key.pem -out ca-pkcs8-key.pem

-------------------------------------------------------------------------------

To create your own device certificate (in the format that DotDot applications
require), use the files generated above, or from another CA authority, as follows:

  openssl x509 -req -SHA256 -days 3650 -in ca-csr.pem -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial -out device-cert.pem

-------------------------------------------------------------------------------

Finally, the files generated above need to be converted into a .c file that can
be consumed by a Silicon Labs application that uses the DTLS Authentication plugin.

(Note: Please save all your .pem files on a hard drive.)

To do this, run the following command, after changing to the directory app/thread/plugin/dtls-auth-params/:

./translate-certificates.sh ca-cert.pem ca-key.pem ca-pkcs8-key.pem device-cert.pem

This should result in a .c file that is named translated-certificates.c

This file can be renamed if desired, and can be used as a plugin option to the DTLS authentication plugin.
