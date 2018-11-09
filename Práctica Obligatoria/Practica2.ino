const int PIN_LDR = 0;  // El LDR usa una resistencia de 10 kohmios

const int PINS_LED[] = { 2, 3, 4, 5 };   // Los LEDs usan resistencias de 220 ohmios
const int N_LEDS = 4;
int valor = 0;

void setup()
{
  // Establecemos todas las conexiones de los LEDS como salidas
  for (int pin_led = 0; pin_led != N_LEDS; ++pin_led)
  {
    pinMode(PINS_LED[pin_led], OUTPUT);
  }

  Serial.begin(9600); // Abrimos la comunicación serie para debug
}

void loop()
{
  int lectura_LDR = analogRead(PIN_LDR);  // Leemos el LDR
  // Calculamos cuántos LEDs deben encenderse
  int nivel = floor(lectura_LDR / 256); // Elegimos 4 niveles

  randomSeed(lectura_LDR);

  switch (nivel)
  {
    case 0:
      valor = random(0, 4);
      break;
    case 1:
      valor = random(4, 8);
      break;
    case 2:
      valor = random(8, 12);
      break;
    case 3:
      valor = random(12, 16);
      break;
    default:
      valor = 0;
      break;
  }

  for (int pin_led = N_LEDS; pin_led >= 0; pin_led--)
  {
    int k = valor >> pin_led;
    digitalWrite(PINS_LED[pin_led], k & 1);
  }

  Serial.println(lectura_LDR);
  Serial.println(nivel);
  Serial.println(valor);
  Serial.println();

  delay(200);
  for (int pin_led = 0; pin_led != N_LEDS; ++pin_led)
  {
    digitalWrite(PINS_LED[pin_led], LOW);
  }
  // delay(200);
}
