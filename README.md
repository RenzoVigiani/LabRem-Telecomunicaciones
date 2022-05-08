# Protocolo para los laboratorios de "Telecomunicaciones"

### Json a enviar

    {"Estado": [ 0, true, false], "Analogicos": [180, 90, 15,52]}

## Sintaxis

**Estado**
Es un array conformado por 3 elementos en el siguiente orden: [Laboratorio, SubLab, Inicio del experimento]

|Laboratorio  | Sub Laboratorio  | Inicio del experimento | Laboratorio Seleccionado | Estado del experimento|
|-|-----|-----|-----------------------------|--------|
|2|true |true |Telecomunicaciones: Wifi 2.4G|Inicia  |
|2|true |false|Telecomunicaciones: Wifi 2.4G|Finaliza|
|2|false|true |Telecomunicaciones: Enlace de Radio por software |Inicia  |
|2|false|false|Telecomunicaciones: Enlace de Radio por software |Finaliza|
**Elementos por Laboratorio**

***WIFI 2.4***

Se utilizan 2 servo motores uno para cambio de angulo de azimut y otro para el angulo de elevación.

  - Variable a utilizar:
    - Analogico: [azimut, elevación]
      - Azimut: 0 - 180 [deg]
      - Elevación: 0 - 90 [deg]

***Enlace de Radio por software***

Se utiliza los parametros elegidos por el usuario en base a las siguientes tablas.

- Variable a utilizar:
  - Analogico: [Intensidad_Min, Intensidad_Max, Modulacion , Codificacion]

|Nivel de intensidad [dB] |  |
| ------------ | ------------ |
|Máximo        |  50 / 80 / 100 / 120|
|Minimo        |  10 / 15 / 20 / 25|

|Tipo de Modulación  |  Valor Analógico |
| ------------ | ------------ |
| 4-QAM| 1 |
| 8-QAM| 2 |
| 16-QAM| 3 |
| PSK| 4 |
| FSK| 5 |
| QPSK| 6 |

|Tipo de Codificación | Valor Analógico |
| ------------ | ------------ |
|Codificación 1|  1 |
|Codificación 2|  2 |
|Codificación 3|  3 |
**Enlace Wifi 2.4 GHz**

Diagrama general

![Enlace Wifi 2.4 GHz](https://raw.githubusercontent.com/RenzoVigiani/Laboratorios-Remotos/Telecomunicaciones/Imagenes/wifi2-4GHz.png)  
**Enlace Radio definido por Software**

Diagrama general

![Enlace Radio definido por Software](https://raw.githubusercontent.com/RenzoVigiani/Laboratorios-Remotos/Telecomunicaciones/Imagenes/sdr.png)
