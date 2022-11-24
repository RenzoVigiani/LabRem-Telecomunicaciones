# Protocolo para los laboratorios de "Telecomunicaciones"
El procedimiento de comunicación es el siguiente.
Cada vez que se quiere modificar algun dato en el laboratorio. El Servidor debe enviar un
```Ruby 
POST /HTTP/1.1 + <JSON a enviar>
```

#### Json a enviar (ejemplo)

```Ruby 
{"Estado": [0,true,false],"Analogico": [45,90]}
```

Y para obtener avances de los datos o reporte de los datos debe enviar un:

```Ruby 
GET /HTTP/1.1
```

De esta forma el Arduino responde ante una petición.

#### Json a recibir (ejemplo)

```Ruby  
{ "Estado": [0,true,false],"Analogico": [50,85], "Error": 0}
```

## Sintaxis

**Estado**
Es un array conformado por 3 elementos en el siguiente orden: [Laboratorio, SubLab, Inicio del experimento]

|Laboratorio  | Sub Laboratorio  | Inicio del experimento | Laboratorio Seleccionado | Estado del experimento|
|-|-----|-----|-----------------------------|--------|
|2|true |true |Telecomunicaciones: Wifi 2.4G|Inicia  |
|2|true |false|Telecomunicaciones: Wifi 2.4G|Finaliza|

**Error**
Es una variable numerica que representa un mensaje de error.

| Tipo de error                         |  Valor  |
| ----------------------------------    |---------|
| Sin errores                           |    0    |
| Error limites de angulo de azimut.    |    1    |
| Error limites de angulo de elevación  |    2    |
| Error de laboratorio incorrecto       |    3    |

**Elementos por Laboratorio**

***WIFI 2.4***

Se utilizan 2 servo motores uno para cambio de angulo de azimut y otro para el angulo de elevación.

- Variable a utilizar:
  - Analogico: [azimut, elevación]
    - Azimut: 0 - 180 [deg]
    - Elevación: 0 - 180 [deg]

***Enlace de Radio por software (A definir)***
Here is a simple flow chart:

> ¡No se realiza con ARDUINO!

Se utiliza los parametros elegidos por el usuario en base a las siguientes tablas.

- Variable a utilizar:
  - Analogico: [Intensidad_Min, Intensidad_Max, Modulacion , Codificacion]

|Nivel de intensidad | [dB] |
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

## Diagramas


**Arduino Mega 2560**
Pin Out
<img alt = "Arduino Mega" src="https://raw.githubusercontent.com/RenzoVigiani/LabRem-SistemasDig/main/Imagenes/Arduino-Mega-Pinout.png" width=1920>

**Enlace Wifi 2.4 GHz**
Diagrama general

<img alt = "Enlace Wifi 2.4 GHz" src="https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/wifi2-4GHz.png" width=1920>

<img alt = "Enlace Wifi 2.4 GHz" src="https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/diagrama_en_lab.png" width=1920>
Vista frontal

<img alt ="Vista frontal" src="https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/wifi-vista-frontal.png" width=1920>

**Enlace Radio definido por Software**
Diagrama general

<img alt = "Enlace Radio definido por Software" src="https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/sdr.png" width=1920>

| |Diseños 3D ||
|--|--|--|
|Caja sistema elevación|![Caja servo Elevación](https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/teleco_3D/Caja_servo_1.jpg)|![Caja servo Elevación](https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/teleco_3D/Caja_servo_5.jpg)|
|Soporte sistema azimut|![Caja servo Elevación](https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/teleco_3D/Soporte_caja_1.jpg)|![Caja servo Elevación](https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/teleco_3D/Soporte_caja_2.jpg)|
|Sistema de movimiento Azimut-Elevación|![Caja servo Elevación](https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/teleco_3D/Completo_4.jpg)![Sistema de movimiento Azimut - Elevación](https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/teleco_3D/Completo_2.jpg)|![Sistema de movimiento Azimut - Elevación](https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/teleco_3D/skp_diseños.png)![Sistema de movimiento Azimut - Elevación](https://raw.githubusercontent.com/RenzoVigiani/LabRem-Telecomunicaciones/main/Imagenes/teleco_3D/Completo_5.jpg)|
