# Configuração do Projeto ESP32-MQTT

### Passos Obrigatórios

1.  **Clone o repositório.**

2.  **Abra a pasta do projeto no VS Code.**

3.  **Abra o "SDK Configuration editor"** (ícone de engrenagem ⚙️ na barra de status).

4.  **Configure os seguintes itens no menu:**
    * **Wi-Fi:** Navegue até `Example Connection Configuration` e preencha `WiFi SSID` e `WiFi Password`.
    * **Broker:** Navegue até `Component config` -> `MQTT` e preencha o campo `Broker URL` com o IP da sua máquina (ex: `mqtt://192.168.1.10`).

5.  **Salve as configurações.**

6.  **Compile (Build 🏗️), Grave (Flash ⚡️) e Monitore (Monitor 🔌)** o projeto usando os botões na barra de status.