/*
  watermon-node
  Reads a capacitive moisture sensor and publishes info to the mesh. Gateway node is
  responsible for then forwarding on. 
  This is version from WEB IDE

  Last modified: 7/5/2019 by CRT 
 */
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

#define moistureMinCeiling  1900
#define moistureMaxFloor  3100

//retained int junk = 1000;

retained uint32_t moistureMin;

retained uint32_t moistureMax;

// Pin 7 has an LED connected on Particle boards
// give it a name:
int led = 7;

// Moisture Sensor attached to A0 Pin
int waterMon = A0;

//Current Value for moisture
uint32_t readValue = 0;

int delaySeconds=20;
//the deviceID
String deviceId;

// the devicename. This will default to deviceID but can be changed to something 
// more friendly
String deviceName;

String publicEventName = "crt_mesh/stats/public/";

//set up some functions
int setDeviceName(String newName);
int flashLED(String state);
int setPublishingDelay(String seconds);


//#######################################################################
//subscribe to mesh event to set publishing delay mesh wide versus on each node
//#######################################################################
void setPublishingDelayFromMaster(const char *event,const char *data){
    Serial.println("Setting publishing delay based on notification from master");
    setPublishingDelay(String(data));
}
//#######################################################################
// the setup routine runs once when you press reset:
//#######################################################################
void setup() {
    //
    char msg[128];
    
    Serial.begin(9600);
    //SET BLUE LED pin
    pinMode(led, OUTPUT);     
    // Set Pin for Water Monitor
    pinMode(waterMon,INPUT);
    
    //
    sprintf(msg,"At startup MoistureMax is [%lu], and moistureMin is [%lu]",moistureMax,moistureMin);
    Serial.println(msg);
    
    //Compensate for weird values that might be in SRAM for retained variables after power 
    //is removed and re-applied
    if (moistureMax > 10000){
        sprintf(msg,"MoistureMax at startup [%u] resetting to [%i]",moistureMax,moistureMaxFloor);
        Serial.println(msg);
        moistureMax = 3100;
    }

    if ((moistureMin < 1000) || (moistureMin > 10000)){
        sprintf(msg,"MoistureMax at startup [%u] resetting to [%i]",moistureMin,moistureMinCeiling);
        Serial.println(msg);
        moistureMin = moistureMinCeiling;
    }

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
//#######################################################################

//#######################################################################
void loop(){
    char json[128];
    char msg[128];

    //Read the sensor
    readValue = analogRead(waterMon);
    sprintf(msg,"analog read value[%i]",readValue);
    Serial.println(msg);

    // let's check/set max 
    sprintf(msg,"Moisture Max before comparison [%i]",moistureMax);
    Serial.println(msg);
//    //if (moistureMax > 10000){
//        moistureMax = 3100;
//    }
    moistureMax = max(moistureMax,readValue);
    sprintf(msg,"MoistureMAx after comparison [%i]",moistureMax);
    Serial.println(msg);
    // let's check/set min
    sprintf(msg,"Moisture min before comparison [%i]",moistureMin);
    Serial.println(msg);
    moistureMin = min(moistureMin,readValue);
    sprintf(msg,"Moisture min after comparison [%i]");
    Serial.println(msg);
    //calculate a relative percentage
    //do percentage cals in two parts because of int to float conversion
    float percentage = (moistureMax-readValue);
    percentage =percentage/(moistureMax -moistureMin) *100.0;
    
    //build JSON we'll use to publish the info.
    sprintf(json,"{\"node\":\"%s\",\"max\": %i, \"min\": %i, \"current\": %i,\"percentage\":%6.2f}",deviceName.c_str(),moistureMax,moistureMin,readValue,percentage);
    Serial.println(json);

    //publish via  general event
    Serial.println("Publishing CRT_MOISTURESTATS");
    Mesh.publish("CRT_MOISTURESTATS",json);
    sprintf(msg,"Publishing [%s]",publicEventName.c_str());
    Serial.println(msg);
    //publish via node specific event
    Mesh.publish(publicEventName,json);
    delay(delaySeconds*1000);
}

//#######################################################################
//set a delay in how frequently we're publishing events between 1 & 300 seconds, default is 20
//#######################################################################
int setPublishingDelay(String seconds){
    
    char msg[65];
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

//#######################################################################
// function to allow user/app to give the node a friendly name
//#######################################################################
int setDeviceName(String newName){
    char msg[128];
    sprintf(msg,"Changing Name from %s to %s",deviceName,newName);
    deviceName = newName;
    Particle.publish("CRT_STATUS",msg);
    return 1;
    
};

//#######################################################################
// turn LED on or off via function
//#######################################################################
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
