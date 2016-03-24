## Networking setup 

Add the following lines to /etc/network/interfaces:

```
auto wlan0
iface wlan0 inet dhcp
    wpa-ssid <boat ssid>
    wpa-psk  <boat psk>
```

Generate wpa-psk by running the following command:

```
$ wpa_passphrase <boat ssid> <boat passphrase>
```

## rc.d changes

Run the following commands:

```
# update-rc.d -f apache2 remove
# update-rc.d -f lighttpd defaults
```
