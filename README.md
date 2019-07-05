# EspNow-MQQT

### Proposta do projeto:
Controlando uma LAN via Esp-Now: Esp Now é um protocolo de comunicação m2m (machine to machine) usando a rede WiFi, porém sem usar o roteador e a pilha TCP/IP, sendo por isto extremamente rápido. Um esp8266 Master controlaria um esp8266 Slave, para recolher temperatura, umidade e luminosidade e enviá-las ao broker MQTT.

Com isso desenvolvemos o código desse repositório.

### Funcionamento:
O Master estabelece uma conexão com o Slave, na resposta do Slave são mandados também os dados que o Master deseja ( temperatura, umidade e luminosidade). Após receber os dados o Master desconecta do Esp Now, e se conecta ao with para poder mandar os dados a um servidor MQQT publico (utilizamos o iot.eclipse.org), o topico criado para publicar os dados é "mc857/topic". Após salvar os dados no MQTT, o reinicia-se a place ESP8266 para começar  de novo a pedir e registrar dados.

Já o Slave fica no modo Esp Now a todo o momento esperando o Master pedir conexão. Utilizando um sensor DHT11 para registar os dados de temperaduta e humidade e um LDR pra registrar a luminosidade ele registra a cada conexão os dados atuais para enviá-los ao Master.

O sensor DHT11 foi colocado na porta D2.  
O sensor LDR foi colocado na porta A0.

### Falhas:
Após algumas leituras o Slave registra um estouro de memoria e reinia o ESP8266. Sabemos que o peblema é na leitura do DHT, já que se comentarmos as linhas que registram os novos dados a cada conexão (Linhas 139 e 141 do Slave.ino), o erro não ocorre.
