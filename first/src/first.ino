// This #include statement was automatically added by the Particle IDE.
#include <oled-wing-adafruit.h>
OledWingAdafruit oled;


char msg[64];
void setup()
{
   oled.setup(); 
    // Clear the buffer.
    oled.clearDisplay();
    oled.display();
    Serial.being(9600);
}
void loop()
{
    displayVoltage();
    delay(3*1000);
}
void displayVoltage(){
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0,0);
    char line[21];
    float voltage = analogRead(BATT) * 0.0011224;
    //sprintf(msg,"%1.2fvolts right now\nline2\nline3\nline4\nline5",voltage);
    //oled.print("123456789012345678901\n");
    oled.print("Node   Max |Min |Curr");
    sprintf(line,"%-6s %4i|%4i|%4i",String("Southern Corner").substring(0,6).c_str(),3477,200,2211);
    oled.print(line);
    sprintf(line,"%-6s %4i|%4i|%4i",String("north corner").substring(0,6).c_str(),477,200,2211);
    oled.print(line);
    sprintf(line,"%-6s %4i|%4i|%4i","door",477,2200,211);
    oled.print(line);

    //oled.print(msg);
    oled.display();
}
//  123456789012345678901
//  Name   max /min /curr