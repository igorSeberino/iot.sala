# Monitor de Temperatura da Sala

N3 de Internet das Coisas · Engenharia de Software, Católica SC.
Equipe: Renato Colin Neto · Adrian Cesar Gonçalves · Gabriel Carvalho · Igor Thiago Seberino.

Plano e requisitos: [`docs/plano/00-visao-geral.md`](docs/plano/00-visao-geral.md) ·
tarefas: [Project @sala](https://github.com/users/igorSeberino/projects/3) ·
registro de IA: [`docs/registro-ia.md`](docs/registro-ia.md)

## 1. Uso escolhido

Um ESP32 mede a temperatura de uma sala a cada 5 segundos e publica a leitura por MQTT. O
dashboard mostra o valor em tempo real e exibe o aviso **"ACIMA DO LIMITE"** quando a
temperatura passa de **28 °C**. Pelo dashboard, a pessoa liga ou desliga um LED que representa o
ventilador da sala, e a página mostra o estado em texto: **LIGADO** ou **DESLIGADO**.

Escolhemos esse uso porque é simples de montar, só usa o que vimos nas aulas e cobre os quatro
critérios da nota: caminho completo (leitura indo e comando voltando pelo broker), dispositivo
(lê, aciona e reconecta sozinho), dashboard (leitura e estado em texto, botão de comando) e
documentação.

## 2. Componentes

| Parte | O que usamos | Observação |
|---|---|---|
| Dispositivo | ESP32 DevKit | Programado na Arduino IDE, biblioteca MQTT `PubSubClient` (Nick O'Leary) |
| Sensor | DHT11 solto (4 pinos), pino de dados no **GPIO 4**, com resistor de 10 kΩ de pull-up | Biblioteca `DHT sensor library` (Adafruit) |
| Atuador | LED + resistor de 220 Ω no **GPIO 2** | Representa o ventilador |
| Broker | HiveMQ público: `broker.hivemq.com` | Porta 1883 (ESP32) e 8884 com WebSocket seguro, caminho `/mqtt` (navegador) |
| Dashboard | Uma página HTML + JavaScript com a biblioteca MQTT.js | Abre direto no navegador, sem servidor próprio |

**Por que esse broker:** é público e gratuito, então ninguém precisa instalar nada; aceita ao
mesmo tempo TCP (ESP32) e WebSocket seguro (navegador), e qualquer pessoa da equipe testa de
casa. Em 28/10 testamos o `test.mosquitto.org` (o Mosquitto das aulas, hospedado) e ele não
respondeu nem na 1883 nem na 8081; o próprio site avisa que WebSocket e TLS são o que mais fica
fora do ar. O HiveMQ passou no mesmo teste: leitura, comando, retain e Last Will. Se o HiveMQ
cair, o Mosquitto volta a ser opção trocando o endereço no ESP32 e o endereço e a porta (8081) no
dashboard. Como o broker é
público, usamos um prefixo de tópico próprio da equipe e não enviamos nenhum dado sensível.

## 3. Tópicos MQTT

Prefixo de todos os tópicos: `catolica/iot/n3-raig/sala/` (iniciais da equipe, para não colidir
com outros grupos no broker público).

| Tópico | Quem publica → quem assina | Mensagem | QoS / retain |
|---|---|---|---|
| `…/sala/temperatura` | ESP32 → Dashboard | número em °C com 1 casa decimal, a cada 5 s: `26.4` | 0 / não |
| `…/sala/ventilador/comando` | Dashboard → ESP32 | `ON` ou `OFF` | 1 / não |
| `…/sala/ventilador/estado` | ESP32 → Dashboard | `LIGADO` ou `DESLIGADO` | 0 / sim |
| `…/sala/status` | ESP32 (e o broker, pelo Last Will) → Dashboard | `online` ou `offline` | `online`: 0 / sim · `offline` (Last Will): 1 / sim |

- **Comando e estado são separados.** O dashboard não "chuta" que o LED ligou: ele só mostra
  LIGADO quando o próprio ESP32 confirma no tópico de estado, e o ESP32 só confirma depois de
  mudar o pino.
- **Comando sem retain.** Um comando antigo não pode ser executado de novo quando o ESP32
  reconecta.
- **Retain no estado e no status.** Quem abre o dashboard depois já recebe o último estado, sem
  esperar a próxima mudança.
- **Last Will (testamento) em `status`.** Ao conectar, o ESP32 registra no broker a mensagem
  `offline`. Se o Wi-Fi ou a energia do ESP32 cair, o broker percebe pelo keep alive
  (15 s × 1,5 ≈ 22 s) e publica `offline` sozinho; o dashboard mostra "Dispositivo
  desconectado".
- **QoS do ESP32.** A `PubSubClient` só publica com QoS 0 (assina com QoS 0 ou 1). Por isso o
  ESP32 publica com QoS 0 e assina o comando com QoS 1; o Last Will vai com QoS 1, porque quem
  publica é o broker.

## 4. Diagrama do sistema

![Diagrama: ESP32, broker e dashboard, com os tópicos e o sentido de cada mensagem](docs/diagrama.svg)

## 5. Ligação do circuito

| De | Para | Observação |
|---|---|---|
| DHT11 pino 1 (VCC) | ESP32 3V3 | Não usar 5 V: o pull-up levaria 5 V ao GPIO, e o ESP32 é 3,3 V |
| DHT11 pino 2 (DATA) | ESP32 GPIO 4 | Resistor de 10 kΩ entre o DATA e o 3V3 (pull-up). O módulo de 3 pinos, numa plaquinha, já vem com ele |
| DHT11 pino 3 | — | Não é ligado |
| DHT11 pino 4 (GND) | ESP32 GND | |
| ESP32 GPIO 2 | Resistor 220 Ω → LED (+, perna longa) | GPIO 2 também acende o LED azul da placa |
| LED (−, perna curta) | ESP32 GND | |

Pinos do DHT11 solto contados com a **grade virada para você e as pernas para baixo**, da
esquerda para a direita. Virado ao contrário, o 3V3 vai para o GND do sensor.

**Montagem usada pela equipe** (protoboard de 830 furos; o ESP32 ocupa as colunas a e i das
fileiras 1 a 15, com a antena para a fileira 1; os fios saem da coluna j, que fica livre):

| Peça | Furos |
|---|---|
| DHT11 (costas lisas para o lado dos fios) | f31 (VCC), f32 (DATA), f33, f34 (GND) |
| Resistor 10 kΩ (pull-up) | h31 e h32 |
| Fio 3V3 / D4 / GND | j15 → j31 · j11 → j32 · j14 → j34 |
| Ponte de GND para o LED | i34 → i38 |
| LED | perna curta h38, perna longa h39 |
| Resistor do LED | i39 e i42 |
| Fio D2 | j12 → j42 |

O GPIO 2 é pino de boot (strapping): precisa estar em nível baixo ou solto na energização. Ele
já tem pull-down interno, e o LED com resistor para o GND não o puxa para alto, então a placa
liga e grava normalmente.

## 6. Caminho de cada mensagem

| Fluxo | Passo a passo |
|---|---|
| Leitura | DHT11 mede → ESP32 publica em `temperatura` → broker repassa → dashboard atualiza o número e mostra o aviso se passar de 28 °C. |
| Comando | Usuário clica em "Ligar" → dashboard publica `ON` em `ventilador/comando` → broker repassa → ESP32 acende o LED → ESP32 publica `LIGADO` em `ventilador/estado` → dashboard mostra "Ventilador: LIGADO". |
| Queda do ESP32 (Wi-Fi ou energia) | O broker para de receber o keep alive → publica `offline` em `status` (Last Will) → dashboard mostra "Dispositivo desconectado". O ESP32 tenta reconectar a cada 5 s e, ao voltar, publica `online` e o estado atual do LED e assina de novo o tópico de comando. |
| Queda do broker | O ESP32 e o dashboard perdem a conexão. O dashboard mostra "Broker: desconectado" (o Last Will não chega, porque quem publica é o broker). Os dois tentam reconectar sozinhos e, quando o broker volta, o ESP32 republica `online` e o estado. |

## 7. Equipe

| Integrante | GitHub | Responsabilidade principal |
|---|---|---|
| Renato Colin Neto | [@rcolinneto](https://github.com/rcolinneto) | Dashboard (HTML + MQTT.js): leitura, aviso de limite, estado em texto e botões |
| Adrian Cesar Gonçalves | [@adrian-cesar](https://github.com/adrian-cesar) | Montagem do circuito e leitura do DHT11 no ESP32 |
| Gabriel Carvalho | [@gabrielcarvallho](https://github.com/gabrielcarvallho) | Código MQTT do ESP32: publicação, comando do LED, reconexão e Last Will |
| Igor Thiago Seberino | [@igorSeberino](https://github.com/igorSeberino) | Diagrama, passo a passo, configuração de exemplo, testes de queda, vídeo e registro de IA |

Todos revisam o código inteiro antes da apresentação, por causa das perguntas do professor.

## 8. Etapa de 4/11: gravar o ESP32 e conferir no cliente MQTT

Firmware: [`firmware/monitor_sala/monitor_sala.ino`](firmware/monitor_sala/monitor_sala.ino)
(issues #2 e #3). Ele lê o DHT11 a cada 5 s e publica em `temperatura` e `status`; o comando do
LED entra na etapa de 11/11 (#4).

**Arduino IDE (uma vez)**

1. *Gerenciador de Placas*: instalar **esp32** da Espressif e escolher a placa **ESP32 Dev
   Module**.
2. *Gerenciador de Bibliotecas*: instalar **PubSubClient** (Nick O'Leary) e **DHT sensor
   library** (Adafruit), aceitando a dependência **Adafruit Unified Sensor**.

**Gravar**

3. Em `firmware/monitor_sala/`, copiar `config.example.h` para `config.h` e preencher
   `WIFI_SSID` e `WIFI_SENHA`. A rede precisa ser de **2,4 GHz** (o ESP32 não conecta em 5 GHz).
4. Abrir `monitor_sala.ino`, gravar e abrir o Monitor Serial em **115200**. O esperado:

```
Monitor de Temperatura da Sala
Conectando ao Wi-Fi MinhaRede...
Wi-Fi conectado. IP: 192.168.0.23
Conectando ao broker broker.hivemq.com como n3-raig-sala-A1B2C3D4E5F6
Broker conectado.
Temperatura: 26.4 °C (publicada)
```

Se aparecer `Falha ao conectar no broker (estado -2)`, a rede está bloqueando a porta 1883
(comum em rede de faculdade): usar o hotspot do celular.

Problemas que já tivemos:

- **Rede da faculdade (CATOLICASC):** é aberta com página de login, e o ESP32 não faz esse
  login. Usar o hotspot do celular em 2,4 GHz (no iPhone: *Acesso Pessoal → Maximizar
  compatibilidade*, com a tela aberta enquanto o ESP32 conecta).
- **`Falha ao ler o DHT11`:** as pernas do sensor solto são finas e curtas. Empurrar o sensor até
  o fim e conferir que cada perna está na mesma fileira do fio correspondente.
- **Monitor Serial mudo e gravação falhando com `No serial data received`:** apertar o botão
  **EN** da placa (reinicia) e tentar de novo.

**Conferir no MQTT Explorer**

5. Nova conexão: protocolo `mqtt://`, host `broker.hivemq.com`, porta `1883`, sem TLS e sem
   usuário. Em **Advanced**, apagar `#` e `$SYS/#` e assinar só `catolica/iot/n3-raig/sala/#`
   (o broker é público: com `#` chega mensagem do mundo inteiro). Se o PC estiver numa rede que
   bloqueia a 1883: protocolo `ws://` com **Encryption (tls)** ligado, porta `8884`, basepath
   `mqtt`.
6. Conectar e conferir:

| O que fazer | O que aparece | Issue |
|---|---|---|
| Ligar o ESP32 | `status` = `online` (retido); `temperatura` chega a cada 5 s com 1 casa decimal, ex.: `26.4` | #3 |
| Segurar o DHT11 entre os dedos | `temperatura` sobe | #2 |
| Soltar o fio DATA | o Serial mostra a falha e `temperatura` para de chegar (leitura `NAN` não é publicada) | #2, #3 |
| Tirar o ESP32 da energia | `status` = `offline` em até 30 s (Last Will, publicado pelo broker) | #3 |
| Desligar o hotspot por 30 s e religar | o Serial mostra `Wi-Fi caiu.` e uma tentativa a cada 5 s; volta a publicar sozinho, sem reset | #3 |

7. Um print do MQTT Explorer com `status` e `temperatura` chegando é a evidência da etapa: vai
   num comentário das issues #2 e #3.

**Resultado em 04/11** (ESP32 no hotspot, MQTT Explorer no PC): `status = online` retido e a
temperatura chegando a cada 5 s; segurando o sensor, ela subiu de 23,0 para 26,7 °C.

![MQTT Explorer com status online e temperatura 23.0](docs/evidencias/etapa2-mqtt-explorer-online.png)

![MQTT Explorer com a temperatura subindo de 23 para 26,7 °C ao segurar o sensor](docs/evidencias/etapa2-mqtt-explorer-temperatura-subindo.png)
