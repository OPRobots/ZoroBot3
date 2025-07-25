# MMSimulator API para ZoroBot3

![MMSim Japan2019](../../../images/mmsim_japan_2019.png "MMSim Japan2019")
Este directorio contiene la API de [MMSimulator](https://github.com/OPRobots/mms) para ZoroBot3, que permite la simulación y control del robot en un entorno virtual.

## Configuración de MMS
Para utilizar la API de [MMSimulator](https://github.com/OPRobots/mms) es necesario configurar el entorno de simulación. Asegúrate de seguir las instrucciones de instalación y configuración de su repositorio.

### Añadir API
Esta es la configuración de esta API:


![MMSim API Config](../../../images/mmsim_api_config.png "MMSim API Config")

  - **Name**: ZoroBot3.
  - **Directory**: La ruta en la que tengas el API. Ej.:  `F:\ZoroBot3\source_code\lib\mmsim_api`.
  - **Build command**: `gcc -I../../lib/mmsim_api -I../../include mmsim_api.c ../../src/control.c ../../src/floodfill.c ../../src/maze.c ../../src/move.c ../../src/floodfill_weigths.c ../../src/sensors.c ../../src/menu_run.c mmsim.c   -o mmsim.exe -D MMSIM_ENABLED`.
  - **Run Command**: La ruta en la que tengas el compilado del API. Ej.: `F:\ZoroBot3\source_code\lib\mmsim_api\mmsim.exe`.