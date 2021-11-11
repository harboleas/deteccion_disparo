// Deteccion de disparo para SIMTAP

#define ADC_PIN 3
#define LASER_PIN 3
#define DISP_INVALIDO_PIN 2


// Umbrales para la deteccion
#define UMBRAL_1 40
#define UMBRAL_2 20

// Cantidad de picos a detectar en la ventana 2
#define PICOS_VENT_2 2

// Duracion de las ventanas de deteccion en microsegundos
#define T1 4000
#define T2 4000
#define T3 4000

// Duracion del pulso del laser en microsegundos
#define T_LASER 500000

enum Estados {
  ESPERA_SIGNAL,
  VENTANA_1,
  VENTANA_2,
  VENTANA_3,
  DISP_INVALIDO,
  DISP_OK };
  
Estados estado = ESPERA_SIGNAL;
unsigned long t_0, t;
int adc_val;
int cant_picos;  //Cantidad de picos que superan el umbral_x

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
//                Serial.println(estado);
                estado = VENTANA_1;
                t_0 = t;
            }
            break;

        case VENTANA_1:
            if (t - t_0 >= T1)
            {
//                Serial.println(estado);
                estado = VENTANA_2;
                cant_picos = 0;
                t_0 = t;
            }
            break;

        case VENTANA_2:
            if (adc_val >= UMBRAL_1)
            {
//                Serial.println(estado);
                estado = DISP_INVALIDO;
                t_0 = t;
            }
            else if (adc_val > UMBRAL_2)
                cant_picos++;

            if (t - t_0 >= T2)
            {
                if(cant_picos >= PICOS_VENT_2)
                {
//                    Serial.println(estado);
                    estado = VENTANA_3;
                    cant_picos = 0;
                    t_0 = t;
                }
                else
                {
//                    Serial.println(estado);
                    estado = DISP_INVALIDO;
                    t_0 = t;
                }
            }           
            break;
        
        case VENTANA_3:
           if (adc_val >= UMBRAL_2)
                cant_picos++;

            if (t - t_0 >= T3) 
            {
                if (cant_picos != 0)
                {
//                    Serial.println(estado);
                    estado = DISP_INVALIDO;
                    t_0 = t;
                }
                else
                {
//                    Serial.println(estado);
                    estado = DISP_OK;
                    t_0 = t;
                }
            }
            break;

        case DISP_INVALIDO:
            digitalWrite(DISP_INVALIDO_PIN, HIGH);
            if (t - t_0 >= T_LASER)
            {
                digitalWrite(DISP_INVALIDO_PIN, LOW);
//                Serial.println(estado);
                estado = ESPERA_SIGNAL;
            }
            break;

        case DISP_OK:
            digitalWrite(LASER_PIN, HIGH);
            if (t - t_0 >= T_LASER)
            {
                digitalWrite(LASER_PIN, LOW);
//                Serial.println(estado);
                estado = ESPERA_SIGNAL;
            }
            break;

        default:
            estado = ESPERA_SIGNAL;

   }

}
