#ifndef CREDENTIALS_EXAMPLE__HPP
#define CREDENTIALS_EXAMPLE__HPP

#error "You are including credentials.example.hpp directly. Copy it to credentials.hpp and edit."


// Mock credentials
const auto SECRET_PINNUMBER = "";
const auto SECRET_GPRS_APN = "dummy.apn";
const auto SECRET_GPRS_LOGIN = "user";
const auto SECRET_GPRS_PASSWORD = "pass";

const auto AWS_MQTT_BROKER = "mock-broker.amazonaws.com";
const auto AWS_MQTT_CLIENT_ID = "MockClientId";

const auto CLIENT_CERTIFICATE = R"(-----BEGIN CERTIFICATE-----
MOCK
-----END CERTIFICATE-----)";


#endif
