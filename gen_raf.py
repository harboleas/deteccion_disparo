# genera un archivo .RAF para el generador de onda RIGOL
import numpy as np

def read_csv(file_name):
    "Abre archivos creados con osci. RIGOL"
    f = open(file_name, "r")
    lines = f.readlines()
    n = len(lines) - 2
    T = (float(lines[-1].split(",")[0]) - float(lines[2].split(",")[0])) / n
    print("Tiempo total:", n*T, " s")
    datos = []
    for line in lines[2:]:
        datos.append(float(line.split(",")[1]))

    datos = np.array(datos)
    min_ = datos.min()
    datos = datos - min_
    max_ = datos.max()
    resol = max_ / 2**14

    datos_14b = np.array([int(x/resol) for x in datos])

    return datos_14b


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
