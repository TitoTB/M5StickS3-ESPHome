# M5StickS3 ESPHome

Integracion inicial para el M5Stack StickS3 ESP32-S3 Mini IoT Dev Kit en Home Assistant mediante ESPHome.

## Estado

Esta v1 incluye:

- Pantalla M5StickS3 mediante componente ESPHome propio.
- Gestion basica de energia mediante M5PM1.
- Boton azul y boton lateral.
- Retroiluminacion de pantalla.
- Bip de confirmacion mediante el altavoz interno.
- Microfono mediante componente propio M5Unified/M5.Mic para asistente de voz nativo de Home Assistant.
- Asistente activado al mantener pulsado el boton azul 800 ms, con bip al empezar a escuchar y bip al ejecutar el comando.

IR y lectura de bateria quedan fuera de esta v1 para mantener estable pantalla, botones, bip y asistente.

## Uso

1. Copia `stick-s3.yaml` en ESPHome Builder.
2. Define estos secretos en ESPHome:

```yaml
wifi_ssid: "..."
wifi_password: "..."
```

3. Reemplaza en `stick-s3.yaml` los placeholders de `api.encryption.key`, `ota.password` y `wifi.ap.password` por los valores que te genere ESPHome.

Nota temporal: `external_components.refresh` esta en `0s` durante el desarrollo para forzar que ESPHome descargue una copia fresca del repositorio. Cuando la base compile estable, conviene volver a `refresh: 1days`.
