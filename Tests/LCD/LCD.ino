#include <Wire.h>
#include <LiquidCrystal_I2C.h>
 
LiquidCrystal_I2C lcd(0x3F, 16, 2);
int t= 20;
 
void setup() {
lcd.init();
lcd.backlight();
 
for (int i = 0; i <= t; i++) {
lcd.clear();
lcd.print("Cronometro");
lcd.setCursor(0, 1);
lcd.print("Tempo: ");
lcd.setCursor(7, 1);
lcd.print(i);
delay(1000);
}
 
lcd.clear();
lcd.print("Fim!");
 
}
 
void loop() {

    lcd.print("Fim!");
}