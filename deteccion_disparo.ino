// Deteccion de disparo para SIMTAP

// Descomentar la siguiente linea para simulacion del piezo  
#define SIMU

// Definicion de pines
#define ADC_PIN 3
#define LASER_PIN 3
#define DISP_INVALIDO_PIN 2

// Umbral para la deteccion
#define UMBRAL_DET_SIGNAL 205   // 1 Volt

// Duracion de las ventanas de deteccion en microsegundos
#define T1 23000L
#define T2 40000L

// Duracion del pulso del laser en microsegundos
#define T_LASER 1000L

// Tiempo de espera hasta la proxima deteccion
#define T_FIN 150000L

////// Declaracion de variables ////////
enum Estados {
  ESPERA_SIGNAL,
  VENTANA_1,
  VENTANA_2,
  DISP_INVALIDO,
  DISP_OK,
  ESPERA_FIN};
  
Estados estado;
unsigned long t_0, t;
int adc_val;
int cant_detec;

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


#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup()
{
    
    pinMode(LASER_PIN, OUTPUT);
    pinMode(DISP_INVALIDO_PIN, OUTPUT);
    Mult = 1;
    estado = ESPERA_SIGNAL;

    #ifdef FASTADC
      // set prescale to 16
      sbi(ADCSRA,ADPS2) ;
      cbi(ADCSRA,ADPS1) ;
      cbi(ADCSRA,ADPS0) ;
    #endif

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
                cant_detec = 0;
                #ifdef SIMU 
                    Serial.println(estado); //Para ver el cambio de estado
                #endif 
            }
            break;

        case VENTANA_1:
//            if (adc_val >= UMBRAL_DET_SIGNAL)    
//                cant_detec++;

            if (t - t_0 >= (T1 * Mult))
            {
/*
                if (cant_detec > 0)
                {
                    estado = VENTANA_2;
                    t_0 = t;
                    cant_detec = 0;

                    #ifdef SIMU 
                        Serial.println(estado);
                    #endif 

                }
                else
                {
                    estado = DISP_INVALIDO;
                    t_0 = t;
                    cant_detec = 0;

                    #ifdef SIMU 
                        Serial.println(estado);
                    #endif 
                }
*/
                    estado = VENTANA_2;
                    t_0 = t;
                    cant_detec = 0;

                    #ifdef SIMU 
                        Serial.println(estado);
                    #endif 


            }
            break;

        case VENTANA_2:
            if (adc_val >= UMBRAL_DET_SIGNAL)
                cant_detec++;

            if (t - t_0 >= (T2 * Mult))
            {
                if (cant_detec == 0)
                {
                    estado = DISP_OK;
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
        
       case DISP_INVALIDO:
            digitalWrite(DISP_INVALIDO_PIN, HIGH);
            if (t - t_0 >= (T_LASER * Mult))
            {
                digitalWrite(DISP_INVALIDO_PIN, LOW);
                estado = ESPERA_FIN;
                t_0 = t;

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
                t_0 = t;

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
