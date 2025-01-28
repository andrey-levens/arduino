#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Мак адрес
byte ip[] = { 192, 168, 6, 6 }; // IP адрес
EthernetServer server(80);

int relayPin = 2;                // Пин для реле
bool relayState = LOW;           // Начальное состояние реле (выключено)
unsigned long previousMillis = 0;  // Переменная для хранения времени
const long relayOnTime = 500;    // Время работы реле в миллисекундах

void setup() {
  pinMode(relayPin, OUTPUT);           // Настраиваем пин реле как выход
  digitalWrite(relayPin, HIGH);        // По умолчанию реле выключено (HIGH, если реле активное по низкому уровню)

  Serial.begin(9600);
  Ethernet.begin(mac, ip);             // Инициализация Ethernet
  server.begin();                      // Запуск веб-сервера
}

void loop() {
  EthernetClient client = server.available();  // Проверяем наличие клиента
  if (client) {
    boolean currentLineIsBlank = true;
    String request = "";

    // Чтение данных от клиента
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c; // Читаем запрос
        if (c == '\n' && currentLineIsBlank) {
          // Проверка на наличие команды для переключения реле
          if (request.indexOf("GET /toggle") != -1) {
            relayState = LOW;                // Включаем реле (LOW, если реле активное по низкому уровню)
            digitalWrite(relayPin, relayState);
            previousMillis = millis();       // Запоминаем текущее время
          }

          // Отправляем HTML-страницу
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Relay Control</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<h3>Love IT door</h3>");
          client.print("<p>Status Relay: ");
          client.print((relayState == LOW) ? "ON" : "OFF");
          client.println("</p>");
          client.println("<form action=\"/toggle\" method=\"GET\">");
          client.println("<input type='submit' value='OPEN' style='width: 200px; height: 100px; font-size: 24px;'>");
          client.println("</form>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    // Отключаем клиента после отправки ответа
    delay(1);
    client.stop();
  }

  // Проверяем, прошло ли заданное время для выключения реле
  if (relayState == LOW && (millis() - previousMillis >= relayOnTime)) {
    relayState = HIGH;              // Выключаем реле
    digitalWrite(relayPin, relayState);
  }
}
