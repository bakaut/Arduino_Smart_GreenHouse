# Arduino_Smart_GreenHouse
Умная теплиа на ардуино.

Цель:
Контроль параметров теплицы. Температура внутри и снаружи. Влажность воздуха. Влажность почтвы в 2-3 местах. Освещённость. Видеонаблюдение ( раз в час фото или видеосъёмка). Контроль закрытия дверей и форточек.

И возможность эти параметры получать и регулировать удалённо со своего сотового телефона. Или с веб интерфейса. Полив почвы. Открытие, закрытие форточек (регулировка тепмературы и влажности). Обогрев теплицы. 

Даные датчиков теплицы отправляются через gprs в формате для influxdb в облако раз в 30 минут. Возможно уведомления в slack через http запросы. Так же данные с датчиков сохраняются на sd карту.

Датчики:
-Температура внутри и снаружи, влажность, 
-Давление, высота над уровнем моря,
-Освещённость (фоторезистор)

Ардуино каждую минуту записывает показания датчиков  в формате csv на sd карту и через gprs отправляет их в облако ( http://www.blynk.cc/ || https://thingspeak.com || https://narodmon.ru/ || http://majordomo.smartliving.ru/Main/HomePage || http://pdacontrolen.com || http://www.openhab.org/ https://internetofthings.ibmcloud.com/#/ https://www.ibm.com/internet-of-things https://www.ibm.com/internet-of-things/spotlight/watson-iot-platform/pricing https://habrahabr.ru/company/ibm/blog/318702/)

Возможно не csv использовать, а mqtt  протокол и базу influxdb и сразу отправлять в облако

https://www.influxdata.com/how-to-send-sensor-data-to-influxdb-from-an-arduino-uno/

https://ipc2u.ru/articles/prostye-resheniya/chto-takoe-mqtt/

https://community.openhab.org/t/influxdb-grafana-persistence-and-graphing/13761

https://geektimes.ru/post/255352/

http://forum.amperka.ru/threads/arduino-esp8266-raspberry-pi-2-openhab-Умный-дом-азы-управления.5043/#post-40253


Сделано:

Мониторинг температуры, давления, изменения давления, влажности и запись данных на sd карту каждый 10 минут в формате для influxdb.
