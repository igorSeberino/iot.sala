# iot.sala · Plano

N3 de Internet das Coisas. Repositório: https://github.com/igorSeberino/iot.sala ·
Project: https://github.com/users/igorSeberino/projects/3

O que o sistema é e como funciona (uso, componentes, tópicos, diagrama, circuito) mora no
[README](../../README.md). Este arquivo guarda só requisitos, decisões e tarefas. Toda issue
aponta para um R ou um D daqui.

## 1. Fontes

| Fonte | O que fixa |
|---|---|
| Escopo da N3 passado pelo professor (texto colado no Teams) | R1–R22, etapas por semana, entrega até 25/11, rubrica |
| Documento "N3 – Etapa 1: uso, tópicos MQTT e diagrama" (PDF da equipe, 4 páginas) | uso, componentes, tópicos, divisão da equipe (D2, D9); corrigido no README |
| Material de aula (resumo da revisão, assuntos 1 a 7) | padrão comando/estado/status, retain, Last Will com keep alive de 15 s × 1,5, pinos de boot do ESP32 |

## 2. Requisitos

| R | Trecho literal do escopo (seção) | Onde é atendido |
|---|---|---|
| R1 | "Um dispositivo, o ESP32, programado por vocês, com pelo menos um sensor ou fonte de dados e um atuador" (O que vocês vão construir) | #2, #3, #4 |
| R2 | "Um broker MQTT [...] desde que vocês digam qual escolheram e por quê" (O que vocês vão construir) | README §2 (D3) |
| R3 | "Um dashboard, a página que mostra o que o sensor está medindo e o estado do atuador, e que tem um botão para mandar um comando ao dispositivo" (O que vocês vão construir) | #5 |
| R4 | "O dado precisa fazer o caminho completo nos dois sentidos" (O que vocês vão construir) | #3, #4, #5, #6 |
| R5 | "o dashboard mostra o novo estado com texto, e não só com cor" (O que vocês vão construir) | #5 |
| R6 | "Escolham um uso concreto para o sistema" (O que vocês vão construir) | README §1 (#1) |
| R7 | "O uso de inteligência artificial é permitido e precisa ser registrado: ferramenta, para que foi usada, o pedido feito, o que vocês aceitaram, o que rejeitaram e o que mudaram depois" (O que vocês vão construir) | `docs/registro-ia.md`, critério em todas as issues |
| R8 | "Semana de 28/10: escolher o uso, definir os tópicos MQTT e desenhar o diagrama com as três partes" (As etapas) | #1 |
| R9 | "Semana de 4/11: dispositivo lendo o sensor e publicando no broker, conferido num cliente MQTT" (As etapas) | #2, #3 |
| R10 | "Semana de 11/11: dashboard mostrando a leitura e enviando o comando ao atuador" (As etapas) | #4, #5 |
| R11 | "Semana de 18/11: testar o sistema inteiro, inclusive o que acontece quando o Wi-Fi ou o broker cai, e escrever a documentação" (As etapas) | #6, #7 |
| R12 | "Código do dispositivo e do dashboard, num repositório, com o link na entrega" (O que entregar) | este repositório (D1), #8 |
| R13 | "Diagrama com o dispositivo, o broker e o dashboard, mostrando os tópicos e para onde vai cada mensagem" (O que entregar) | `docs/diagrama.svg` (#1) |
| R14 | "Arquivo de configuração de exemplo, sem nenhuma senha ou chave real" (O que entregar) | `firmware/monitor_sala/config.example.h` (D8, #1, #3) |
| R15 | "Um passo a passo curto para outra pessoa montar e ligar o sistema" (O que entregar) | #7 |
| R16 | "Um vídeo curto do sistema funcionando: a leitura chegando ao dashboard e o comando mudando o atuador" (O que entregar) | #7 |
| R17 | "Um envio por equipe, nesta tarefa do Teams, até 25/11, com os arquivos em PDF, o vídeo e o link do repositório. Escrevam o nome de todos os integrantes e o que cada um fez. Depois de enviar, confiram se a entrega aparece na tarefa." (Como e onde entregar) | #8 (D10) |
| R18 | "Cada equipe mostra o sistema funcionando ao vivo" (Como e onde entregar) | #9 |
| R19 | Critério "Caminho completo: a leitura chega ao dashboard e o comando chega ao atuador, passando pelo broker" (2,5) (Como a nota é dada) | #6 |
| R20 | Critério "Dispositivo: o ESP32 lê o sensor, aciona o atuador e volta a se conectar sozinho quando o Wi-Fi ou o broker cai" (2,5) (Como a nota é dada) | #2, #3, #4, #6 |
| R21 | Critério "Dashboard: mostra a leitura e o estado do atuador com texto, e manda o comando" (2,5) (Como a nota é dada) | #5 |
| R22 | "Cada pergunta não respondida desconta 2 pontos da nota [...] todos precisam entender o código inteiro, e não só a própria parte" (Como a nota é dada) | #9 |

O critério "Documentação" da rubrica é a soma de R7, R13–R16. Não são requisitos: "Tudo o que
vocês precisam já foi visto nas aulas" (orientação, virou D4/D5) e "Um ritmo que cabe no prazo"
(as semanas são sugestão; o único prazo fixo é 25/11, mas o plano segue as semanas).

## 3. Decisões

| D | Decisão | Quem | Motivo | Descartado |
|---|---|---|---|---|
| D1 | Repositório público `igorSeberino/iot.sala` + Project `@sala`; commits direto na `main`, cada um na sua pasta, sem PR obrigatório | Igor (sem trunk-based) / agente (nome) | O link vai na entrega e o professor precisa abrir; as partes ficam em pastas separadas (`firmware/`, `dashboard/`, `docs/`) | Fluxo trunk-based com PR e revisão do data.vigia |
| D2 | Uso: monitor de temperatura da sala, aviso acima de 28 °C, LED como ventilador | equipe (documento da etapa 1) | Simples e cobre os quatro critérios | — |
| D3 | Broker `broker.hivemq.com` (1883 TCP no ESP32, 8884 WSS `/mqtt` no navegador); `test.mosquitto.org` fica como troca só de configuração | agente | Em 28/10 o `test.mosquitto.org` falhou daqui em todas as tentativas (1883: `ETIMEDOUT`; 8081: `ECONNRESET`) e o próprio site diz que WebSocket e TLS são o que mais cai; o HiveMQ passou no teste completo (§5) | `test.mosquitto.org` como principal (escolha do documento); Mosquitto local (exige PC ligado e na mesma rede na apresentação) |
| D4 | ESP32 com `PubSubClient`: publica com QoS 0 e assina o comando com QoS 1; Last Will com QoS 1 e retain | agente | A `PubSubClient` só publica com QoS 0; o keep alive padrão de 15 s é o do material de aula. O documento pedia QoS 1 nas publicações do ESP32, o que essa biblioteca não faz | Outra biblioteca com publicação QoS 1 (mais uma dependência que nenhum R pede) |
| D5 | Temperatura publicada como número puro (`26.4`); umidade fora | agente | Padrão de telemetria do material de aula ("payload contendo apenas a informação pura") e o tópico se chama `temperatura`; nenhum R pede umidade | JSON `{"temp":26.4,"umid":61}` do documento |
| D6 | Dashboard mostra também, em texto, a conexão do próprio navegador com o broker | agente | Quando o broker cai, o Last Will não chega (quem publica é o broker) e o dashboard ficaria mostrando o último valor como se estivesse tudo bem; o teste de queda do broker (R11) precisa de algo para observar | Só o status do dispositivo |
| D7 | O ESP32 liga com o LED DESLIGADO e, a cada (re)conexão, publica `online` e o estado atual (retidos) e assina de novo o comando | agente | Sem isso o estado retido pode continuar LIGADO depois de um reinício com o LED apagado | — |
| D8 | `config.example.h` versionado; `config.h` real no `.gitignore` | agente | R14 | Senha no código |
| D9 | Responsáveis = divisão do documento da etapa 1 (README §7); a entrega no Teams fica com o Igor | equipe / agente (entrega) | O documento não diz quem envia; o Igor já cuida da documentação | — |
| D10 | Os arquivos em PDF da entrega são o README e o registro de IA exportados (o README tem o diagrama e o passo a passo); vídeo de até 2 min | agente | R17 pede PDF; uma fonte só para a documentação. "Curto" virou um limite que dá para conferir | Documento separado escrito à mão |

## 4. Fora do escopo

- Umidade, histórico e gráfico de leituras — nenhum R pede; o dashboard mostra o valor atual (D5).
- Ligar o ventilador sozinho quando passa de 28 °C — o uso é aviso + comando manual (D2).
- Relé ou ventilador de verdade — o LED cumpre R1 e o documento já o trata como representação.
- TLS, usuário e senha no ESP32 — broker público, sem dado sensível; nenhum R pede.
- Hospedar o dashboard (GitHub Pages, servidor) — a página abre direto do arquivo.
- CI, testes automatizados, PR obrigatório, proteção de branch — sem R e sem trunk-based (D1).
- Labels, templates de issue, campos do Project além de Status — nada filtra por eles.
- LICENSE, CONTRIBUTING, CHANGELOG — ninguém de fora reusa o código.
- Mais de uma sala ou dispositivo — o uso é uma sala.

## 5. Fatos verificados

| Fato | Valor | Origem |
|---|---|---|
| `test.mosquitto.org` em 28/10 | 1883 `ETIMEDOUT` (21 s), 8081 `ECONNRESET`, 3 rodadas | `mqtt.connect('mqtt://test.mosquitto.org:1883')` e `wss://…:8081` (MQTT.js 5.16.1, Node 24) |
| HiveMQ público, ciclo completo | TCP 1883 + WSS 8884 `/mqtt`: status retido chegou, leitura chegou, comando QoS 1 chegou, estado publicado, Last Will `offline` (QoS 1) chegou ao derrubar o socket | sonda Node com dois clientes, mesmo prefixo de tópico com sufixo `-probe` |
| `PubSubClient` | "It can only publish QoS 0 messages. It can subscribe at QoS 0 or QoS 1"; keep alive padrão 15 s | README de knolleary/pubsubclient |
| Keep alive | broker detecta queda após 15 s × 1,5 = 22,5 s | material de aula, assunto 4 |

## 6. Tarefas

Uma issue por tarefa (`#n` = número da issue). A semana vem do escopo.

| # | Entrega | Atende | Responsável | Semana | Bloqueada por |
|---|---|---|---|---|---|
| 1 | Etapa 1: uso, tópicos, diagrama, configuração de exemplo (README §1–7, `docs/diagrama.svg`) | R6, R8, R13, R14, D2–D8 | @igorSeberino | 28/10 | — |
| 2 | Circuito montado e ESP32 lendo o DHT11 a cada 5 s | R1, R9, R20 | @adrian-cesar | 04/11 | #1 |
| 3 | ESP32 conectando ao Wi-Fi e ao broker, publicando temperatura e status, com Last Will e reconexão | R1, R4, R9, R14, R20, D3, D4, D5, D7, D8 | @gabrielcarvallho | 04/11 | #2 |
| 4 | ESP32 recebendo o comando, mudando o LED e confirmando o estado | R1, R4, R10, R20, D4, D7 | @gabrielcarvallho | 11/11 | #3 |
| 5 | Dashboard com leitura, aviso, estados em texto e botões | R3, R4, R5, R10, R21, D6 | @rcolinneto | 11/11 | #1 |
| 6 | Teste do sistema inteiro, com queda do Wi-Fi, do broker e da energia | R4, R11, R19, R20 | @igorSeberino | 18/11 | #4, #5 |
| 7 | Passo a passo, registro de IA fechado e vídeo | R7, R11, R15, R16, D10 | @igorSeberino | 18/11 | #6 |
| 8 | Entrega no Teams | R12, R17, D9, D10 | @igorSeberino | até 25/11 | #7 |
| 9 | Todos entendem o código inteiro e o sistema roda ao vivo | R18, R22 | todos | antes da apresentação | #4, #5 |

`#1 → #2 → #3 → #4 → #6 → #7 → #8`; `#1 → #5 → #6`; `{#4, #5} → #9`.

Critério comum a toda issue: se alguém usou IA na tarefa, a linha está em `docs/registro-ia.md`
com as sete colunas preenchidas (R7).

## 7. Contratos entre tarefas

- Tópicos, mensagens, QoS e retain: README §3. Pinos: README §5. Broker e prefixo:
  `firmware/monitor_sala/config.example.h` e a constante equivalente no topo de
  `dashboard/index.html`.
- Sketch em `firmware/monitor_sala/monitor_sala.ino` (a Arduino IDE exige pasta e arquivo com o
  mesmo nome). O #2 expõe a leitura numa função que devolve a temperatura em °C ou `NAN` se o
  sensor falhar; o #3 a usa.
- Dashboard em um arquivo só, `dashboard/index.html`.

## 8. Pronto

- [ ] Os 9 itens do Project em Done.
- [ ] `git grep -n -i -E "senha|password" -- ':!*.example.h' ':!docs/'` não mostra senha real.
- [ ] A entrega aparece na tarefa do Teams com os PDFs, o vídeo e o link deste repositório.
