#include <Arduino.h>
#include <PubSubClient.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <WiFi.h>

uint8_t led_lid;
const char * wifi_ssid = "zuhmcu";
const char * wifi_pw = "microhobby";
const char * mqtt_broker_addr = "test.mosquitto.org";
uint16_t mqtt_broker_port = 1883;
const char * mqtt_topic_touch = "zuhmcu/esp32/touch";
const char * mqtt_client_id = "zuhmcu";
const int sensor_touch_pin = 4;
int sensor_touch_value = 0;

WiFiClient wifi_sta;
PubSubClient mqtt_client(wifi_sta);

void sensor_measure(int * sensor_value, int pin){
  *sensor_value = touchRead(pin);
}

void mqtt_printf(byte * msg, unsigned int len, const char * name){
  printf("[ %s ]:\n", name);
  for(int i = 0; i < len; i++){
    printf("%c", msg[i]);
  }
  printf("\n");
}

void mqtt_callback(char * topic, byte * message, unsigned int length){
  if(String(topic) == mqtt_topic_touch){
    mqtt_printf(message, length, "touch");
  }
}

void mqtt_connect() {
  while(!mqtt_client.connected()){
    printf("Connecting to MQTT Broker\n");

    if(mqtt_client.connect(mqtt_client_id)){
      printf("MQTT Connected to %s:%d\n", mqtt_broker_addr, mqtt_broker_port);
      mqtt_client.subscribe(mqtt_topic_touch);
      printf("Subscribed to %s\n", mqtt_topic_touch);
    }else{
      delay(5000);
    }
  }
}

void mqtt_buffer_pack(int sensor_data, const char * location, char * buffer){
  char header[16];
  sprintf(header, "%s/", location);
  sprintf(buffer, "%s", header);
  sprintf(buffer +  + strlen(header), "%d", sensor_data);
}

void mqtt_setup() {
  mqtt_client.setServer(mqtt_broker_addr, mqtt_broker_port);
  mqtt_client.setCallback(mqtt_callback);
}

void mqtt_data_proceed() {
  char mqtt_msg_buffer[256] = {0};
  
  sensor_measure(&sensor_touch_value, sensor_touch_pin);
  mqtt_buffer_pack(sensor_touch_value, "garden", mqtt_msg_buffer);

  mqtt_client.publish(mqtt_topic_touch, mqtt_msg_buffer);
}

void wifi_setup() {
  printf("Connecting to %s\n", wifi_ssid);
  
  WiFi.begin(wifi_ssid, wifi_pw);
  while(WiFi.status() != WL_CONNECTED) delay(100);

  printf("WiFi Connected\n");
}

void setup() {
  led_lid = 0;
  wifi_setup();
  mqtt_setup();
}

void loop() {
  if(!mqtt_client.connected()){
    mqtt_connect();
  }

  mqtt_client.loop();
  mqtt_data_proceed();
  delay(500);
}