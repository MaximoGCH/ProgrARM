
# ProgrArm

Vídeo del resultado:

[![VIDEO](https://img.youtube.com/vi/-UbCkGc3Uh8/0.jpg)](https://youtu.be/-UbCkGc3Uh8)


Prograrm es un brazo robótico creado por **Máximo García-Cesto** y **Pablo Menchén**.
La idea del proyecto era construir un brazo robótico fácil de usar por cualquier persona.
Por ello, el proyecto se divide en dos partes:
- ##### La creación del brázo robótico.
- ##### La creación de una aplicación web SPA que permita a cualquier usuario mandar instrucciones al brazo.
Para la conexión del brazo a la red y para poder almacenar en el brazo distintos programas, se usó una pantalla táctil y un lector de tarjetas SD.

## ¿Como funciona progrArm?
![Cómo funciona 1](/readmeFiles/Diapositiva2.PNG)
![Cómo funciona 2](/readmeFiles/Diapositiva3.PNG)
![Cómo funciona 3](/readmeFiles/Diapositiva4.PNG)
![Cómo funciona 4](/readmeFiles/Diapositiva5.PNG)


------------

### El brázo robótico
El brazo consta de 6 servos y 1 stepper, que controlan las distintas articulaciones y permiten a la base moverse 360 grados.
Para controlar todos estos servos, se usó un AdafruitMotoshield y un Arduino Mega.
La estructura del brazo se construyó de plástico usando una impresora 3D.
El modelo del brazo se encontró en la siguientes páginas: 
- https://www.thingiverse.com/thing:1748596
- https://www.thingiverse.com/thing:1750025
- https://www.thingiverse.com/thing:1838120

------------

### El servidor web
La idea era crear una aplicación web que permitiera a cualquier persona construir programas para el brazo robótico. Basándonos en la aplicación Scratch, hicimos una aplicación SPA con manipulación directa que permite a los usuarios arrastrar bloques para construir secuencias de instrucciones.
Para la creación del servidor, se usó un nodeMCU, una placa de desarollo basada en el ESP8266, que permite conectarse a la red.
Dentro de esta placa se metió el servidor, escrito en C.
El cliente está hecho con HTML, CSS y JavaScript, usando tecnología AJAX para enviar información al servidor.

------------

### La conexión entre el servidor web y el brazo
Entre el servidor y el brazo se colocó una arduino con una pantalla táctil y un lector de tarjetas SD, de manera que se le permitiera al usuario almacenar los distintos programas creados con el servidor web, y ejecutarlos cuando quisiera para que el brazo realizara las acciones.
La pantalla también permite que el usuario se pueda conectar a la red escribiendo en un teclado táctil la contraseña de la red y el nombre de esta.
Para conectar el arduino de la pantalla con el nodeMCU (del cliente) y el arduino del servidor, se usaron los puertos de conexión serial que presentan los arduinos.
![enter image description here](/readmeFiles/Diapositiva8.PNG)
## Contacto
**Máximo García-Cesto Huret** 

 maximo.garcia.cesto@gmail.com
 
Encargado de:
 - La aplicación SPA y la conexión con el servidor web.
 - La pantalla táctil y el lector de tarjetas SD.


**Pablo Menchén**

Encargado de 
 - La construcción del brazo.

