// Auto-generated trust anchors header
#ifndef TRUST_ANCHORS_H
#define TRUST_ANCHORS_H

#define _GSMROOT_CERTS_H_INCLUDED // This overrides the default GSMROOT_CERTS_H inclusion
// --------- TRUST ANCHORS OPTIONS ----------- (UNCOMMENT TO INCLUDE)
#define TRUST_ANCHORS_INCLUDE_AMAZONROOTCA1
// #define TRUST_ANCHORS_INCLUDE_BALTIMORECYBERTRUST
// #define TRUST_ANCHORS_INCLUDE_COMODORSA_CERTIFICATION_AUTHORITY
// #define TRUST_ANCHORS_INCLUDE_DIGICERTGLOBALROOTCA
// #define TRUST_ANCHORS_INCLUDE_DIGICERTGLOBALROOTG2
// #define TRUST_ANCHORS_INCLUDE_DIGICERTROOT
// #define TRUST_ANCHORS_INCLUDE_ENTRUST
// #define TRUST_ANCHORS_INCLUDE_GEOTRUSTPRIMARYCERTIFICATIONAUTHORITY_G3
// #define TRUST_ANCHORS_INCLUDE_GLOBALSIGNROOTR1
// #define TRUST_ANCHORS_INCLUDE_GODADDYROOTCERTIFICATEAUTHORITY_G2
// #define TRUST_ANCHORS_INCLUDE_ISRGROOTX1
// #define TRUST_ANCHORS_INCLUDE_MICROSOFTRSAROOTCERTIFICATEAUTHORITY2017
// #define TRUST_ANCHORS_INCLUDE_VERISIGN_CERTIFICATE


struct GSMRootCert
{
     const char* name;
     const unsigned char* data;
     const int size;
};

// #define GSM_NUM_ROOT_CERTS (sizeof(GSM_ROOT_CERTS) / sizeof(GSM_ROOT_CERTS[0]))
extern const GSMRootCert GSM_ROOT_CERTS[];
extern const unsigned int GSM_NUM_ROOT_CERTS;

extern const GSMRootCert GSM_CUSTOM_ROOT_CERTS[];
extern const unsigned int GSM_CUSTOM_NUM_ROOT_CERTS;

// Certificado: AmazonRootCA1
#ifdef TRUST_ANCHORS_INCLUDE_AMAZONROOTCA1
extern const unsigned char AmazonRootCA1_der[];
extern const unsigned int AmazonRootCA1_der_len;
#endif // TRUST_ANCHORS_INCLUDE_AMAZONROOTCA1

#ifdef TRUST_ANCHORS_INCLUDE_BALTIMORECYBERTRUST
// Certificado: BaltimoreCyberTrust
extern const unsigned char BaltimoreCyberTrust_der[];
extern const unsigned int BaltimoreCyberTrust_der_len;
#endif // TRUST_ANCHORS_INCLUDE_BALTIMORECYBERTRUST

#ifdef TRUST_ANCHORS_INCLUDE_COMODORSA_CERTIFICATION_AUTHORITY
// Certificado: COMODO RSA Certification Authority
extern const unsigned char COMODO_RSA_Certification_Authority_der[];
extern const unsigned int COMODO_RSA_Certification_Authority_der_len;
#endif // TRUST_ANCHORS_INCLUDE_COMODORSA_CERTIFICATION_AUTHORITY

#ifdef TRUST_ANCHORS_INCLUDE_DIGICERTGLOBALROOTCA
// Certificado: DigiCertGlobalRootCA
extern const unsigned char DigiCertGlobalRootCA_der[];
extern const unsigned int DigiCertGlobalRootCA_der_len;
#endif // TRUST_ANCHORS_INCLUDE_DIGICERTGLOBALROOTCA

#ifdef TRUST_ANCHORS_INCLUDE_DIGICERTGLOBALROOTG2
// Certificado: DigiCertGlobalRootG2
extern const unsigned char DigiCertGlobalRootG2_der[];
extern const unsigned int DigiCertGlobalRootG2_der_len;
#endif // TRUST_ANCHORS_INCLUDE_DIGICERTGLOBALROOTG2

#ifdef TRUST_ANCHORS_INCLUDE_DIGICERTROOT
// Certificado: Digicert_Root
extern const unsigned char Digicert_Root_der[];
extern const unsigned int Digicert_Root_der_len;
#endif // TRUST_ANCHORS_INCLUDE_DIGICERTROOT

#ifdef TRUST_ANCHORS_INCLUDE_ENTRUST
// Certificado: EnTrust
extern const unsigned char EnTrust_der[];
extern const unsigned int EnTrust_der_len;
#endif // TRUST_ANCHORS_INCLUDE_ENTRUST

#ifdef TRUST_ANCHORS_INCLUDE_GEOTRUSTPRIMARYCERTIFICATIONAUTHORITY_G3
// Certificado: GeoTrustPrimaryCertificationAuthority-G3
extern const unsigned char GeoTrustPrimaryCertificationAuthority_G3_der[];
extern const unsigned int GeoTrustPrimaryCertificationAuthority_G3_der_len;
#endif // TRUST_ANCHORS_INCLUDE_GEOTRUSTPRIMARYCERTIFICATIONAUTHORITY_G3

#ifdef TRUST_ANCHORS_INCLUDE_GLOBALSIGNROOTR1
// Certificado: GlobalSignRootR1
extern const unsigned char GlobalSignRootR1_der[];
extern const unsigned int GlobalSignRootR1_der_len;
#endif // TRUST_ANCHORS_INCLUDE_GLOBALSIGNROOTR1

#ifdef TRUST_ANCHORS_INCLUDE_GODADDYROOTCERTIFICATEAUTHORITY_G2
// Certificado: GoDaddyRootCertificateAuthority-G2
extern const unsigned char GoDaddyRootCertificateAuthority_G2_der[];
extern const unsigned int GoDaddyRootCertificateAuthority_G2_der_len;
#endif // TRUST_ANCHORS_INCLUDE_GODADDYROOTCERTIFICATEAUTHORITY_G2

#ifdef TRUST_ANCHORS_INCLUDE_ISRGROOTX1
// Certificado: ISRGRootX1
extern const unsigned char ISRGRootX1_der[];
extern const unsigned int ISRGRootX1_der_len;
#endif // TRUST_ANCHORS_INCLUDE_ISRGROOTX1

#ifdef TRUST_ANCHORS_INCLUDE_MICROSOFTRSAROOTCERTIFICATEAUTHORITY2017
// Certificado: MicrosoftRSARootCertificateAuthority2017
extern const unsigned char MicrosoftRSARootCertificateAuthority2017_der[];
extern const unsigned int MicrosoftRSARootCertificateAuthority2017_der_len;
#endif // TRUST_ANCHORS_INCLUDE_MICROSOFTRSAROOTCERTIFICATEAUTHORITY2017

#ifdef TRUST_ANCHORS_INCLUDE_VERISIGN_CERTIFICATE
// Certificado: VeriSign
extern const unsigned char VeriSign_der[];
extern const unsigned int VeriSign_der_len;
#endif // TRUST_ANCHORS_INCLUDE_VERISIGN_CERTIFICATE

#endif // TRUST_ANCHORS_H
