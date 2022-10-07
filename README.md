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
Cumpliendo con el paradigma de pogramacion modular. Se estructura el codigo del programa en los siguientes modulos:
- **main**: Punto de entrada del programa, interactua con los demas modulos.
- **utils**: contiene funciones de utilidad, como delays.
- **Communications**: Contiene el codigo para la comunicacion serial por SPI.
- **FlowMeter**: Contiene el codigo para la medicion de flujo masico y volumetrico de gas, medicion de presion y calibracion del USS.
- **Memory**: Controla y manipula la memoria FRAM, para respaldo de datos.
- **UI**: Maneja los distintos componentes de la interfaz con el usuario (leds, botones, etc.)
- **ValveControl**: Controla la valvula (apertura,cierre)

El codigo esta compuesto por las siguientes carpetas:	
- `hal`: hardware abstraction layer. Contiene varios archivos de codigo con funciones para manejar interactuar con perifericos y submodulos del MSP abstrayendose de la interaccion con el harware. Se implementa con funciones de librerias como `driverlib`.
- `libraries`: Contiene las librerias utilizadas en el codigo (USS, FRAM utilities, driverlib), asi como los archivos `.h`. **NO tocar**. Se puede romper el codigo
- `modules`: Contiene los archivos de codigo de los distintos modulos del programa.
- `USS_config`: Contiene archivos de configuracion del modulo USS. Estos archivos pueden generarse desde el Ultrasonic Design Center en base a la configuracion que presente los mejores resultados.

### Funciones de FlowMeter:
- `flowMeter_setup()`: Inicializa el medidor de flujo; se encarga de iniciar la USSlib, iniciar los algoritmos y cargar los capacitores para arrancar a medir en regimen permanente. Debe llamarse al principio del programa
- `flowMeter_getVolumeFloatRate()`: Devuelve el flujo volumetrico de gas. Para esto envia un pulso, espera en LPM3 a recibirlo y lo procesa mediante el algoritmo de HillbertWide para hallar el flujo.
- `flowMeter_getMassFlowRate()`: Devuelve el flujo masico de gas (**WIP**)
- `flowMeter_getPressure()`: Devuelve la presion del gas (**WIP**)
