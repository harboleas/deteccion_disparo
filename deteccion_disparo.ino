// Deteccion de disparo para SIMTAP

// Umbrales para la deteccion
#define UMBRAL_1 100
#define UMBRAL_2 50

// Duracion de las ventanas de deteccion en milisegundos
#define T1 0.3
#define T2 2
#define T3 6

enum Estados {
  ESPERA_SIGNAL,
  VENTANA_1,
  VENTANA_2,
  DISP_INVALIDO,
  DISP_OK };
  
Estados estado = ESPERA_SIGNAL;

void setup()
{
  
}

void loop()
{
  
}

