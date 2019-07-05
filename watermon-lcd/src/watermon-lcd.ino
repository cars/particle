//#include <Particle.h>

// This #include statement was automatically added by the Particle IDE.
#include <oled-wing-adafruit.h>
OledWingAdafruit oled;

// This #include statement was automatically added by the Particle IDE.
#include <JsonParserGeneratorRK.h>

#define numNodes 2


String nodeIdsArray[numNodes] = {"e00fce68056c78de89fdef0b","e00fce681973306f2d84e3cd"};
String nodeNamesArray[numNodes] = {"Coir","Soil"};
unsigned long lastHeardFrom[numNodes] = {0,0};
retained String displayTable[4] = {"Node   Max |Min |Curr","TBD","TBD","TBD"};



unsigned long ctr; 

// Pin 7 has an LED connected on most Arduino boards.
// give it a name:
int led = 7;

//int waterMon = A0;

retained int publishingDelay = 5;

bool debug = false;
//char msg[64];

String deviceId;
String deviceName;

int setDeviceName(String newName);
int toggleLED(String state);
int toggleDebug(String state);
int setPublishingDelay(String seconds);


//************************************************************
// Take Mesh.publish events and publish to cloud as well as 
// updating table for LCD display
//************************************************************
void rePublish(const char *event,const char *data){
    Particle.publish(event,data,PUBLIC);
    updateTable(String(data));
}
void localPublish(const char *event,const char *data){
    updateTable(String(data));
}


//************************************************************
// Take Mesh.publish events for upper and lower bounds for a 
// device and publish to cloud
//************************************************************
void publishBounds(const char *event,const char *data){
    Particle.publish(event,data,PUBLIC);
    //updateTable(String(data));
}


//************************************************************
//Function to let us toggle blue led remotely
//************************************************************


void setup() {
    //Set LED Pin 
    pinMode(led, OUTPUT);     
    // Set Moisture Sensor Pin
    //pinMode(waterMon,INPUT);
    
    
    oled.setup();
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setTextColor(WHITE);
    
    Serial.begin(9600);
    ctr = 0;
    Mesh.subscribe("CRT_MOISTURESTATS",rePublish);
    //Mesh.subscribe()
    Mesh.subscribe("crt_mesh/stats/public",rePublish);
    Mesh.subscribe("crt_mesh/stats/local",localPublish);
    deviceId = String(System.deviceID());
    deviceName = deviceId;
    
//  Particle.variable("moistureMax",moistureMax);
//  /article.variable("moistureMin",moistureMin);
 // Particle.variable("rawValue",readValue);
    
    //Particle.variable("nodeDeviceID",deviceId);
    Particle.variable("deviceName",deviceName);
    Particle.variable("publishingDelay",publishingDelay);
  
    Particle.function("toggleLed",toggleLED);
    Particle.function("setName",setDeviceName);
    Particle.function("toggleDebug",toggleDebug);
    Particle.function("setPublishingDelay",setPublishingDelay);
    //
    //Particle.function("saveValues",storeDeviceValues);
    oled.setup();
    
}
//
//************************************************************
//Function to let us toggle blue led remotely
//************************************************************

void loop(){
    Serial.println(ctr);
    if (ctr > 4){
        // do some admin stuff....
        Serial.println("Calling checkUpdateStatus()");
        checkUpdateStatus();
        Serial.println("Back from checkUpdateStatus()");        
        ctr = 0;
    }
    
    ctr++;
    updateOLED();
    delay(5000);
    
}

//************************************************************
//Function to let us toggle blue led remotely
//************************************************************
int setPublishingDelay(String seconds){
    char msg[65];
    if (seconds.toInt() > 0  && seconds.toInt() < 300){
        sprintf(msg,"setting publishingDelay from [%i] to [%i]",publishingDelay,seconds.toInt());
        publishingDelay = seconds.toInt();
        //Mesh.publish("crt_mesh/config/publishdelay",seconds);
        Mesh.publish("crt_mesh/configset/publishdelay",seconds);
        Particle.publish("crt_mesh/status/master",msg);
        return 1;
    } else {
        sprintf(msg,"Invalid Publishing Delay value [%s] Requested...remaining at [%i]",seconds.c_str(),publishingDelay);
        Particle.publish(msg);
        return -1;
    }

}

//************************************************************
//Function to let us toggle blue led remotely
//************************************************************
int setDeviceName(String newName){
    char msg[128];
    sprintf(msg,"Changing Name from %s to %s",deviceName.c_str(),newName.c_str());
    deviceName = newName;
    Particle.publish("crt_mesh/status/master",msg);
    return 0;
    
};
//************************************************************
//Function to let us toggle blue led remotely
//************************************************************
int toggleDebug(String state){
    if (state.toUpperCase() == "ON"){
        debug = true;
        Particle.publish("crt_mesh/status","Setting debug ON",PUBLIC);
        return 1;
    }
    if (state.toUpperCase() == "OFF"){
        debug = false;
        Particle.publish("crt_mesh/status/master","Setting debug OFF",PUBLIC);
        return 1;
    }
    Particle.publish("crt_mesh/status/master","invalid value for debug doing nothing",PUBLIC);
    return -1;
}
//************************************************************
//Function to let us toggle blue led remotely
//************************************************************
int toggleLED(String state){
    if (state.toUpperCase() == "ON"){
        digitalWrite(led,HIGH);
        Particle.publish("CRT_STATUS","Setting LED ON",PUBLIC);
        return 1;
    }
    if (state.toUpperCase() == "OFF"){
        digitalWrite(led,LOW);
        Particle.publish("crt_mesh/status/master","Setting LED OFF",PUBLIC);
        return 1;
    }
    Particle.publish("crt_mesh/status/master","Setting LED to invalid value doing nothing",PUBLIC);
    return -1;
    
}

//************************************************************
//Function to let us toggle blue led remotely
//************************************************************
void checkUpdateStatus(){
    
    char msg[128];
    sprintf(msg,"Entering checkUpdateStatus()");
    //Particle.publish("CRT_STATUS/mesh/nodestatus",msg);
    Serial.println(msg);
    unsigned long now = millis();
    sprintf(msg,"Now is %lu",now);
    Serial.println(msg);

    for (int idx = 0; idx <numNodes; idx++){
        float howLong = (now - lastHeardFrom[idx])/1000.0;
        sprintf(msg,"Idx =[%i], nodeName is[%s], Last Heard from [%8.2f] seconds ago",idx,nodeNamesArray[idx].c_str(),howLong);
        Serial.println(msg);
        if ((now - lastHeardFrom[idx]) > (publishingDelay * 1000 * 3)){
            Serial.println(howLong);
            sprintf(msg,"Haven't heard from node [%s] aka [%s] in %f seconds",nodeIdsArray[idx].c_str(),nodeNamesArray[idx].c_str(),howLong);
            Serial.println(msg);
            Particle.publish("crt_mesh/status/node",msg);
            delay(1000);
        }
        Serial.println("Out of if");
    
    }
    Serial.println("leaving checkupdatestatus");
    //Serial.println("past publish");
    return;
}

//************************************************************
//Function to let us toggle blue led remotely
//************************************************************

void updateOLED(){
    char msg[64];
    if (debug){
        Particle.publish("crt_mesh/status/master","in UpdateOLED",PUBLIC);
        delay(1000);
    }
    Serial.println("In updateOLED");
    oled.clearDisplay();
    Serial.println("Cleared Display");
    if (debug){
        Particle.publish("crt_mesh/status/master","clear",PUBLIC);
        delay(1000);
    }
    oled.setTextSize(1);
    Serial.println("Set Text Size");
    if (debug){
        Particle.publish("crt_mesh/status/master","textsize",PUBLIC);
        delay(1000);
    }
    oled.setTextColor(WHITE);
    Serial.println("set TextColor");
    if (debug){
        Particle.publish("crt_mesh/status/master","textcolor",PUBLIC);
        delay(1000);
    }
    oled.setCursor(0,0);
    Serial.println("set cursor");
    if (debug){
        Particle.publish("crt_mesh/status/master","setCursor",PUBLIC);
        delay(1000);
    }
    int numItems =sizeof(displayTable)/sizeof(String);
    String tmpString;
    for (int idx=0; idx <numItems; idx++){
        tmpString = displayTable[idx];
        sprintf(msg,"idx = [%i], line = [%s]",idx,tmpString.c_str());
        Serial.println(msg);
        if (debug){
            Particle.publish("crt_mesh/status/master",msg,PUBLIC);
            delay(1000);
        }
        //oled.clearDisplay();
        //oled.setCursor(0,0);
        oled.print(tmpString.c_str());
        //oled.display();
        //delay(2000);
    }
    oled.display();
    return;
}

//************************************************************
//Function to let us toggle blue led remotely
//************************************************************

void updateTable(String data){
    
    JsonParserStatic<256, 20> parser1;
    if (debug){
        Particle.publish("crt_mesh/status/master","in UpdateTable",PUBLIC);
        delay(1000);
    };
    String nodeId;
    int max;
    int min;
    int current;
    String temp;
    char tmpLine[21];
    char statLine[128];
    parser1.clear();
    parser1.addString(data);
    if (debug){
        sprintf(statLine,"Data is [%s]",data.c_str());
        Particle.publish("crt_mesh/status/master",statLine);
    }
    if (!parser1.parse()){
        Particle.publish("crt_mesh/status/master","Failed to Parse",PUBLIC);
        delay(1000);
        return;
    }
    if(!parser1.getOuterValueByKey("node",nodeId)){
        Particle.publish("crt_mesh/status/master","Failed to get value outer for node",PUBLIC);
        return;
    }
    
    if(!parser1.getOuterValueByKey("max",max)){
        Particle.publish("crt_mesh/status/master","Failed to get value outer for max",PUBLIC);
        return;        
    }
    if(!parser1.getOuterValueByKey("min",min)){
        Particle.publish("crt_mesh/status/master","Failed to get value outer for min",PUBLIC);
        return;        
    }
    if(!parser1.getOuterValueByKey("current",current)){
        Particle.publish("crt_mesh/status/master","Failed to get value outer for current",PUBLIC);
        return;        
    } else {
        if (debug){
            Particle.publish("crt_mesh/status/master","Parsed JSON successfully",PUBLIC);
        }
    }
    parser1.clear();

    for (int idx=0; idx <sizeof(nodeIdsArray); idx++ ){
        if (nodeId == nodeIdsArray[idx]){
            sprintf(tmpLine,"%-6s %4i|%4i|%4i",String(nodeNamesArray[idx]).substring(0,6).c_str(),max,min,current);
            displayTable[idx+1] = String(tmpLine);
            unsigned long now = millis();
            //update our last heard from table for this node
            lastHeardFrom[idx] = now;
        }
    }
    
}

