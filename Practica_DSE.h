/**
   Includes
*/
#include "Servo.h"
#include <limits.h>

/**
   Declaracion de los pines
*/
const char PIN_MOTOR_BRAZO = 0;
const char PIN_MOTOR_SPRAY = 1;
const char PIN_MOTOR_BASE = 8;
const char PIN_LED_LISTO = 3;
const char PIN_LED_EJECUTANDO = 4;
const char PIN_LED_TERMINADO = 5;
const char PIN_ZUMBADOR = 6;
const char PIN_LDR = 3;
const char PIN_BOTON = 2; // Debe ser 2 o 3 para que funcione la interrupcion

/**
   Variables de control del pintando
*/
const double TIEMPO_PINTADO = 10000.0; // Tiempo necesario para realizar el pintando
const short TIEMPO_FINALIZACION = 200; /* Delay utilizado al final del pintado para garantizar
                                            que los servos vuelven a su posicion por defecto */

/**
   Constantes para los servos
*/
const unsigned char ANGULO_POR_MS = 0;
const unsigned int ANGULO_BASE_ROTACION = 0; /* Angulo total que debe recorrer la base hasta
                                                que finaliza el pintado */
const char ANGULO_BASE_DEFECTO = 0;          // Angulo por defecto de la base
const char ANGULO_SPRAY_PINTAR = 0;          /* Angulo en el cual el servo del spray pulsa
                                                el boton del mismo */
const char ANGULO_SPRAY_NO_PINTAR = 0;       // Angulo en el que el servo no presiona el boton
const char ANGULO_BRAZO_ARRIBA = 0;          /* La posicion mas elevado del brazo que sostiene
                                                el spray */
const char ANGULO_BRAZO_ABAJO = 0;           /* La posicion mas baja del brazo que sostiene
                                                el spray */
const char ANGULO_BRAZO_DEFECTO = 0;         // Angulo por defecto del brazo
const char MOTOR_DELAY = 100;                /* Delay utilizado entre movimientos de los
                                                motores para asegurar que alcanzan sus posiciones */

/**
   Constantes para el zumbador
*/
const int ZUMBADOR_FRECUENCIA = 0; // Frecuencia del sonido emitido por el zumbador

/**
   Valores de control del LDR
*/
const short LDR_HAY_LUZ = 150;     // Valor umbral que indica si se percibe luz
const short LDR_NO_HAY_LUZ = 40;   // Valor umbral que indica si no se percibe luz
const short DELAY_BLOQUEADO = 500; /* Cuanto delay se realiza dentro de bloqueado_f(),
                                            con el objetivo de no leer constantemente el LDR */

// Enumeracion con los estados en los que se puede encontrar el dispositivo
enum estado_enum
{
  inicio,    /* o init. El dispositivo se esta iniciando. Despues de inicio se pasa a en_espera.
                Una vez terminada la finalizacion, no se deberia volver a este estado */
  listo,     // El dispositivo puede pintar. Se puede pasar a los estados en_espera o pintando
  en_espera, // El dispositivo no puede empezar a pintar. Se puede acceder al estado listo
  pintando,  // El dispositivo esta pintando. Puede pasar a los estados bloqueado y finalizado
  bloqueado, // El dispositivo ha sido interrumpido durante la ejecucion. Pasa al estado pintando
  terminado, // El dispositivo ha terminado. Posteriormente pasa a en_espera
  error,     // Error. No se deberia alcanzar este estado
  n_estados  // La cantidad de estados disponibles en la maquina
};

/**
   Declaracion de los metodos
*/
void inicio_f();    // Callback que ejecuta el estado inicio
void listo_f();     // Callback que ejecuta el estado listo
void en_espera_f(); // Callback que ejecuta el estado en_espera
void pintando_f();  // Callback que ejecuta el estado pintando
void bloqueado_f(); // Callback que ejecuta el estado bloqueado
void terminado_f(); // Callback que ejecuta el estado terminado
void error_f();     // Callback que ejecuta el estado error

void int_boton();
void mover_motor(Servo servo, char pin, int angulo);
void abortar();
void print_double(double val, unsigned int precision); // Fuente: http://forum.arduino.cc/index.php?topic=44216.0

/**
   Variables del sistema
*/
// Array que almacena los punteros a cada callback
void (*funciones_estados[n_estados])() = {inicio_f, listo_f, en_espera_f, pintando_f,
                                          bloqueado_f, terminado_f, error_f
                                         };
volatile int estado = inicio; // Almacena el estado actual de la maquina
char inicializado = 0;        // 0: El sistema no se ha inicializado. 1: Si lo ha hecho
/// Objetos para el control de los motores
Servo servo_base;
Servo servo_spray;
Servo servo_brazo;
unsigned int base_angulo_actual = 0;  // Angulo recorrido por el motor de la base en el pintado
unsigned int brazo_angulo_actual = 0; // Angulo recorrido por el motor del brazo en el pintado
char signo_brazo = 1;                 // 1 ó -1. Para mover el brazo de arriba a abajo
