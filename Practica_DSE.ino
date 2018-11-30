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
  pinMode(PIN_BOTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), int_boton, RISING);
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
  Serial.println("Ejecutando listo");

  Serial.println(analogRead(PIN_LDR));
  // Si el LDR recibe luz, pasar al estado en_espera
  if (analogRead(PIN_LDR) >= LDR_HAY_LUZ)
  {
    digitalWrite(PIN_LED_LISTO, LOW);
    analogWrite(PIN_ZUMBADOR, ZUMBADOR_FRECUENCIA);
    estado = en_espera;
    return;
  }
}

void en_espera_f()
{
  Serial.println("Ejecutando en_espera_f");

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
  Serial.println("Ejecutando pintando_f");

  unsigned long t_ejecucion = 0.0; // Tiempo que lleva pintando el programa
  unsigned long t0 = 0;            /* t0 y tF se usan para medir el tiempo transcurrido entre
                                        iteraciones del bucle. Almacenan microsegundos */
  unsigned long tF = 0.0;
  unsigned long elapsed_ms = 0.0; // Diferencia entre t0 y tF en milisegundos
  int clamp = 0;                  /* Variable auxiliar que se utiliza para ajustar el
                                        angulo del brazo */

  // Se inicializan las variables de control para los angulos
  base_angulo_actual = ANGULO_BASE_DEFECTO;
  brazo_angulo_actual = ANGULO_BRAZO_DEFECTO;
  mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_PINTAR);

  // Bucle de pintado
  while (t_ejecucion <= TIEMPO_PINTADO)
  {
    print_double(t_ejecucion, 2);
    Serial.println(analogRead(PIN_LDR));
    Serial.println();

    // Si se percibe luz, pasar al estado bloqueado
    if (analogRead(PIN_LDR) >= LDR_HAY_LUZ)
    {
      Serial.println("Se ha detectado luz. Pasando al estado bloqueado");
      digitalWrite(PIN_LED_EJECUTANDO, LOW);
      analogWrite(PIN_ZUMBADOR, ZUMBADOR_FRECUENCIA);
      mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_NO_PINTAR);
      estado = bloqueado;
      bloqueado_f(); /* Para no perder el progreso del bucle, se llama directamente
                                a la callback */
      mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_PINTAR);
      t0 = tF; // t0 y tF se igualan para ignorar el tiempo transcurrido en el bloqueo
    }

    elapsed_ms = (tF - t0) / 1000; // Se calcula el tiempo transcurrido
    t_ejecucion += elapsed_ms;     // Se actualiza el tiempo de ejecucion. ¿Precision adecuada?

    t0 = micros();

    // Angulo recorrido en el elapsed_ms
    int angulo_recorrido = ANGULO_POR_MS * elapsed_ms;

    // Se actualizan los angulos de brazo y motor
    base_angulo_actual = (base_angulo_actual + angulo_recorrido) % 360;

    brazo_angulo_actual += signo_brazo * angulo_recorrido;
    clamp = constrain(brazo_angulo_actual, ANGULO_BRAZO_ABAJO, ANGULO_BRAZO_ARRIBA);
    if (clamp != brazo_angulo_actual) // Si el brazo ha excedido sus angulos
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
  Serial.println("Ejecutando bloqueado_f");

  // No proseguir hasta que el LDR no perciba oscuridad
  while (analogRead(PIN_LDR) >= LDR_HAY_LUZ)
  {
    delay(DELAY_BLOQUEADO); // Delay para que no se realice polling constante
  }
  Serial.println("Se ha detectado oscuridad. Pasando al estado pintando");
  digitalWrite(PIN_LED_EJECUTANDO, HIGH);
  analogWrite(PIN_ZUMBADOR, 0);
  estado = pintando;
}

void terminado_f()
{
  Serial.println("Ejecutando terminado_f");

  // Devolver los motores a sus posiciones por defecto
  mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_NO_PINTAR);
  mover_motor(servo_base, PIN_MOTOR_BASE, ANGULO_BASE_DEFECTO);
  mover_motor(servo_brazo, PIN_MOTOR_BRAZO, ANGULO_BRAZO_DEFECTO);
  delay(TIEMPO_FINALIZACION); // Para asegurar que todo vuelve a su lugar
  digitalWrite(PIN_LED_TERMINADO, LOW);

  // Volver al estado en_espera
  Serial.println("Pasando al estado en_espera");
  estado = en_espera;
}

void error_f()
{
  Serial.println("Ejecutando error_f");

  Serial.println("Error: El sistema no deberia alcanzar el estado error");
  abortar();
}

void int_boton()
{
  Serial.println("Se ha pulsado el boton");
  if (estado == listo)
  {
    Serial.println("Se va a pasar al estado pintando");
    estado = pintando;
  }
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
  Serial.println("Abortando ejecucion");
  /* Encender todos los leds para mostrar el error fisicamente y devolver los motores
    a sus posiciones por defecto. Posteriormente, acabar la ejecucion */
  digitalWrite(PIN_LED_EJECUTANDO, HIGH);
  digitalWrite(PIN_LED_LISTO, HIGH);
  digitalWrite(PIN_LED_TERMINADO, HIGH);
  mover_motor(servo_base, PIN_MOTOR_BASE, ANGULO_BASE_DEFECTO);
  mover_motor(servo_brazo, PIN_MOTOR_BRAZO, ANGULO_BRAZO_DEFECTO);
  mover_motor(servo_spray, PIN_MOTOR_SPRAY, ANGULO_SPRAY_PINTAR);
  delay(UINT_MAX); // No existe una instruccion halt o stop
}

void print_double(double val, unsigned int precision)
{
  // prints val with number of decimal places determine by precision
  // NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
  // example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

  Serial.print(int(val)); //prints the int part
  Serial.print(".");      // print the decimal point
  unsigned int frac;
  if (val >= 0)
    frac = (val - int(val)) * precision;
  else
    frac = (int(val) - val) * precision;
  int frac1 = frac;
  while (frac1 /= 10)
    precision /= 10;
  precision /= 10;
  while (precision /= 10)
    Serial.print("0");

  Serial.println(frac, DEC);
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
  delay(1000);
}
