
class SerialUtils {
public:
    static void cleanSerialRxBuffer(HardwareSerial &serial) {
        while (serial.available()) {
            serial.read();
        }
    }
};
