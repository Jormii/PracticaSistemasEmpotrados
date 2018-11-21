/**
 * Los pines a los que se conectan los distintos componentes
 */
const char PIN_MOTOR_BRAZO = 0;
const char PIN_MOTOR_SPRAY = 1;
const char PIN_MOTOR_BASE = 2;
const char PIN_LED_LISTO = 3;
const char PIN_LED_EJECUTANDO = 4;
const char PIN_LED_TERMINADO = 5;
const char PIN_ZUMBADOR = 6;
const char PIN_LDR = 7;
const char PIN_BOTON = 8;

/**
 * Variables de control del pintando
 */
const double TIEMPO_PINTADO = 0.0;
const double TIEMPO_ITERACION = 0.0;
const double TIEMPO_FINALIZACION = 0.0;

/**
 * Los angulos que usan los motores
 */
const short MOTOR_BASE_ROTACION = 0;
const char MOTOR_SPRAY_PINTAR = 0;
const char MOTOR_SPRAY_NO_PINTAR = 0;
const char MOTOR_BRAZO_ARRIBA = 0;
const char MOTOR_BRAZO_ABAJO = 0;

/**
 * Valores para el zumbador
 */
const int ZUMBADOR_FRECUENCIA = 0;

/**
 * Valores de control del LDR
 */
const short LDR_HAY_LUZ = 0;
const short LDR_NO_HAY_LUZ = 1;

// Enumeracion con los estados en los que se puede encontrar el dispositivo
enum estado_enum
{
    inicio,    // El dispositivo se esta iniciando
    listo,     // El dispositivo puede pintar
    en_espera, // El dispositivo no puede empezar a pintar
    pintando,  // El dispositivo esta pintando
    bloqueado, // El dispositivo ha sido interrumpido durante la ejecucion
    terminado, // El dispositivo ha terminado
    error,     // Error
    n_estados
};

// Array que contiene las callbacks de los estados. inicio_f es equivalente a setup
void (*funciones_estados[n_estados])() = {inicio_f, listo_f, en_espera_f, pintando_f,
                                          bloqueado_f, terminado_f, error_f};

/**
 * Variables de control del sistema
 */
char estado = inicio;           // Almacena el estado actual de la maquina
int lectura_LDR = 0;            // Valor leido por el LDR
double tiempo_ejecucion = -1.0; // Tiempo que lleva pintando la maquina

// Funcion que ejecuta el estado inicio
void inicio_f()
{
    Serial.println("Error: El programa no deberia alcanzar el estado init");
    // Detener todo
}

// Funcion que ejecuta el estado listo
void listo_f()
{
    lectura_LDR = analogRead(PIN_LDR);
    if (lectura_LDR == LDR_HAY_LUZ)
    {
        digitalWrite(PIN_LED_LISTO, LOW);
        analogWrite(PIN_ZUMBADOR, ZUMBADOR_FRECUENCIA);
        estado = en_espera;
        return;
    }

    // Meter codigo para comprobar posibles errores electro-mecanicos del boton
    if (digitalRead(PIN_BOTON) == HIGH)
    {
        digitalWrite(PIN_LED_EJECUTANDO, HIGH);
        digitalWrite(PIN_LED_LISTO, LOW);
        estado = pintando;
        return;
    }
}

// Funcion que ejecuta el estado en_espera
void en_espera_f()
{
    lectura_LDR = analogRead(PIN_LDR);

    if (lectura_LDR == LDR_NO_HAY_LUZ)
    {
        digitalWrite(PIN_LED_LISTO, HIGH);
        analogWrite(PIN_ZUMBADOR, 0);
        estado = listo;
    }
}

// Funcion que ejecuta el estado pintando
void pintando_f()
{
    while (tiempo_ejecucion <= TIEMPO_PINTADO)
    {
        int lectura_LDR = analogRead(PIN_LDR);
        if (lectura_LDR == LDR_HAY_LUZ)
        {
            digitalWrite(PIN_LED_EJECUTANDO, LOW);
            // Parar motores
            analogWrite(PIN_ZUMBADOR, ZUMBADOR_FRECUENCIA);
            estado = bloqueado;
            bloqueado_f();
        }
        tiempo_ejecucion += TIEMPO_ITERACION;
    }
    digitalWrite(PIN_LED_TERMINADO, HIGH);
    digitalWrite(PIN_LED_EJECUTANDO, LOW);
    estado = terminado;
}

// Funcion que ejecuta el estado bloqueado
void bloqueado_f()
{
    while ((lectura_LDR = analogRead(PIN_LDR)) != LDR_NO_HAY_LUZ)
    {
        ;
    }
    digitalWrite(PIN_LED_EJECUTANDO, HIGH);
    analogWrite(PIN_ZUMBADOR, 0);
    estado = pintando;
}

// Funcion que ejecuta el estado terminado
void terminado_f()
{
    // Apagar motores y devolverlos a sus posiciones por defecto
    delay(TIEMPO_FINALIZACION); // Para asegurar que todo vuelve a su lugar
    estado = en_espera;
}

// Funcion que ejecuta el estado error
void error_f()
{
    Serial.println("Error: El sistema no deberia alcanzar el estado error");
    // Detener todo
}

void setup()
{
    pinMode(PIN_MOTOR_BRAZO, INPUT);
    pinMode(PIN_MOTOR_SPRAY, INPUT);
    pinMode(PIN_MOTOR_BASE, INPUT);
    pinMode(PIN_LED_LISTO, OUTPUT);
    pinMode(PIN_LED_EJECUTANDO, OUTPUT);
    pinMode(PIN_LED_TERMINADO, OUTPUT);
    pinMode(PIN_BOTON, INPUT);
    Serial.begin(9600);

    estado = en_espera;
}

void loop()
{
    (*funciones_estados[estado]);
}
