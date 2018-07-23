# webtermometer
Web accessible six-channel ethernet thermometer based on Anduino Nano v3 board and ENC28J60 Ethernet board.

Has separate link for each channel with simple text/plain output to easy pairing with Zabbix agents

REQUIRED Libraries: 
- https://github.com/milesburton/Arduino-Temperature-Control-Library
- https://github.com/PaulStoffregen/OneWire
- https://github.com/jcw/ethercard       

Device is configured to use only DHCP address

Data can be accesses as:      
- index page - http://device_ip_from_dhcp/   e.g http://192.168.0.100
- single sensor channel  http://device_ip_from_dhcp/?channel=X   where X is {0..5} e.g http://192.168.0.100/?channel=4
- use serial monitor to read an ip or check your router/server's dhcp lease logs
