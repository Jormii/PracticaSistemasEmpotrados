#include "Practica_DSE.h"

void inicio_f()
{
    // Si ya se habia inicializado el sistema, abortar
    if (inicializado)
    {
        Serial.println("Error: El programa no deberia alcanzar el estado inicio una vez inicializado");
        abortar();
    }

    // Inicializar los pines
    pinMode(PIN_MOTOR_BRAZO, INPUT);
    pinMode(PIN_MOTOR_SPRAY, INPUT);
    pinMode(PIN_MOTOR_BASE, INPUT);
    pinMode(PIN_BOTON, INPUT);
    pinMode(PIN_LED_LISTO, OUTPUT);
    pinMode(PIN_LED_EJECUTANDO, OUTPUT);
    pinMode(PIN_LED_TERMINADO, OUTPUT);

    // Mover los motores a sus posiciones por defecto
    mover_motor(servo_base, PIN_MOTOR_BASE, ANGULO_BASE_DEFECTO);
    mover_motor(servo_brazo, PIN_MOTOR_BRAZO, ANGULO_BRAZO_DEFECTO);
    mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_NO_PINTAR);

    // Inicializar el bus serie
    Serial.begin(9600);
    while (!Serial)
    {
        ;
    }
    Serial.println("Setup finalizado");

    // Se actualizan las variables de control
    estado = en_espera;
    inicializado = 1;
}

void listo_f()
{
    // Si el LDR recibe luz, pasar al estado en_espera
    if (analogRead(PIN_LDR) >= LDR_HAY_LUZ)
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
    // Si el LDR no detecta luz, pasar al estado listo
    if (analogRead(PIN_LDR) <= LDR_NO_HAY_LUZ)
    {
        digitalWrite(PIN_LED_LISTO, HIGH);
        analogWrite(PIN_ZUMBADOR, 0);
        estado = listo;
    }
}

void pintando_f()
{
    unsigned long t_ejecucion = 0.0;    // Tiempo que lleva pintando el programa
    unsigned long t0 = 0;               /* t0 y tF se usan para medir el tiempo transcurrido entre
                                        iteraciones del bucle. Almacenan microsegundos */
    unsigned long tF = 0.0;
    unsigned long elapsed_ms = 0.0;     // Diferencia entre t0 y tF en milisegundos
    int clamp = 0;                      /* Variable auxiliar que se utiliza para ajustar el
                                        angulo del brazo */

    // Se inicializan las variables de control para los angulos
    base_angulo_actual = ANGULO_BASE_DEFECTO;
    brazo_angulo_actual = ANGULO_BRAZO_DEFECTO;
    mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_PINTAR);

    // Bucle de pintado
    while (t_ejecucion <= TIEMPO_PINTADO)
    {
        // Si se percibe luz, pasar al estado bloqueado
        if (analogRead(PIN_LDR) >= LDR_HAY_LUZ)
        {
            digitalWrite(PIN_LED_EJECUTANDO, LOW);
            analogWrite(PIN_ZUMBADOR, ZUMBADOR_FRECUENCIA);
            mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_NO_PINTAR);
            estado = bloqueado;
            bloqueado_f();      /* Para no perder el progreso del bucle, se llama directamente
                                a la callback */
            mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_PINTAR);
            t0 = tF;    // t0 y tF se igualan para ignorar el tiempo transcurrido en el bloqueo
        }

        elapsed_ms = (tF - t0) / 1000;  // Se calcula el tiempo transcurrido
        t_ejecucion += elapsed_ms;       // Se actualiza el tiempo de ejecucion. ¿Precision adecuada?

        t0 = micros();

        // Angulo recorrido en el elapsed_ms
        int angulo_recorrido = ANGULO_POR_MS * elapsed_ms;

        // Se actualizan los angulos de brazo y motor
        base_angulo_actual = (base_angulo_actual + angulo_recorrido) % 360;

        brazo_angulo_actual += signo_brazo * angulo_recorrido;
        clamp = constrain(brazo_angulo_actual, ANGULO_BRAZO_ABAJO, ANGULO_BRAZO_ARRIBA);
        if (clamp != brazo_angulo_actual)   // Si el brazo ha excedido sus angulos
        {
            signo_brazo = -1 * signo_brazo;
        }
        brazo_angulo_actual = clamp;

        // Desplazar los motores
        mover_motor(servo_base, PIN_MOTOR_BASE, base_angulo_actual);
        mover_motor(servo_brazo, PIN_MOTOR_BRAZO, brazo_angulo_actual);

        tF = micros();
    }

    // Finalizar el pintado
    digitalWrite(PIN_LED_TERMINADO, HIGH);
    digitalWrite(PIN_LED_EJECUTANDO, LOW);
    estado = terminado;
}

void bloqueado_f()
{
    // No proseguir hasta que el LDR no perciba oscuridad
    while (analogRead(PIN_LDR) >= LDR_HAY_LUZ)
    {
        delay(DELAY_BLOQUEADO); // Delay para que no se realice polling constante
    }
    digitalWrite(PIN_LED_EJECUTANDO, HIGH);
    analogWrite(PIN_ZUMBADOR, 0);
    estado = pintando;
}

void terminado_f()
{
    // Devolver los motores a sus posiciones por defecto
    mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_NO_PINTAR);
    mover_motor(servo_base, PIN_MOTOR_BASE, ANGULO_BASE_DEFECTO);
    mover_motor(servo_brazo, PIN_MOTOR_BRAZO, ANGULO_BRAZO_DEFECTO);
    delay(TIEMPO_FINALIZACION);     // Para asegurar que todo vuelve a su lugar
    digitalWrite(PIN_LED_TERMINADO, LOW);

    // Volver al estado en_espera
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
    delay(MOTOR_DELAY); // Breve delay para asegurar que el motor alcanza el angulo indicado
    servo.detach();
}

void abortar()
{
    /* Encender todos los leds para mostrar el error fisicamente y devolver los motores
    a sus posiciones por defecto. Posteriormente, acabar la ejecucion */
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
    // Se llama a la callback inicio_f() para realizar el setup
    inicio_f();
}

void loop()
{
    // Llamar a la callback correspondiente al estado actual
    (*funciones_estados[estado])();
}
