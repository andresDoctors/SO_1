# MyBash Grupo N°24
## Integrantes
 - Ariel Agustin Alvaro `ariel.alvaro@mi.unc.edu.ar`
 - Andres David Doctors `andresdaviddoctors@mi.unc.edu.ar`
 - Nicolas Alejandro Greiff `alejandro.greiff@mi.unc.edu.ar`
## Introduccion
Al implementar mybash tuvimos como objetivo:  
- complir con las exigencias del enunciado  
- generalizar el comando pipeline “|” a una cantidad arbitraria de comandos simples (punto estrella)  
- modificar lo menos posible el esqueleto que nos entregaron  
- manipular tos tipos pipeline, scommand y GSList* solo mediante sus funciones correspondientes, con el fin de mantener el código simple  
- imitar en la medida de lo posible el funcionamiento del shell de linux

## Modularizacion 
Seguimos la modularización propuesta por la cátedra
## Tecnicas de programacion
> Librerias Destacables
>> - glib.h <- **listas enlazadas GSList**
>>- string.h<-**definicion de strerror y funciones usadas para facilitar el uso de strings**
>> - strextra.h <-**definición de strmerge**
>>- unistd.h <- **llamados al sistema**
>>- errno.h <- **nos permite acceder el número de error de la última llama al sistema**
>>- fcntl.h <-**descriptores de archivo**
>>

El estilo de código que proporcionamos consta de nombre de variables simples y concretos según el caso, las funciones están escritas en minusculas para distinguirlas de macros y constantes si es que hubiese y separadas por guiones bajo en el caso que lo necesite, líneas con longitudes cortas que permite leer con facilidad el código y comentarios no triviales que ayudan a la compresión del mismo. Para la memoria dinámica usamos la variable Calloc que nos permite tener un buen manejo del tamaño de la memoria. El código es robusto, por lo tanto al mínimo error obtenemos una respuesta clara sobre el error

## Herramientas de programación
Usamos editores de texto para el desarrollo del código (Visual Studio, Notepad++,etc), para la compilación de estos la cátedra nos proporciona MakeFiles para compilar atraves de gcc (GNU Compiler Collection). Para el debugging utilizamos impresiones por pantalla de las variables y GDB(GNU debugger)

## Desarrollo 
- builtin:  
en caso de ejecutarse un exit no se ejecuta ninguna instrucción, désde mybash.c se libera la memoria reservada y se para el programa  
- execute:  
agregamos la función interna execute_scommand, que toma una pipeline y ejecuta el comando en el front, previamente haciendo las redirecciones marcadas con ‘<’ y ‘>’.  
agregamos la función _execute_pipeline, que toma como parámetros una pipeline y un parser y se encarga de forkear, hacer las conexiones pipe necesarias entre los comandos y llamar a execute_scommand. En caso de que falle alguna llamada al sistema: imprime un mensaje de error y toma las medidas correspondientes (liberar memoria, cancelar la ejecución del comando/pipeline y/o cerrar el programa) considerando cuál llamada falló y si fue llamada por el padre o uno de sus hijos.  
execute_pipeline se comporta igual que _execute_pipeline con la excepción de que bajo ninguna circunstancia puede liberar memoria reservada para un tipo Parser reservado por el llamador, la implementamos para testear nuestro execute sin necesidad de editar los archivos test_execute.c y test_execute.h.
- mybash:  
La unica edición fue cambiar el llamado _execute_pipeline(apipe, parser) en vez de a execute_pipeline(apipe). Con el fin de que en caso de que falle alguna llamada al sistema y se decida cerrar el programa _execute_pipeline pueda liberar la memoria reservada para parser.



