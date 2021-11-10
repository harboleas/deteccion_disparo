// Deteccion de disparo para SIMTAP

#define ADC_PIN 3

// Umbrales para la deteccion
#define UMBRAL_1 100
#define UMBRAL_2 50

// Duracion de las ventanas de deteccion en microsegundos
#define T1 0.3
#define T2 2
#define T3 6

// Duracion del pulso del laser en microsegundos
#define T_LASER 2000

enum Estados {
  ESPERA_SIGNAL,
  VENTANA_1,
  VENTANA_2,
  VENTANA_2B,
  VENTANA_3,
  DISP_INVALIDO,
  DISP_OK };
  
Estados estado = ESPERA_SIGNAL;
unsigned long t_0, t;
int adc_val;


/////// Simulacion de muestreo ////////

union B {
    int val;
    char _byte[2];
    } dato;

// Obtiene la senal por el USB

int simula_muestreo()
{
    
    if (Serial.available() >= 2)
    {
        Serial.readBytes(dato._byte, 2);
        return dato.val;
    };

    return 0;
}

////////////////////////////////////////

void setup()
{

    Serial.begin(112000);  // Solo para simulacion  

}

void loop()
{
    // Tiempo actual
    t = micros();

    // Muestreo 
    //adc_val = analogRead(ADC_PIN);    
    adc_val = simula_muestreo();   // Solo para simulacion

    switch(estado)
    {
        case ESPERA_SIGNAL:
            if (adc_val >= UMBRAL_1)
            {
                estado = VENTANA_1;
                t_0 = t;
            }
            break;

        case VENTANA_1:
            if (t - t_0 >= T1)
            {
                estado = VENTANA_2;
                t_0 = t;
            }
            break;

        case VENTANA_2:
    }

}

