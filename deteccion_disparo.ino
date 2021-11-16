// Deteccion de disparo para SIMTAP

// Descomentar la siguiente linea para simulacion del piezo  
#define SIMU

// Definicion de pines
#define ADC_PIN 3
#define LASER_PIN 3
#define DISP_INVALIDO_PIN 2

// Umbrales para la deteccion
#define UMBRAL_DET_SIGNAL 12   // Deteccion de signal
#define UMBRAL_A_V1 40         // Aceptacion de Ventana 1
#define UMBRAL_A_V2 20         // Aceptacion de Ventana 2
#define UMBRAL_R_V2 40         // Rechazo de Ventana 2
#define UMBRAL_R_V3 18         // Rechazo Ventana 3

// Cantidad de picos a detectar en las ventanas 
#define PICOS_V1 1


// Duracion de las ventanas de deteccion en microsegundos
#define T1 500L
#define T2 1600L
#define T3 7500L

// Debido a que en la simulacion no puedo suministrar las muestrar
// con la misma frecuencia que lo realiza el conversor AD
// utilizo un multiplicador para la duracion de las ventanas
#ifndef SIMU
    #define MULT 1L
#else
    #define MULT 25L
#endif

// Duracion del pulso del laser en microsegundos
#define T_LASER 500000

////// Declaracion de variables ////////
enum Estados {
  ESPERA_SIGNAL,
  VENTANA_1,
  VENTANA_2,
  VENTANA_3,
  DISP_INVALIDO,
  DISP_OK };
  
Estados estado;
unsigned long t_0, t;
int adc_val;
bool supero_umbral_acept;  
////////////////////////////////////////

/////// Simulacion de muestreo ////////

union {
    int val;
    byte bytes[2];
    } dato;

// Obtiene la senal por el USB

int simula_muestreo()
{
    
    if (Serial.available() >= 2)
    {
        Serial.readBytes(dato.bytes, 2);
        return dato.val;
    };

    return 0;

}
////////////////////////////////////////

void setup()
{

    Serial.begin(115200);  // Solo para simulacion  
    pinMode(LASER_PIN, OUTPUT);
    pinMode(DISP_INVALIDO_PIN, OUTPUT);
    estado = ESPERA_SIGNAL;
}

void loop()
{
    // Tiempo actual
    t = micros();

    // Muestreo 
#ifndef SIMU 
    adc_val = analogRead(ADC_PIN);    
#else
    adc_val = simula_muestreo();   
#endif

// FSM
    switch(estado)
    {
        case ESPERA_SIGNAL:
            if (adc_val >= UMBRAL_DET_SIGNAL)
            {
                estado = VENTANA_1;
                Serial.println(estado);
                t_0 = t; // Reset del delta t 
                // Resetea condicion de aceptacion para la sig. ventana
                supero_umbral_acept = false;  
            }
            break;

        case VENTANA_1:
            if (adc_val >= UMBRAL_A_V1)    
                supero_umbral_acept = true;

            if (t - t_0 >= (T1 * MULT))
            {
                if (supero_umbral_acept)
                {
                    estado = VENTANA_2;
                    Serial.println(estado);
                    t_0 = t;
                    supero_umbral_acept = false;
                }
                else
                {
                    estado = DISP_INVALIDO;
                    Serial.println(estado);
                    t_0 = t;
                }
            }
            break;

        case VENTANA_2:
            if (adc_val >= UMBRAL_R_V2)
            {
                estado = DISP_INVALIDO;
                Serial.println(estado);
                t_0 = t;
                break;
            }
            else if (adc_val >= UMBRAL_A_V2)
                supero_umbral_acept = true;

            if (t - t_0 >= (T2 * MULT))
            {
                if (supero_umbral_acept)
                {
                    estado = VENTANA_3;
                    Serial.println(estado);
                    t_0 = t;
                }
                else
                {
                    estado = DISP_INVALIDO;
                    Serial.println(estado);
                    t_0 = t;
                }
            }           
            break;
        
        case VENTANA_3:
            if (adc_val >= UMBRAL_R_V3)
            {
                estado = DISP_INVALIDO;
                Serial.println(estado);
                t_0 = t;
                break;
            }
 
            if (t - t_0 >= (T3 * MULT)) 
            {
                estado = DISP_OK;
                Serial.println(estado);
                t_0 = t;
            }
            break;

        case DISP_INVALIDO:
            digitalWrite(DISP_INVALIDO_PIN, HIGH);
            if (t - t_0 >= T_LASER)
            {
                digitalWrite(DISP_INVALIDO_PIN, LOW);
                estado = ESPERA_SIGNAL;
                Serial.println(estado);
            }
            break;

        case DISP_OK:
            digitalWrite(LASER_PIN, HIGH);
            if (t - t_0 >= T_LASER)
            {
                digitalWrite(LASER_PIN, LOW);
                estado = ESPERA_SIGNAL;
                Serial.println(estado);
            }
            break;

        default:
            estado = ESPERA_SIGNAL;

   }

}
