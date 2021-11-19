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

// Duracion de las ventanas de deteccion en microsegundos
#define T1 600L
#define T2 3500L
#define T3 8000L

// Duracion del pulso del laser en microsegundos
#define T_LASER 1000L

// Tiempo de espera hasta la proxima deteccion
#define T_FIN 10000L

////// Declaracion de variables ////////
enum Estados {
  ESPERA_SIGNAL,
  VENTANA_1,
  VENTANA_2,
  VENTANA_3,
  DISP_INVALIDO,
  DISP_OK,
  ESPERA_FIN};
  
Estados estado;
unsigned long t_0, t;
int adc_val;
bool supero_umbral_acept;  

// Debido a que en la simulacion no puedo suministrar las muestrar
// con la misma frecuencia que lo realiza el conversor AD
// utilizo un multiplicador para la duracion de las ventanas
int Mult;

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
    
    pinMode(LASER_PIN, OUTPUT);
    pinMode(DISP_INVALIDO_PIN, OUTPUT);
    Mult = 1;
    estado = ESPERA_SIGNAL;

    #ifdef SIMU
        Serial.begin(115200);  // Solo para simulacion  
        // Lee el valor del multiplicador, el simulador antes de enviar 
        // este dato, resetea el arduino
        Serial.readBytes(dato.bytes, 2);
        Mult = dato.val;
        Serial.println(Mult);
    #endif

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
                t_0 = t; // Reset del delta t 
                // Resetea condicion de aceptacion para la sig. ventana
                supero_umbral_acept = false;  
                
                #ifdef SIMU 
                    Serial.println(estado); //Para ver el cambio de estado
                #endif 
            }
            break;

        case VENTANA_1:
            if (adc_val >= UMBRAL_A_V1)    
                supero_umbral_acept = true;

            if (t - t_0 >= (T1 * Mult))
            {
                if (supero_umbral_acept)
                {
                    estado = VENTANA_2;
                    t_0 = t;
                    supero_umbral_acept = false;

                    #ifdef SIMU 
                        Serial.println(estado);
                    #endif 

                }
                else
                {
                    estado = DISP_INVALIDO;
                    t_0 = t;

                    #ifdef SIMU 
                        Serial.println(estado);
                    #endif 
                }
            }
            break;

        case VENTANA_2:
            if (adc_val >= UMBRAL_R_V2)
            {
                estado = DISP_INVALIDO;
                t_0 = t;

                #ifdef SIMU 
                    Serial.println(estado);
                #endif 
                
                break;
            }
            else if (adc_val >= UMBRAL_A_V2)
                supero_umbral_acept = true;

            if (t - t_0 >= (T2 * Mult))
            {
                if (supero_umbral_acept)
                {
                    estado = VENTANA_3;
                    t_0 = t;
 
                    #ifdef SIMU 
                        Serial.println(estado);
                    #endif 
                }
                else
                {
                    estado = DISP_INVALIDO;
                    t_0 = t;

                    #ifdef SIMU 
                        Serial.println(estado);
                    #endif 
                }
            }           
            break;
        
        case VENTANA_3:
            if (adc_val >= UMBRAL_R_V3)
            {
                estado = DISP_INVALIDO;
                t_0 = t;

                #ifdef SIMU 
                    Serial.println(estado);
                #endif 

                break;
            }
 
            if (t - t_0 >= (T3 * Mult)) 
            {
                estado = DISP_OK;
                t_0 = t;

                #ifdef SIMU 
                    Serial.println(estado);
                #endif 

            }
            break;

        case DISP_INVALIDO:
            digitalWrite(DISP_INVALIDO_PIN, HIGH);
            if (t - t_0 >= (T_LASER * Mult))
            {
                digitalWrite(DISP_INVALIDO_PIN, LOW);
                estado = ESPERA_FIN;

                #ifdef SIMU 
                    Serial.println(estado);
                #endif 

            }
            break;

        case DISP_OK:
            digitalWrite(LASER_PIN, HIGH);
            if (t - t_0 >= (T_LASER * Mult))
            {
                digitalWrite(LASER_PIN, LOW);
                estado = ESPERA_FIN;

                #ifdef SIMU 
                    Serial.println(estado);
                #endif 

            }
            break;

        case ESPERA_FIN:

            if (t - t_0 >= (T_FIN * Mult))
            {
                estado = ESPERA_SIGNAL;

                #ifdef SIMU 
                    Serial.println(estado);
                #endif 

            }
            break;
        
        default:
            estado = ESPERA_SIGNAL;

   }

}
