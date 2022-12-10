# Proyecto Final: Modulo para control de GLP 🔥💥

## 📚 Librerias:

El codigo precisa las librerias `QMathlib`, `IQmathlib`, `MSP-driverlib`, `USS-sw-lib` y `FRAM utilities`. Algunas de estas vienen en forma de archivos c crudos (`.c` y .`.h`), otras vienen en archivos precomilados `.lib`.

### Modelos de Memoria

El compilador de TI define como administrar la memoria a partir de ciertos "modelos" llamados "memory models". Ciertos modelos permiten alocar más memoria de programa y/o usar ciertas librerías. El modelo a utilizar en general depende de la arquitectura del micro.
Los memory models se subdividen en 2 tipos:
  - **Code model**: define como se maneja la memoria de programa. Hay 2 modelos:
    - *small*: Intenta compilar todo el codigo en los 64KB de FRAM
    - *large*: Hace algo raro, destina 1 MB de memoria a codigo
  - Data model: define como se maneja la memoria de . Hay 3 modelos:
    - *small*:
    - *restricted*:
    - *large*: 

Los MSP430 tienen 2 arquitecturas: `MSP430` y `MSP430x`. Para saber la arquitectura hay que ver el datasheet. **La arquitectura del MSP430FR6043 es `MSP430x`** esto permite usar `code model = large`, que es necesario para la libreria USS.

### Archivos de librerias:
Algunas librerias como `driverlib` y `FRAM-utilities` vienen en forma de archivos de C crudos y se instalan copiando y pegando el codigo en la carpeta del proyecto. Otras librerias vienen precompiladas en formato `.lib`. Estas librerias contienen varios archivos de este formato. Uno por cada tipo de modelo de memoria.
En general estas tambien incluyen un archivo `.a`, que actua como "indice" o "dicionario", indicandole al linker cual de los `.lib` usar para el MSP seleccionado.
Por lo tanto, hay 3 formas de indicarle una libreria al linker:
 1) Le especifico el archivo `.lib` especifico para la arquitectura del MSP que tengo.
 2) Le especifoc un archivo `.a` que indica al linker cual de los `lib` usar.
 3) Le indico una carpeta donde buscar librerias.

### Configurar CCStudio para incluir librerias:
En el compilador de TI, para indiciarle una libreria al linker se usa el flag `--library`, aunque nadie usa comandos para compilar. Desde el IDE, hago **click derecho sobre el proyecto -> Properties -> MSP430 Linker -> File search path** y agrego un path (relativo al workspace) al archivo `.lib`, `.a`, etc.

### Librerias vs Includes:
Las librerias y los includes **NO** son lo mismo, las librerias vienen precompiladas (`.lib`), y tambien vienen con un archivo `.h` que debo incluir en el codigo para usar las librerias.
Los includes los resuelve el compilador (especificamente el preprocesador), las librerias las resuelve el linker. Por esto, se le pueden indicar ciertas "rutas" al linker para buscar includes, de forma que no sea necesario aclararle la ruta al `.h`. Esto se hace haciendo **click derecho sobre el proyecto -> Properties -> MSP430 Compiler -> include path** y agregando un path.
Para incluir un archivo al codigo uso `#include "archivo .h"` o `#include <archivo.h>`. La primer opcion va a buscar el archivo en la misma carpeta donde esta el codigo en ejecucion, la segunda opcion busca primero en la lista de includes especificada en el compiler y despues en la carpeta.
Si no hago ninguna de estas 2 la opcion que tengo es escribir toda la ruta en el `#include`

### Uso de la libreria USSlib:
La libreria esta estructurada en varios modulos. Para ver mas sobre esta, leer el archivo "MSP apuntes sobre funcionamiento" en drive.

## 🌳 Estructura del codigo:
El codigo esta compuesto por las siguientes carpetas:	
- `/hal`: hardware abstraction layer. Contiene varios archivos de codigo con funciones para manejar interactuar con perifericos y submodulos del MSP abstrayendose de la interaccion con el harware. Se implementa con funciones de librerias como `driverlib`. Por ahora, el HAL esta programado para el hardware de la EVM430-FR6043, se debera adaptar para nuestro hardware propio. **NOTA:** El HAL del LCD es **solo** para testeo en la EVM.
- `/libraries`: Contiene las librerias utilizadas en el codigo (USS, FRAM utilities, driverlib), asi como los archivos `.h`. **NO tocar**. Se puede romper el codigo
- `/modules`: Contiene los archivos de codigo de los distintos modulos del programa.
- `/USS_config`: Contiene archivos de configuracion del modulo USS. Estos archivos pueden generarse desde el Ultrasonic Design Center en base a la configuracion que presente los mejores resultados.

## 📦 Modulos:
Cumpliendo con el paradigma de pogramacion modular. Se estructura el codigo del programa en los siguientes modulos:
- **main**: Punto de entrada del programa, interactua con los demas modulos.
- **utils**: contiene funciones de utilidad, como delays.
- **Communications**: Contiene el codigo para la comunicacion serial por SPI.
- **FlowMeter**: Contiene el codigo para la medicion de flujo masico y volumetrico de gas, medicion de presion y calibracion del USS.
- **Memory**: Controla y manipula la memoria FRAM, para respaldo de datos.
- **UI**: Maneja los distintos componentes de la interfaz con el usuario (leds, botones, etc.)
- **ValveControl**: Controla la valvula (apertura,cierre)
Cada modulo tiene sus funciones especificas, asi como un `struct` unico con los atributos de cada modulo (por ejemplo, el modulo de comunicaciones tiene un `struct` con las propiedades de la comunicacion por SPI.

### FlowMeter:
Modulo de medicion de flujo de gas.
**Funciones:**
- `flowMeter_setup()`: Inicializa el medidor de flujo; se encarga de iniciar la USSlib, iniciar los algoritmos y cargar los capacitores para arrancar a medir en regimen permanente. Debe llamarse al principio del programa. Devuelve un mensaje `USS_message_code` indicando si hubo algun error.
- `flowMeter_getVolumeFlowRate()`: Calcula el flujo volumetrico de gas. Para esto envia un pulso, espera en LPM3 a recibirlo y lo procesa mediante el algoritmo de HillbertWide para hallar el flujo. Devuelve el flujo en formato punto fijo, representado por una variable del tipo `_iq16` de la libreria `IQmathlib`.
- `flowMeter_getMassFlowRate(_iq16 vol_flow_rate)`: Calcula el flujo masico de gas a partir de la formula explicitada en la UNIT 1072, en formato punto fijo representado por una variable del tipo `_iq16`. (**WIP**)
- `flowMeter_getPressure()`: Devuelve la presion del gas medida por el sensor, en formato punto fijo representado por una variable del tipo `_iq16`. (**WIP**)
- `flowMeter_getTemperature()`: Devuelve la temperatura del gas medida por el sensor, en formato punto fijo representado por una variable del tipo `_iq16`. (**WIP**)
- `flowMeter_getDensity()`: Devuelve la densidad del gas en funcion de la temperatura y presion, en formato punto fijo representado por una variable del tipo `_iq16`. (**WIP**)
- `flowMeter_measure()`: Funcion principal del modulo, llama a las demas: mide el flujo volumetrico, presion, temp., etc., calcula el flujo masico y actualiza el totalizador. No devuelve nada (`void`) .

### Communications:
Modulo de comunicacion por SPI.
**Estructuras:**
- `struct SPI_Communications_Module`: modulo de comunicaciones, tiene los siguientes atributos:
	- `communication_Status`: Estado de la comunicaciones (ACTIVE, LISTENING, PROCESSING FRAME, SENDING RESPONSE)
	- `byte_Read_Counter`: Cuenta cuantos bytes han sido recibidos del paquete actual.
	- `byte_Transmit_Counter`: Cuenta cuantos bytes han sido transmitidos del paquete actual.
	- `byte_Read_buffer`: buffer que contiene los bytes del frame que se esta recibiendo.
	- `byte_Transmit_buffer`: buffer que contiene los bytes del frame que se esta transmitiendo.
	- `received_Frame_Length`: Largo del frame que se esta recibiendo.
	- `transmit_Frame_Length`: largo del frame que se esta transmitiendo.
	- `byte_Rx_received`: flag que indica que se recibio un byte.
	- `byte_Tx_ready`: flag que indica que se esta listo para transmitir un nuevo byte por SPI.
	
- `struct SPI_Communications_Frame`: paquete del protocolo de comunicacion con los siguientes atributos:
	- `frame_Type`: El tipo de frame, definido como un enum `SPI_Communications_Frame_Type`.
	- `frame_Length`: El largo de frame/
	- `frame_CRC`: El byte de CRC del frame.
	- `*frame_Body`: Array que representa los bytes del payload del paquete.
- `enum SPI_Communications_Frame_Type`: tipo del paquete, hay varios tipos:
	- `FRAME_REQUEST_TOTALIZER = 0x01`: Pedido de totalizador.
    - `FRAME_REQUEST_STATUS = 0x03`: Pedido de status.
    - `FRAME_REQUEST_MEASUREMENT = 0x05`: Pedido de realizar una medida.
    - `FRAME_REQUEST_AVERAGE_MASS_FLOW_RATE = 0x07`: Pedido de flujo promedio (no usado).
    - `FRAME_REQUEST_OPEN_VALVE = 0xA0`: Pedido de abrir la valvula.
    - `FRAME_REQUEST_CLOSE_VALVE = 0xAF`: Pedido de cerrar la valvula.
    - `FRAME_REQUEST_CONFIGURE = 0x10`: Pedido de configurar el medidor.
    - `FRAME_RESPONSE_TOTALIZER = 0x02`: Respuesta del medidor, devuelve el totalizador.
    - `FRAME_RESPONSE_STATUS = 0x04`: Respuesta del medidor, devuelve el status.
    - `FRAME_RESPONSE_AVERAGE_MASS_FLOW_RATE = 0x08`: Respuesta del medidor, devuelve el flujo promedio (no usado).
    - `FRAME_RESPONSE_VALVE_ACK = 0xA5`: Respuesta del medidor, devuelve una verificacion de que la orden de la valvula se ejecuto.

**Funciones:**
- `Communications_setup()`: Inicializa el modulo de comunicacion por SPI, no devuelve nada (`void`).
- `Communications_send(SPI_Communications_Frame frame)`: Envia el paquete recibido por parametro. Al llamar esta funcion se inicia la transferencia del paquete por SPI la cual continua en paralelo con el resto del codigo, siempre y cuando se llame a `Communications_Update()` desde el loop principal.
- `Communications_isActive()`: Devuelve `true` si la comunicacion esta activa, `false` en caso contrario.
- `Communications_ProcessFrame(SPI_Communications_Frame request)`: Procesa el paquete recibido por parametro y genera la respuesta correspondiente. Por ej: si llega un request del flujo promedio, genera un paquete respuesta con el flujo promedio y lo devuelve.
- `Communications_update()`: Funcion principal del modulo, debe llamarse periodicamente desde el loop principal. Chequea si llego algun paquete, y actualiza la transmision del paquete actual en caso de haber uno. No devuelve nada.

### ValveControl:
Modulo de control de las valvulas.
**Funciones:**
- `valveControl_update()`: Abre la valvula, esto no es inmediato,, pone un pin de control en 1 durante un tiempo hasta que la valvula se cierra completamente. Devuelve `true` si la valvula comenzo a abrirse, `false` si no puede abrirse.
- `valveControl_close()`: Cierra la valvula, esto no es inmediato, pone un pin de control en 1 durante un tiempo hasta que la valvula se cierra completamente. Devuelve `true` si la valvula comenzo a cerrarse, `false` si la valvula no puede cerrarse.
- `valveControl_delay()`: Genera un delay  de 4 segundos utilizado para apagar el/los pines de control cuando se termina de abrir/cerrar la valvula. Utiliza el timer A4. Solo puede haber uno de estos delays a la vez.
- `valveControl_update()`: Funcion principal del modulo, debe llamarse periodicamente desde el loop principal, actualiza el estado de las valvulas.
