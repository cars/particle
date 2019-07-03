/*
  watermon-node
  Reads a capacitive moisture sensor and publishes info to the mesh. 
 */
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

//int junk = 100;
//retained uint32_t moistureMin  = 1900;
//retained uint32_t moistureMax  = 3200;
retained int junk = 1000;
retained uint32_t moistureMin  = 1900;
retained uint32_t moistureMax  = 3200;

// Pin 7 has an LED connected on PArticle boards
// give it a name:
int led = 7;

// Moisture Sensor attached to A0 Pin
int waterMon = A0;

//Current Value for moisture
//uint32_t readValue = 0;
int readValue = 0;
int delaySeconds=20;

String deviceId;

String deviceName;


char msg[128];
char json[128];
String publicEventName = "crt_mesh/stats/public/";
//set up some functions
int setDeviceName(String newName);
int flashLED(String state);
int setPublishingDelay(String seconds);

void setPublishingDelayFromMaster(const char *event,const char *data){
    setPublishingDelay(String(data));

}

// the setup routine runs once when you press reset:
void setup() {
    
    Serial.begin(9600);
    //SET BLUE LED pin
    pinMode(led, OUTPUT);     
    // Set Pin for Water Monitor
    pinMode(waterMon,INPUT);
    Serial.println("Setting particle.variable stuff");
    Particle.variable("current",readValue);
    Particle.variable("moistureMax",moistureMax);
    Particle.variable("moistureMin",moistureMin);
    //This is kind of redundant
    Particle.variable("nodeDeviceID",deviceId);
    Particle.variable("deviceName",deviceName);
    Particle.variable("delaySeconds",delaySeconds);
    
    Serial.println("Setting particle.function stuff");
    Particle.function("toggleLed",flashLED);
    Particle.function("setName",setDeviceName);
    Particle.function("setDelay",setPublishingDelay);
  
    Mesh.subscribe("crt_mesh/configset/publishdelay",setPublishingDelayFromMaster);
    deviceId = String(System.deviceID());
    deviceName = deviceId;
    Serial.println("Setting pulic event name");
    publicEventName.concat(deviceId);
    Serial.println(publicEventName.c_str());

//    moistureMax = 0;
//moistureMin = 6000;
}
//
void loop(){
//    char json[128];
//    char msg[128];
    //Read the sensor
    readValue = analogRead(waterMon);
    sprintf(msg,"analog read value[%i]",readValue);
    Serial.println(msg);

    sprintf(msg,"Moisture Max before comparison [%u]",moistureMax);
    Serial.println(msg);
    if (moistureMax > 10000){
        moistureMax = 3100;
    }
    moistureMax = max(moistureMax,readValue);
    sprintf(msg,"MoistureMAx after comparison [%u]",moistureMax);
    Serial.println(msg);
    sprintf(msg,"Moisture min before comparison [%u]",moistureMin);
    Serial.println(msg);
    moistureMin = min(moistureMin,readValue);
    sprintf(msg,"Moisture min after comparison [%u]");
    Serial.println(msg);
    //do percentage cals in two parts because of int to float conversion
    float percentage = (moistureMax-readValue);
    percentage =percentage/(moistureMax -moistureMin) *100.0;
    
    sprintf(json,"{\"node\":\"%s\",\"max\": %u, \"min\": %u, \"current\": %i,\"percentage\":%f}",deviceName.c_str(),moistureMax,moistureMin,readValue,percentage);
    Serial.println(json);
    Serial.println("Publishing CRT_MOISTURESTATS");
    Mesh.publish("CRT_MOISTURESTATS",json);
//    Mesh.publish(publicEventName.c_str(),json);
//    Particle.publish(publicEventName.c_str(),json);
    sprintf(msg,"Publishing [%s]:[%s]",publicEventName.c_str(),json);
/*    Serial.println("**********************************************");
    Serial.println(msg);
    Serial.println("**********************************************");
    Serial.println(json);
    Serial.println("**********************************************");
*/
    Serial.println("Delaying");
    delay(delaySeconds*1000);
    Serial.println("Done Delaying");
}

int setPublishingDelay(String seconds){
    
//   char msg[65];
    if (seconds.toInt() > 0  && seconds.toInt() < 300){
        sprintf(msg,"setting publishingDelayon [%s] from [%i] to [%i]",deviceName,delaySeconds,seconds.toInt());
        delaySeconds = seconds.toInt();
        Particle.publish("CRT_STATUS",msg);
        return 1;
    } else {
        sprintf(msg,"Invalid Publishing Delay value [%s] Requested...remaining at [%i]",seconds.c_str(),delaySeconds);
        Particle.publish(msg);
        return -1;
    }

}
int setDeviceName(String newName){
//    char msg[128];
    sprintf(msg,"Changing Name from %s to %s",deviceName,newName);
    deviceName = newName;
    Particle.publish("CRT_STATUS",msg);
    return 1;
    
};

int flashLED(String state){
    if (state.toUpperCase() == "ON"){
        digitalWrite(led,HIGH);
        Particle.publish("CRT_STATUS","Setting LED ON");
        return 1;
    }
    if (state.toUpperCase() == "OFF"){
        digitalWrite(led,LOW);
        Particle.publish("CRT_STATUS","Setting LED OFF");
        return 1;
    }
    Particle.publish("CRT_STATUS","Setting LED to invalid value doing nothing");
    return -1;
    
}

//{"node":"e00fce681973306f2d84e3cd","max": 3100, "min": 2046, "current": 2070,"percentage":97.722961}
//crt_mesh/stats/public/e00fce681973306f2d84e3cd
//1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890