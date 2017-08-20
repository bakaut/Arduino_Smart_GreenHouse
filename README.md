# Arduino_Meteo_Station
Метеостанция на ардуино.
Датчики:
-Температура, влажность, 
-Давление, высота над уровнем моря,
-Освещённость (фоторезистор)

Ардуино каждую минуту записывает показания датчиков  в формате csv на sd карту и через gprs отправляет их в облако ( http://www.blynk.cc/ || https://thingspeak.com || https://narodmon.ru/ || http://majordomo.smartliving.ru/Main/HomePage || http://pdacontrolen.com || http://www.openhab.org/)

Возможно не csv использовать, а mqtt  протокол и базу influxdb и сразу отправлять в облако

https://www.influxdata.com/how-to-send-sensor-data-to-influxdb-from-an-arduino-uno/

https://ipc2u.ru/articles/prostye-resheniya/chto-takoe-mqtt/

https://community.openhab.org/t/influxdb-grafana-persistence-and-graphing/13761

https://geektimes.ru/post/255352/
