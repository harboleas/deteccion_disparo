# genera un archivo .RAF para el generador de onda RIGOL
import numpy as np

def read_csv(file_name, cap_resol):
    "Abre archivos creados con osci."
    f = open(file_name, "r")
    lines = f.readlines()
    datos = np.array([int(x) for x in lines[13:-1]])
    # desplazo los datos y hago la conversion a 16 bits
    offset = 2**(cap_resol - 1)
    return (datos + offset) * 2**(14-cap_resol)

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
