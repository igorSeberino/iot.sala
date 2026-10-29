// Configuração de exemplo. NÃO coloque senha real aqui.
// Para usar: copie este arquivo para config.h (na mesma pasta) e preencha a rede Wi-Fi.
// O config.h está no .gitignore e nunca vai para o repositório.

#define WIFI_SSID   "NOME_DA_REDE"
#define WIFI_SENHA  "SENHA_DA_REDE"

// Broker público (README §2). Alternativa: "test.mosquitto.org", mesma porta.
#define MQTT_HOST   "broker.hivemq.com"
#define MQTT_PORTA  1883

// Prefixo de todos os tópicos (README §3).
#define TOPICO_BASE "catolica/iot/n3-raig/sala/"
