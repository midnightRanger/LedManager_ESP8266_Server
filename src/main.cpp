#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
//#include <ESP8266WebServer.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define led_d1 D1
#define led_d2 D2
#define led_d3 D3
#define led_d4 D4

#define eeprom_delay 10000

int val = 0;
int ledState = 1;

long eeprom_timer = 0;

String ledState_str = "1";
const char *ssid = "Lev";
const char *password = "12345678";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Менеджер освещения:)</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Lighting Manager</h2>
  %BUTTONPLACEHOLDER%

<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?state_led=0", true); }
  else { xhr.open("GET", "/update?state_led=1", true); }
  xhr.send();
}
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if(this.responseText == 0){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
      document.getElementById("output").checked = inputChecked;
      document.getElementById("outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 1000 ) ;
</script>
</body>
</html>
)rawliteral";

String outputState(){
  if(digitalRead(LED_BUILTIN) == LOW){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

String processor(const String &var)
{
  // Serial.println(var);
  if (var == "BUTTONPLACEHOLDER")
  {
    String buttons = "";

    String outputStateValue = outputState();

     // buttons += "<h4>LED <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
       
    for(int i = 0; i < 6; i++) {
      switch(i) {
        case 1: { buttons += "<h4>LED <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";} break;
        case 2: { buttons += "<h4>D1 <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox0(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";} break;
        case 3: { buttons += "<h4>D2 <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox1(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";} break;
        case 4: { buttons += "<h4>D3 <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox2(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";} break;
        case 5: { buttons += "<h4>D4 <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox3(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";} break;
      }
      }
    return buttons;
  }
  return String();
}

String getStringValue() {

  if(digitalRead(LED_BUILTIN)==HIGH) {
    return "1";
  }
  else return "0";
}

void save_eeprom(byte state, uint8_t led_num) {
    EEPROM.put(led_num, state);
}

void check_eeprom(){
  if(millis() - eeprom_timer > eeprom_delay) {
    eeprom_timer = millis();
    Serial.println("Значения EEPROM сохранены! ");
    save_eeprom(digitalRead(LED_BUILTIN), 1);
    Serial.print("Данные: ");
    Serial.println(EEPROM.read(1));
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(led_d1, OUTPUT);
  pinMode(led_d2, OUTPUT);
  pinMode(led_d3, OUTPUT);
  pinMode(led_d4, OUTPUT);

  digitalWrite(led_d1, LOW); //У меня нет MFC...
  digitalWrite(led_d2, LOW);
  digitalWrite(led_d3, LOW);
  digitalWrite(led_d4, LOW);

  delay(1000);                 
  WiFi.softAP(ssid, password); // Создаём точку WiFi
  // Указываем по каким роутам какие методы запускать
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });

  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // получаем значение input1 <ESP_IP>/state?led=0
    if (request->hasParam("state_led")) {
      inputMessage = request->getParam("state_led")->value();
      inputParam = "state_led";
      digitalWrite(LED_BUILTIN, inputMessage.toInt());
      ledState = !ledState;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
 
  // Отправляем запрос GET на <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getStringValue());
  });

  
  // Запускаем сервер
  server.begin();
  Serial.println(getStringValue());
}
void loop()
{
  check_eeprom();
  digitalWrite(LED_BUILTIN, !ledState);
}
