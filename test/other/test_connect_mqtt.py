import os
import ssl
import time

import paho.mqtt.client as mqtt
from dotenv import load_dotenv

# ============================================================
# CARGA DE VARIABLES DE ENTORNO
# ============================================================
load_dotenv()

ENDPOINT = os.getenv("AWS_ENDPOINT")
PORT = int(os.getenv("AWS_PORT", 8883))
CLIENT_ID = os.getenv("AWS_CLIENT_ID", "python_mqtt_client")

TOPIC_PUB = os.getenv("TOPIC_PUB", "acousea/test/out")
TOPIC_SUB = os.getenv("TOPIC_SUB", "acousea/test/in")

PATH_ROOT_CA = os.getenv("PATH_ROOT_CA", "./AmazonRootCA1.pem")
PATH_CERT = os.getenv("PATH_CERT", "./certificate.pem.crt")
PATH_PRIVATE_KEY = os.getenv("PATH_PRIVATE_KEY", "./private.pem.key")

__flag_connected = False


# ============================================================
# CALLBACKS
# ============================================================
def on_connect(client, userdata, flags, rc):
    global __flag_connected
    if rc == 0:
        print("✅ Conectado correctamente a AWS IoT Core")
        client.subscribe(TOPIC_SUB)
        print(f"📡 Suscrito a: {TOPIC_SUB}")
        __flag_connected = True
    else:
        print(f"❌ Error de conexión: {rc}")
        __flag_connected = False


def on_message(client, userdata, msg):
    print(f"📥 Mensaje recibido en {msg.topic}: {msg.payload.decode()}")


def on_disconnect(client, userdata, rc):
    global __flag_connected
    print("⚠️ Desconectado del broker (RC =", rc, ")")
    __flag_connected = False


def main():
    # ============================================================
    # CLIENTE MQTT
    # ============================================================
    client = mqtt.Client(client_id=CLIENT_ID)
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_disconnect = on_disconnect

    # Carga de certificados y configuración TLS
    client.tls_set(
        ca_certs=PATH_ROOT_CA,
        certfile=PATH_CERT,
        keyfile=PATH_PRIVATE_KEY,
        tls_version=ssl.PROTOCOL_TLSv1_2,
        ciphers=None
    )

    # Opcional: desactivar verificación del hostname (no recomendado)
    client.tls_insecure_set(False)

    # ============================================================
    # CONEXIÓN Y LOOP
    # ============================================================
    print(f"🔗 Conectando a {ENDPOINT}:{PORT} ...")
    client.connect(ENDPOINT, PORT, keepalive=60)

    # Loop en segundo plano
    client.loop_start()

    while not __flag_connected:
        print("⏳ Esperando conexión...")
        time.sleep(1)

    try:
        while True:
            payload = '{"msg": "Hello from Python!"}'
            print(f"📤 Publicando: {payload}")
            client.publish(TOPIC_PUB, payload, qos=1)
            time.sleep(5)

    except KeyboardInterrupt:
        print("\n🛑 Cerrando conexión...")
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    main()
