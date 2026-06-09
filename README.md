# M5StickS3 ESPHome

Integracion inicial para el M5Stack StickS3 ESP32-S3 Mini IoT Dev Kit en Home Assistant mediante ESPHome.

## Estado

Esta v1 incluye:

- Pantalla M5StickS3 mediante componente ESPHome propio.
- Gestion basica de energia mediante `M5.Power`.
- Bateria, voltaje, corriente de bateria, estado de carga y salida EXT 5V.
- Boton azul y boton lateral.
- Retroiluminacion de pantalla.
- Emisor y receptor IR.
- Diagnostico Wi-Fi, uptime, IP, SSID y MAC.

Audio, reproductor multimedia y asistente de voz quedan fuera de esta v1 para mantener una base segura.

## Uso

1. Copia `stick-s3.yaml` en ESPHome Builder.
2. Define estos secretos en ESPHome:

```yaml
wifi_ssid: "..."
wifi_password: "..."
fallback_ap_password: "..."
api_encryption_key: "..."
ota_password: "..."
```

## Nota sobre IR

La documentacion oficial de M5Stack indica que la recepcion IR debe usar RMT y que, cuando el amplificador de altavoz este activo, puede ser necesario apagarlo para recibir IR correctamente. La v1 no activa audio, asi que no deberia interferir.
