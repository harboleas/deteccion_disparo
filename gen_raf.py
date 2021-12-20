# genera un archivo .RAF para el generador de onda RIGOL
import numpy as np

def read_csv(file_name, cap_resol):
    "Abre archivos creados con osci."
    f = open(file_name, "r")
    lines = f.readlines()
    n = int(lines[0].split(",")[1])
    T = float(lines[10].split(",")[1])
    escala_V = float(lines[4].split(",")[1])
    V_unit = lines[3].split(",")[1]
    T_unit = lines[6].split(",")[1]
    print("Tiempo total:", n*T, T_unit)
    print("vmin:", -escala_V * 5, V_unit)
    print("Vmax:", escala_V * 5, V_unit)
    datos = np.array([int(x) for x in lines[13:-1]])
    # desplazo los datos, para obtener solo valores positivos
    # y luego hago la conversion de resolucion a 14 bits
    offset = 2**(cap_resol - 1)
    return (datos + offset) * 2**(14-cap_resol)


def recorta(datos, val_min, val_max):

    datos_proc = []
    for val in datos:
        if val <= val_min:
           datos_proc.append(val_min)
        elif val >= val_max:
            datos_proc.append(val_max)
        else:
            datos_proc.append(val)

    return np.array(datos_proc)


def gen_raf(file_name, datos):
    # interpolacion lineal de los datos para alcanzar los 2**14
    # puntos que necesita el archivo raf
    n = datos.size
    k = 2**14
    delta = (n-1) / (k-1)
    xp = np.linspace(0, n-1, n)
    x = np.linspace(0, n-1, k)
    datos_raf = np.interp(x, xp, datos)

    f = open(file_name, "wb")
    for val in datos_raf:
        f.write(int(val).to_bytes(2, "little"))
    f.close()
