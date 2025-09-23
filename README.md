# LED Publisher (minimal)

Servicio Python mínimo que se suscribe a lecturas de sensores (PM2.5, temperatura, humedad y CO), deriva un color y lo publica a `SAD/led` cada 1 segundo.

## Qué publica
- Topic: `SAD/led`
- Payload: texto plano `green`, `yellow` o `red`

## Configuración (.env)
Ejemplo de `.env` (única variable necesaria):

```
MQTT_BROKER=200.13.4.202
```

## Ejecutar localmente

```bash
cd python
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python app.py
```

## Docker

```bash
docker compose build
docker compose up -d
```

Ver logs:

```bash
docker compose logs -f env-sensors-led
```

Detener:

```bash
docker compose down
```

### Docker directo

```bash
docker build -t env-sensors-led .
docker run --rm -d --name env-sensors-led \
  --env-file .env \
  --network host \
  env-sensors-led
```

## Notas
- Requiere acceso al broker MQTT (`MQTT_BROKER`/`MQTT_PORT`).
- Para más detalle en logs: `LOG_LEVEL=DEBUG` en `.env`.

## Reglas de color (resumen)
- PM2.5: `green` ≤ 15, `yellow` ≤ 25, `red` > 25 µg/m³
- Temperatura: `green` 20–27°C; `yellow` <20 o 27–32°C; `red` >32°C
- Humedad: `green` 40–70%; `yellow` 30–40% o 70–90%; `red` <30% o >90%
- CO: `green` ≤ 9 ppm; `yellow` ≤ 25 ppm; `red` > 25 ppm
- Color final: el peor caso entre los parámetros disponibles.
