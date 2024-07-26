
# Proyecto de Infraestructura de Comunicaciones para Acousea

Este proyecto implementa la infraestructura de comunicaciones para el derivador oceánico utilizando dispositivos Arduino MKR 1310. El sistema está diseñado para proporcionar una solución robusta y eficiente para el monitoreo acústico pasivo en áreas remotas.

## Requisitos

Antes de comenzar, asegúrate de tener instalados los siguientes requisitos:

- [PlatformIO Core](https://platformio.org/install/cli)
- [VSCode](https://code.visualstudio.com/) con la extensión de PlatformIO
- Placas Arduino MKR 1310
- Componentes adicionales (GPS UBlox GNSS, SD shield, modem Iridium RockBLOCK 9603, etc.)


## Instalación

1. Clona el repositorio a tu máquina local:
```bash
git clone [URL_DEL_REPOSITORIO]
cd [DIRECTORIO_DEL_REPOSITORIO]
```

2. Abre el proyecto en tu IDE preferido (recomendado Visual Studio Code con la extensión de PlatformIO).

## Estructura del Proyecto

El proyecto está organizado de la siguiente manera:

- **include/**: Archivos de configuración y bibliotecas.
  - **config.h**: Configuración general del proyecto.
  - **libraries.h**: Inclusión de bibliotecas necesarias.
- **lib/**: Bibliotecas y módulos específicos del proyecto.
  - **Battery/**: Gestión de la batería.
  - **Display/**: Gestión del display.
  - **GPS/**: Gestión del GPS.
  - **OperationManager/**: Gestión de operaciones.
  - **OperationModes/**: Modos de operación del sistema.
  - **Packet/**: Gestión y definición de paquetes de datos.
  - **Port/**: Gestión de puertos de comunicación.
  - **Processor/**: Procesamiento de paquetes.
  - **RTC/**: Controlador del reloj en tiempo real.
  - **Router/**: Enrutamiento de paquetes.
  - **Routines/**: Rutinas de operación.
  - **RoutingTable/**: Tabla de enrutamiento.
  - **SDManager/**: Gestión de la tarjeta SD.
  - **Services/**: Servicios adicionales.
- **platformio.ini**: Archivo de configuración de PlatformIO.
- **src/**: Código fuente principal del proyecto.
  - **dependencies.h**: Inclusión de dependencias.
  - **main.cpp**: Archivo principal del proyecto.
- **test/**: Pruebas del proyecto.
  - **tests.ipynb**: Pruebas en Jupyter Notebook.

## Configuración del Proyecto

### platformio.ini

El archivo `platformio.ini` contiene la configuración necesaria para compilar y subir el código a los dispositivos Arduino MKR 1310. Asegúrate de tener configurados correctamente los entornos y las bibliotecas necesarias.

```ini
[env:mkr1310]
platform = atmelsam
board = mkrwan1310
framework = arduino
lib_deps =
    ...dependencias
```

## Funcionalidades Principales

### Sistema de Control y Comunicaciones (SCC)

La primera etapa del desarrollo del SCC se centra en la selección del hardware y la implementación del software necesarios para el funcionamiento efectivo del derivador oceánico. Esta etapa establece la base para las capacidades de monitoreo y comunicación del sistema.

#### Infraestructura de Comunicaciones

La infraestructura de comunicaciones se compone de dos nodos principales:
- **Nodo Localizer**
- **Nodo Drifter**

La transmisión de paquetes se realiza mediante dos tecnologías:
- **LoRa**
- **Iridium**

#### Enrutamiento de Paquetes

El sistema utiliza un enrutador que gestiona la transmisión y procesamiento de paquetes. El diseño del software se basa en un `Router` y un `Processor`, que trabajan juntos para manejar la comunicación y el procesamiento de datos.

#### Modos de Operación

Los nodos tienen tres modos de operación principales:
- **Launching**: Fase inicial de despliegue.
- **Working**: Operación normal.
- **Recovering**: Fase de recuperación.


## Despliegue

Para desplegar el código en los dispositivos Arduino MKR 1310, sigue estos pasos:

1. Conecta tu Arduino MKR 1310 a tu PC.
2. Compila y sube el código utilizando PlatformIO:
```bash
    platformio run --target upload
```


## Licencia

Este proyecto está licenciado bajo la Licencia MIT. Consulta el archivo `LICENSE` para más detalles.

## Contacto

Para cualquier duda o consulta, por favor contacta a [antonio.aparicio101@alu.ulpgc.es](antonio.aparicio101@alu.ulpgc.es).
