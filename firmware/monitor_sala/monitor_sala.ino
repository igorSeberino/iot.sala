// Monitor de Temperatura da Sala · N3 de Internet das Coisas
//
// Etapa de 4/11 (issues #2 e #3): o ESP32 lê o DHT11 a cada 5 s e publica a temperatura no
// broker MQTT, com status online/offline (Last Will) e reconexão sozinha do Wi-Fi e do broker.
// O comando do LED (ventilador) entra na etapa de 11/11 (issue #4).
//
// Tópicos, QoS e retain: README §3. Ligação dos pinos: README §5.
//
// Antes de gravar: copie config.example.h para config.h (nesta pasta) e preencha o Wi-Fi.

#include <WiFi.h>
#include <PubSubClient.h>  // MQTT, de Nick O'Leary
#include <DHT.h>           // DHT sensor library, da Adafruit
#include "config.h"        // Wi-Fi, broker e prefixo dos tópicos (fora do git)

// ---------- Sensor (README §5) ----------
#define PINO_DHT 4
#define TIPO_DHT DHT11

// ---------- Tópicos (README §3) ----------
// TOPICO_BASE vem do config.h; o compilador junta os dois textos num só.
#define TOPICO_TEMPERATURA TOPICO_BASE "temperatura"
#define TOPICO_STATUS      TOPICO_BASE "status"

// ---------- Tempos ----------
const unsigned long INTERVALO_LEITURA   = 5000;  // ms entre uma leitura e outra
const unsigned long INTERVALO_RECONEXAO = 5000;  // ms entre tentativas de Wi-Fi e de broker
const uint16_t      KEEP_ALIVE_S        = 15;    // o broker percebe a queda em ~22 s (15 × 1,5)

DHT dht(PINO_DHT, TIPO_DHT);
WiFiClient wifiClient;          // conexão TCP que o MQTT usa por baixo
PubSubClient mqtt(wifiClient);

// Guardam o millis() da última vez que cada coisa aconteceu. Comparar com millis() no loop()
// substitui o delay(): o loop() nunca para, então o MQTT continua respondendo enquanto espera.
unsigned long ultimaLeitura       = 0;
unsigned long ultimaTentativaWifi = 0;
unsigned long ultimaTentativaMqtt = 0;

// Só servem para escrever no Serial quando a conexão cai ou volta (uma vez, não a cada volta).
bool wifiEstavaConectado = false;
bool mqttEstavaConectado = false;

// Lê o DHT11. Devolve a temperatura em °C ou NAN se o sensor não responder.
float lerTemperatura() {
  float t = dht.readTemperature();  // a biblioteca já devolve NAN quando a leitura falha
  if (isnan(t)) {
    Serial.println("Falha ao ler o DHT11: confira o fio DATA no GPIO 4 e o 3V3.");
  }
  return t;
}

// Devolve true se o Wi-Fi está conectado. Se não está, pede uma nova tentativa a cada 5 s.
// WiFi.reconnect() só dispara a tentativa e retorna na hora, sem travar o loop().
bool manterWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiEstavaConectado) {
      Serial.print("Wi-Fi conectado. IP: ");
      Serial.println(WiFi.localIP());
      wifiEstavaConectado = true;
    }
    return true;
  }

  if (wifiEstavaConectado) {
    Serial.println("Wi-Fi caiu.");
    wifiEstavaConectado = false;
  }

  if (millis() - ultimaTentativaWifi >= INTERVALO_RECONEXAO) {
    ultimaTentativaWifi = millis();
    Serial.println("Tentando conectar ao Wi-Fi " WIFI_SSID "...");
    WiFi.disconnect();
    WiFi.reconnect();
  }
  return false;
}

// Client ID único no broker público: prefixo da equipe + MAC da placa, sem os dois-pontos.
// Dois clientes com o mesmo ID derrubam um ao outro no broker.
String idDoCliente() {
  String id = "n3-raig-sala-" + WiFi.macAddress();
  id.replace(":", "");
  return id;
}

// Conecta ao broker já registrando o Last Will (testamento): se o ESP32 sumir sem se
// despedir, o próprio broker publica "offline" em status, com QoS 1 e retido.
// O mqtt.connect() espera a resposta: com o broker fora do ar, cada tentativa segura o loop()
// por alguns segundos (até 3 s da conexão TCP e até 5 s do setSocketTimeout), e depois o
// loop() segue normal até a próxima tentativa.
void conectarMqtt() {
  String id = idDoCliente();
  Serial.print("Conectando ao broker " MQTT_HOST " como ");
  Serial.println(id);

  //                       ID          tópico do Last Will  QoS  retido  mensagem
  bool conectou = mqtt.connect(id.c_str(), TOPICO_STATUS,      1,   true,   "offline");

  if (conectou) {
    Serial.println("Broker conectado.");
    // Retido: quem abrir o dashboard ou o MQTT Explorer depois já recebe "online".
    mqtt.publish(TOPICO_STATUS, "online", true);
  } else {
    // mqtt.state(): -2 = não abriu a conexão TCP (broker fora do ar ou porta bloqueada),
    // -4 = o broker não respondeu a tempo. Lista completa no PubSubClient.h.
    Serial.print("Falha ao conectar no broker (estado ");
    Serial.print(mqtt.state());
    Serial.println("). Nova tentativa em 5 s.");
  }
}

// Mantém a conexão com o broker: se está conectado, roda o mqtt.loop(), que manda o keep
// alive e recebe mensagens; se caiu, tenta de novo a cada 5 s.
void manterMqtt() {
  if (mqtt.connected()) {
    mqttEstavaConectado = true;
    mqtt.loop();
    return;
  }

  if (mqttEstavaConectado) {
    Serial.println("Conexão com o broker caiu.");
    mqttEstavaConectado = false;
  }

  if (millis() - ultimaTentativaMqtt >= INTERVALO_RECONEXAO) {
    ultimaTentativaMqtt = millis();
    conectarMqtt();
  }
}

// Lê o sensor e, se a leitura deu certo e o broker está conectado, publica com 1 casa decimal.
void lerEPublicar() {
  float t = lerTemperatura();
  if (isnan(t)) {
    return;  // leitura com falha não é publicada
  }

  String valor = String(t, 1);  // número puro com 1 casa decimal, ex.: "26.4"
  Serial.print("Temperatura: ");
  Serial.print(valor);
  Serial.print(" °C");

  // Confere o Wi-Fi também: logo depois de o Wi-Fi cair, o mqtt.connected() ainda pode dizer
  // que sim, porque a conexão TCP só é dada como morta quando o keep alive falha.
  bool podePublicar = WiFi.status() == WL_CONNECTED && mqtt.connected();
  if (podePublicar && mqtt.publish(TOPICO_TEMPERATURA, valor.c_str())) {  // QoS 0, sem retain
    Serial.println(" (publicada)");
  } else {
    Serial.println(" (sem broker, não publicada)");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Monitor de Temperatura da Sala");

  dht.begin();

  WiFi.mode(WIFI_STA);  // só cliente de uma rede, sem criar ponto de acesso
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  ultimaTentativaWifi = millis();
  Serial.println("Conectando ao Wi-Fi " WIFI_SSID "...");

  mqtt.setServer(MQTT_HOST, MQTT_PORTA);
  mqtt.setKeepAlive(KEEP_ALIVE_S);
  mqtt.setSocketTimeout(5);  // espera no máximo 5 s pela resposta do broker ao conectar
}

void loop() {
  if (manterWifi()) {  // sem Wi-Fi não adianta tentar o broker
    manterMqtt();
  }

  if (millis() - ultimaLeitura >= INTERVALO_LEITURA) {
    ultimaLeitura = millis();
    lerEPublicar();
  }
}
