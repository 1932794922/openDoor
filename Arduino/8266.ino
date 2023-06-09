#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <string.h>


// M{"topic":"testtopic/1","code":30,"data":"New York"}Q
// M{"code":30,"data":"New York","message":"ok"}Q   格式


// const char *ssid = "Trister";             // 输入你的WiFi名称
const char *ssid = "XzYiPhone";  
const char *password = "77777778";    // 输入你的WiFi密码
const char *mqtt_broker = "cd.xx.cc";  //输入你的无线局域网适配器IPv4地址
const int mqtt_port = 1883;               //默认为1883

// MQTT服务端连接用户名密码
const char *mqttUserName = "admin";
const char *mqttPassword = "*****";

// 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
String clientId = "esp8266-" + WiFi.macAddress();

const char *DefaultTopic = "esp8266/sub/door";
const char *DefaultPubTopic = "esp8266/pub/door";
const char *sussce = "{\"message\":\"ok\",\"code\":200}";

//帧头
const char frameHead = 'M';
//帧尾
const char frameTail = 'Q';



WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // Initialize the LED_BUILTIN pin as an output
  // Set software serial baud to 115200;
  Serial.begin(9600);



    // 注册中断回调函数
  //attachInterrupt(digitalPinToInterrupt(2), serialEvent, CHANGE);

  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on (Note that LOW is the voltage level
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(1000);
  }
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(60);
  // 连接MQTT服务器。此处使用了程序首部定义的用户名和密码来实现MQTT服务端认证
  while (!client.connected()) {
    Serial.println("Connecting to public emqx mqtt broker.....");
    if (client.connect(clientId.c_str(), mqttUserName, mqttPassword)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  // publish and subscribe
  client.subscribe(DefaultTopic);
  client.publish(DefaultPubTopic, "ESP8266上线");
}

void callback(char *topic, byte *payload, unsigned int length) {
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on (Note that LOW is the voltage level

  Serial.println(topic);

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
   // Serial.print("callback  deserializeJson() failed: ");
    //Serial.println(error.c_str());
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    return;
  }

  String action = doc["action"].as<String>();  // 获取 action  要执行的动作
  if (action != "null") {
    // 执行订阅动作
    String subname = doc["subname"].as<String>();  // 获取将要订阅的主题名称
    if (action == "subscribe") {
      if (client.subscribe(subname.c_str())) {
        client.publish(topic, sussce);
      }
    } else if (action == "unsubscribe") {
      if (client.unsubscribe(subname.c_str())) {
        client.publish(topic, sussce);
      }
    }
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    return;
  } else {
    // 序列化JsonDocument对象
    String json_data;
    serializeJson(doc, json_data);
    String data = frameHead + json_data + frameTail;
    Serial.println(data);
  }
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}


// 重新连接


void loop() {
  // 检查MQTT连接是否正常
  if (!client.connected()) {
    reconnect_mqtt();
  }

  client.loop();
}

//串口中断入口
void serialEvent() {
  String rx_data = "";
  while (Serial.available()) {
    // 读取串口数据
    rx_data += char(Serial.read());
    delay(2);  //这里不能去掉，要给串口处理数据的时间
  }
  //Serial.println(rx_data);
  String data = getMiddleData(String(rx_data), frameHead, frameTail);
  if (data == "") {
    Serial.println("没有帧头帧尾");
    return;
  }
  // 表示格式正确
   Serial.println(rx_data);
  forwardCustomizeAP(data);
}

// AP模式,发布到指定主题
void forwardAP(const char *topic, String jsonString) {
  //Serial.println(jsonString);
  client.publish(topic, jsonString.c_str());
}


// AP模式,发布到自定义主题
void forwardCustomizeAP(String jsonString) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    //Serial.print("forwardCustomizeAP deserializeJson() failed: ");
    //Serial.println(error.c_str());
    return;
  }

  String topic = doc["topic"].as<String>();  // 获取 name 字段的值
  //Serial.println(topic);
  if (topic != "null") {
    client.publish(topic.c_str(), jsonString.c_str());
  } else {
    // json数据里没有topic 就传给默认主题
    client.publish(DefaultPubTopic, jsonString.c_str());
  }
  //Serial.println(jsonString.c_str());
}

//去掉帧头帧尾,获取中间的数据
String getMiddleData(String str, const char start, const char end) {
  // 查找帧头位置
  int startIdx = str.indexOf(start);
  if (startIdx == -1) {
    // 如果找不到帧头，则返回空字符串
    return "";
  }

  // 查找帧尾位置
  int endIdx = str.lastIndexOf(end);
  if (endIdx == -1) {
    // 如果找不到帧尾，则返回空字符串
    return "";
  }
  // 截取中间的数据部分
  return str.substring(startIdx + 1, endIdx);
}



void reconnect_mqtt() {
  // 断开MQTT连接
  client.disconnect();
  Serial.println("Disconnected from MQTT server");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  // 重新连接MQTT服务器
  while (!client.connected()) {
    if (client.connect(clientId.c_str(), mqttUserName, mqttPassword)) {
      // publish and subscribe
      client.subscribe(DefaultTopic);
      client.publish(DefaultPubTopic, "ESP8266上线");
      Serial.println("reconnect to MQTT server");
    } else {
      Serial.println("Failed to reconnect to MQTT server");
      delay(2000);
      // 检查是不是WIFI问题
      if (WiFi.status() != WL_CONNECTED) {
        reconnect_wifi();
      }
    }
  }
}

void reconnect_wifi() {
  // 断开WiFi连接
  WiFi.disconnect();
  Serial.println("Disconnected from WiFi");

  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on (Note that LOW is the voltage level
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(1000);
  }
  Serial.println("Connected to the WiFi network");
}
