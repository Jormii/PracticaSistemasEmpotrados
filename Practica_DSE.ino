#include "Practica_DSE.h"

void inicio_f()
{
    if (inicializado)
    {
        Serial.println("Error: El programa no deberia alcanzar el estado inicio una vez inicializado");
        abortar();
    }

    // Inicializar la posicion de los pines
    pinMode(PIN_MOTOR_BRAZO, INPUT);
    pinMode(PIN_MOTOR_SPRAY, INPUT);
    pinMode(PIN_MOTOR_BASE, INPUT);
    pinMode(PIN_BOTON, INPUT);
    pinMode(PIN_LED_LISTO, OUTPUT);
    pinMode(PIN_LED_EJECUTANDO, OUTPUT);
    pinMode(PIN_LED_TERMINADO, OUTPUT);

    // Mover los motores a las posiciones por defecto
    mover_motor(servo_base, PIN_MOTOR_BASE, ANGULO_BASE_DEFECTO);
    mover_motor(servo_brazo, PIN_MOTOR_BRAZO, ANGULO_BRAZO_DEFECTO);
    mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_NO_PINTAR);

    // Inicializar el bus serial
    Serial.begin(9600);
    while (!Serial)
    {
        ;
    }
    Serial.println("Setup finalizado");

    estado = en_espera;
    inicializado = 1;
}

void listo_f()
{
    lectura_LDR = analogRead(PIN_LDR);

    // Si el LDR recibe luz, pasar al estado en_espera
    if (lectura_LDR >= LDR_HAY_LUZ)
    {
        digitalWrite(PIN_LED_LISTO, LOW);
        analogWrite(PIN_ZUMBADOR, ZUMBADOR_FRECUENCIA);
        estado = en_espera;
        return;
    }

    /// Meter codigo para comprobar posibles errores electro-mecanicos del boton
    // Si se pulsa el boton, pasar al estado pintando
    if (digitalRead(PIN_BOTON) == HIGH)
    {
        digitalWrite(PIN_LED_EJECUTANDO, HIGH);
        digitalWrite(PIN_LED_LISTO, LOW);
        estado = pintando;
    }
}

void en_espera_f()
{
    lectura_LDR = analogRead(PIN_LDR);

    // Si el LDR no detecta luz, pasar al estado listo
    if (lectura_LDR <= LDR_NO_HAY_LUZ)
    {
        digitalWrite(PIN_LED_LISTO, HIGH);
        analogWrite(PIN_ZUMBADOR, 0);
        estado = listo;
    }
}

// WIP
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

void bloqueado_f()
{
    // No proseguir hasta que el LDR no perciba oscuridad
    while ((lectura_LDR = analogRead(PIN_LDR)) >= LDR_HAY_LUZ)
    {
        delay(DELAY_BLOQUEADO);
    }
    digitalWrite(PIN_LED_EJECUTANDO, HIGH);
    analogWrite(PIN_ZUMBADOR, 0);
    // ¿Como retomar el estado de los motores?
    estado = pintando;
}

void terminado_f()
{
    mover_motor(servo_base, PIN_MOTOR_BASE, ANGULO_BASE_DEFECTO);
    mover_motor(servo_brazo, PIN_MOTOR_BRAZO, ANGULO_BRAZO_DEFECTO);
    mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_NO_PINTAR);
    delay(TIEMPO_FINALIZACION); // Para asegurar que todo vuelve a su lugar
    estado = en_espera;
}

void error_f()
{
    Serial.println("Error: El sistema no deberia alcanzar el estado error");
    abortar();
}

void mover_motor(Servo servo, char pin, int angulo)
{
    servo.attach(pin);
    servo.write(angulo);
    delay(MOTOR_DELAY);
    servo.detach();
}

void abortar()
{
    digitalWrite(PIN_LED_EJECUTANDO, HIGH);
    digitalWrite(PIN_LED_LISTO, HIGH);
    digitalWrite(PIN_LED_TERMINADO, HIGH);
    mover_motor(servo_base, PIN_MOTOR_BASE, ANGULO_BASE_DEFECTO);
    mover_motor(servo_brazo, PIN_MOTOR_BRAZO, ANGULO_BRAZO_DEFECTO);
    mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_PINTAR);
    delay(UINT_MAX);  // No existe una instruccion halt o stop
}

void setup()
{
    inicio_f();
}

void loop()
{
    // Llamar a la callback correspondiente al estado actual
    (*funciones_estados[estado])();
}
