
// Usamos resistencias de 220 ohmios para los LEDs y de 10 kOhmiospara el sensor de luminosidad.
int sensor = 0; // LDR
int val = 0; // Valor devuelto por el LDR
int nivel = 0;
int led[] = {2, 3, 4, 5};
int valor = 0;
void setup() {
  // Establecemos todas las conexiones de los LEDS como salidas
  for (int i = 0; i < 4; i++)
  {
    pinMode(led[i] , OUTPUT);
  }

  Serial.begin(9600); // Abrimos la comunicación serie
}

void loop() {
  val = analogRead(sensor); // Leemos el sensor de luminosidad
  // Calculamos cuántos512 LEDs deben encenderse. Recordemos que lamedida del sensor va de 0 a 1023 y tenemos 10 LEDs.
  nivel = floor(val / 256); // Elegimos 4 niveles

  randomSeed(val);

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

  for (int c = 3; c >= 0; c--)
  {
    int k = valor >> c;
    digitalWrite(led[c], k & 1);
  }

  Serial.println(val);
  Serial.println(nivel);
  Serial.println(valor);
  Serial.println();

  delay(100);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(led[i], LOW);
  }
  delay(100);

}
