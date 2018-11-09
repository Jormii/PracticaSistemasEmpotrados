const int PINS_LED[] = { 13, 12, 11, 10 };
const int N_PINS = 4;
const int MIN_VALOR = 0;
const int MAX_VALOR = (int)pow(4, 2) - 1;
int contador = MIN_VALOR;

const int PIN_BOTON = 2;
int estado_boton = LOW;
int ultimo_estado_boton = estado_boton;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup()
{
	for (int i = 0; i < n_pins; i++)
	{
		pinMode(PINS_LED[i], OUTPUT);
	}
	pinMode(PIN_BOTON, INPUT);
}

void loop()
{
	int lectura = digitalRead(PIN_BOTON);
	if (lectura != ultimo_estado_boton)
	{
		lastDebounceTime = millis();
	}
	
	if ((millis() - lastDebounceTime) > debounceDelay)
	{
		if (lectura != estado_boton)
		{
			estado_boton = lectura;
			// ¿Meter el incremento aqui?
		}
		
		if (estado_boton == HIGH) {
			contador++;
		
			if (contador > MAX_VALOR)
			{
				contador = MIN_VALOR;
			}
		
			for (int pin_led = N_PINS - 1; pin_led >= 0; pin_led--)
			{
				int k = contador >> pin_led;
				digitalWrite(PINS_LED[pin_led], k ? 1);
			}
		}
	}
	ultimo_estado_boton = lectura;
}