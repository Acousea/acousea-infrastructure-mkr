#ifndef CREDENTIALS_HPP
#define CREDENTIALS_HPP

#error "You are including credentials.example.hpp directly. Copy it to credentials.hpp and edit."


// Mock credentials
#define SECRET_PINNUMBER ""
#define SECRET_GPRS_APN "dummy.apn"
#define SECRET_GPRS_LOGIN "user"
#define SECRET_GPRS_PASSWORD "pass"

#define AWS_MQTT_BROKER "mock-broker.amazonaws.com"
#define AWS_MQTT_CLIENT_ID "MockClientId"

#define CLIENT_CERTIFICATE R"(-----BEGIN CERTIFICATE-----
MOCK
-----END CERTIFICATE-----)"

#endif
