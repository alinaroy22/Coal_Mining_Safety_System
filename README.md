# Coal_Mining_Safety_System
CDAC Project : Coal Mining Safety System using ESP32 and Cloud Platform

A mobile device created based on ESP32 DevKit and DHT22 and MQ-7 sensors to detect temperature and
Air Quality anomaly inside coal mine tunnels. It alerts the user as well as the Control Room 
through IoT platform. FreeRTOS has been used to run the tasks concurrently and with precision 
to get the sensor data and send it to the cloud platform using MQTT protocol. Espressif IDF has 
been used as a platform and C language to build the logic.

These safety devices are able to measure exposures in near real time. If a measurement
collected exceeds threshold limits, mine operators must take corrective actions
immediately. In addition, miners wearing these devices receive information about their
personal exposures and sometimes can modify their locations within a mine in response
to elevated readings. These early detection devices will play a crucial role in evading any
potential disaster.

