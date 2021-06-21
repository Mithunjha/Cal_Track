//==============================================================
//                           LIBRARIES
//==============================================================
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>

//==============================================================
//                          Variables
//==============================================================
String device_id = "159"; //Unique id that will be provided to the user
String MQTTmsg ; 
String IP;
String Item;
int Calories = -1;
int consum = -1;
String date = "";
String Quantity = ""; 
String Ingredient = "";
String Authentication = "";
int latency = 1000; // Waiting/Latency time for MQTT transmission
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 300000; //Timeout Time in milliseconds --> 5 minutes for powering down due to inactive
const long timeoutTime_min = timeoutTime/1000; //Timeout Time in seconds --> 5 minutes for powering down due to wifi not connected
const char* mqtt_server = "test.mosquitto.org";
WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80); //Server on port 80
//==============================================================
//                       HTML Webpages
//==============================================================

//=================================================================================================================
//   Home Page --> Redirect authentication, Redirect daily consumption, Calorie Checker, Poweroff Confirmation
//=================================================================================================================

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

  <style>
    html { 
      font-family: Helvetica; 
      display: inline-block; 
      margin: 0px auto; 
      text-align: center;
      }
    input[type=submit] {
    width: 60%;
    background-color: lightgreen;
    color: black;
    padding: 14px 20px;
    margin: 8px 0;
    border: none;
    cursor: pointer;
    }
    

input[type=submit]:hover {
  background-color: #45a049;
}
  </style>


  <h2 style="color:black;font-size:50px;">Cal-Track</h2>

    <form action="/redirectauthentication" method="post">
      <input style="color:black;font-size:35px;" type="submit" value="Authenticate"><br><br>
      </form> 
     <form action="/redirectdailyconsumption" method="post">
      <input style="color:black;font-size:35px;" type="submit" value="Daily Consumption"><br><br>
      </form> 
      <form action="/caloriechecker" method="post">
      <input style="color:black;font-size:35px;" type="submit" value="Calorie Checker"><br><br>
      </form> 
      <form action="/poweroffconfirm" method="post">
      <input style="color:black;font-size:35px;" type="submit" value="POWER OFF"><br><br>
      </form>
<img src="https://static01.nyt.com/images/2016/08/11/well/well_nutritionforrunners_gif/well_nutritionforrunners_gif-jumbo-v5.gif" style="width:30%;height:250px;">
</body>
</html>
)=====";


//=================================================================
//           Redirect Authentication --> Authentication
//=================================================================


const char Redirect_page_authentication[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

  <style>
    html { 
      font-family: Helvetica; 
      display: inline-block; 
      margin: 0px auto; 
      text-align: center;
      }
    input[type=submit] {
    width: 80%;
    background-color: lightgreen;
    color: black;
    padding: 14px 20px;
    margin: 8px 0;
    border: none;
    cursor: pointer;
    }
    
input[type=submit]:hover {
  background-color: #45a049;
}
  </style>


  <h2 style="color:black;font-size:50px;">Cal-Track</h2>

  
    <form action="/authentication" method="post">
      <input style="color:black;font-size:35px;" type="submit" value="Check your Authentication Status!"><br><br>
      </form> 

</body>
</html>
)=====";



//=================================================================
//       Redirect daily consumption --> Daily consumption
//=================================================================



const char Redirect_page_dailyconsumption[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

  <style>
    html { 
      font-family: Helvetica; 
      display: inline-block; 
      margin: 0px auto; 
      text-align: center;
      }
    input[type=submit] {
    width: 550px;
    background-color: lightgreen;
    color: black;
    padding: 14px 20px;
    margin: 8px 0;
    border: none;
    cursor: pointer;
    }
    

input[type=submit]:hover {
  background-color: #45a049;
}
  </style>


  <h2 style="color:black;font-size:50px;">Cal-Track</h2>

    <form action="/dailyconsumption" method="post">
      <input style="color:black;font-size:35px;" type="submit" value="Check your Daily Consumption!"><br><br>
      </form> 
   
</body>
</html>
)=====";



//=================================================================
//                Caloriechecker --> Redirect calorie
//=================================================================

const char Calculate_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

  <style>
    html { 
      font-family: Helvetica;  
      display: inline-block; 
      margin: 0px auto; 
      text-align: center;
      }
    input[type=submit] {
    width: 40%;
    background-color: lightgreen;
    color: black;
    padding: 14px 20px;
    margin: 8px 0;
    border: none;
    cursor: pointer;
    }

input[type=submit]:hover {
  background-color: #45a049;
}

input[type=reset] {
    width: 40%;
    background-color: lightgreen;
    color: black;
    padding: 14px 20px;
    margin: 8px 0;
    border: none;
    cursor: pointer;
}

input[type=text], select {
  width: 50%;
  padding: 12px 20px;
  margin: 8px 0;
  display: inline-block;
  border: 2px solid #ccc;
  border-radius: 4px;
  box-sizing: border-box;
}

input[type=reset]:hover {
  background-color: #45a049;
}
  </style>

  <h2 style="color:black;font-size:50px;">Cal-Track</h2>

  
  <form action="/redirectcalorie" method="post" autocomplete="off">
    <label style="color:black;font-size:30px;">Quantity:</label><br>
    <input type="text" name="Quantity" value="" style="color:black;font-size:20px;" required>
    <br><br>
    <label style="color:black;font-size:30px;">Ingredient:</label><br>
    <input type="text" name="Ingredient" value="" style="color:black;font-size:20px;" required>
    <br><br>
    <input type="submit" value="Submit" style="color:black;font-size:30px;"><br>
    <input type="reset" value="Clear" style="color:black;font-size:30px;"><br><br>
    <label style="color:black;font-size:25px;"><b><u>Examples</b></u></label><br><br>
    <label style="color:black;font-size:20px;">3 - apples</label><br>
    <label style="color:black;font-size:20px;">1 tbsp - sugar</label><br>
    <label style="color:black;font-size:20px;">1/2 glass - milk</label><br>
    <label style="color:black;font-size:20px;">Half - carrot</label><br>
  </form>



<br><br> <a href='/'> Return to Homepage </a>
</body>
</html>
)=====";


//=================================================================
//                Redirect calorie --> Calorie
//=================================================================


const char Redirect_page_calorie[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

  <style>
    html { 
      font-family: Helvetica; 
      display: inline-block; 
      margin: 0px auto; 
      text-align: center;
      }
    input[type=submit] {
    width: 75%;
    background-color: lightgreen;
    color: black;
    padding: 14px 20px;
    margin: 8px 0;
    border: none;
    cursor: pointer;
    }

input[type=submit]:hover {
  background-color: #45a049;
}
  </style>


  <h2 style="color:black;font-size:50px;">Cal-Track</h2>

    <form action="/calorie" method="post">
      <input style="color:black;font-size:35px;" type="submit" value="Proceed to the Results!"><br><br>
      </form> 
  
</body>
</html>
)=====";

//=================================================================
//               Power Off Confirmation --> Off/Cancel
//=================================================================

const char Poweroffconfirm[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>
html { 
      font-family: Helvetica; 
      display: inline-block; 
      margin: 0px auto; 
      text-align: center;
      }
.submit {
  background-color: #4CAF50; /* Green */
  border: none;
  color: white;
  padding: 16px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
  margin: 4px 2px;
  transition-duration: 0.4s;
  cursor: pointer;
  width: 400px;
}

.submit1 {
  background-color: lightgreen; 
  color: black;
}

.submit1:hover {
  background-color: darkgreen;
  color: white;
}

.submit2 {
  background-color: pink; 
  color: black;
}

.submit2:hover {
  background-color: red;
  color: white;
}


</style>

<h2 style="color:black;font-size:45px;">Confirm Poweroff!</h2>
<form action="/poweroff" method="post">
<input style="font-size:35px;"type="submit" value="YES" class="submit submit1"><br><br>
</form> 
<form action="/" method="post">
<input style="font-size:35px;"type="submit" value="NO" class="submit submit2">
</form> 

</html>

)=====";


//=================================================================
//                            Power Off
//=================================================================

const char Poweroff[] PROGMEM = R"=====(
<!DOCTYPE html> <html>
<body>
<style> html {font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;}</style>
<h2 style="color:red;font-size:50px;">Device Off</h2>
</body>
</html>
)=====";

//=================================================================
//                        Authentication
//=================================================================

String Auth(String Authentication){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<body>\n";
  ptr +="<style> html {font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;}input[type=submit] {width: 60%;background-color:lightgreen;color: black;padding: 14px 20px;margin: 8px 0;border: none;border-radius: 4px;cursor: pointer;}input[type=button] {width: 60%;background-color: #45a049;color: black;padding: 14px 20px;margin: 8px 0;border: none;border-radius: 4px;cursor: text;}input[type=submit]:hover {background-color: #45a049;}</style>\n";
  ptr +="<h2 style=\"color:black;font-size:50px;\">Cal-Track</h2>\n";
  ptr +="<form>\n";
  if (Authentication=="Device is Authenticated"){
    ptr +="<input style=\"color:black;font-size:40px;\" type=\"button\" value=\"Authenticated\" disabled><br><br>\n";
  }
  else{
    ptr +="<input style=\"color:black;background-color:red;font-size:40px;cursor:auto;\" type=\"button\" value=\"Unauthenticated\" disabled>\n";
    ptr +="<input style=\"color:red;font-size:25px;background-color:white;width: 75%;\" type=\"button\" value=\"Login to the Dashboard and Try again!\" disabled><br><br>\n";
  }
  ptr +="</form>\n";
  ptr +="<br><br> <a href='/'> Return to Homepage </a>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}



//=================================================================
//                       Calorie Checker
//=================================================================

String Calc_calorie(int Calories, String Quantity, String Ingredient){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<body>\n";
  if (Calories == 0){
    ptr +="<style> html {font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;}</style>\n";
    ptr +="<h2 style=\"color:red;font-size:50px;\">Ingredients Unknown!</h2>\n";
    ptr +="<a style=\"font-size:20px;\" href='/caloriechecker'> Check and Try Again! </a> ";
    ptr +="</body>\n";
    ptr +="</html>\n"; 
  }
  else if (Calories == -1){
    ptr +="<style> html {font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;}</style>\n";
    ptr +="<h2 style=\"color:red;font-size:50px;\">Server Down</h2>\n";
    ptr +="<a style=\"font-size:20px;\" href='/'> Return to Homepage </a> ";
    ptr +="</body>\n";
    ptr +="</html>\n";  
  }
  else{
    ptr +="<style> html {font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;}input[type=submit] {width: 60%;background-color: lightgreen;color: black;padding: 14px 20px;margin: 8px 0;border: none;border-radius: 4px;cursor: pointer;}input[type=button] {width: 60%;background-color: #45a049;color: black;padding:14px 20px;margin: 8px 0;border: none;border-radius: 4px;cursor: pointer;}input[type=submit]:hover {background-color: #45a049;}</style>\n";
    ptr +="<h2 style=\"color:black;font-size:50px;\">Cal-Track</h2>\n";
  
    ptr +="<h3 style=\"color:green;font-size:35px;\">";
    ptr +=(String)Quantity;
    ptr +=" ";
    ptr +=(String)Ingredient;
    ptr +="</h3>";
    ptr +="<p style=\"color:lightgreen;font-size:35px;\">";
    ptr +=(int)Calories;
    ptr +=" ";
    ptr +="Calories</p>";
    ptr +="<br><br> <a style=\"font-size:20px;\" href='/caloriechecker'> Calculate for another ingredient </a> ";
    ptr +="</body>\n";
    ptr +="</html>\n";  
  }
  return ptr;
}

//=================================================================
//                        Daily Consumption
//=================================================================

String Calc_daily_consum(int consum, String date){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<body>\n";
  if (consum == -1 || date == ""){
    ptr +="<style> html {font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;}</style>\n";
    ptr +="<h2 style=\"color:red;font-size:50px;\">Server Down</h2>\n";
    ptr +="<a style=\"font-size:20px;\" href='/'> Return to Homepage </a> ";
    ptr +="</body>\n";
    ptr +="</html>\n";  
  }
  else {
    ptr +="<style> html {font-family: Helvetica;display: inline-block;margin: 0px auto;text-align: center;}input[type=submit] {width: 60%;background-color: lightgreen;color: black;padding: 14px 20px;margin: 8px 0;border: none;border-radius: 4px;cursor: pointer;}input[type=button] {width: 60%;background-color: #45a049;color: black;padding:14px 20px;margin: 8px 0;border: none;border-radius: 4px;cursor: pointer;}input[type=submit]:hover {background-color: #45a049;}</style>\n";
    ptr +="<h2 style=\"color:black;font-size:50px;\">Cal-Track</h2>\n";
  
    ptr +="<h3 style=\"color:green;font-size:35px;\"> Consumption on ";
    ptr +=(String)date;
    ptr +="</h3>";
    ptr +="<p style=\"color:lightgreen;font-size:35px;\">";
    ptr +=(int)consum;
    ptr +=" ";
    ptr +="Calories</p>";
    ptr +="<br><br> <a style=\"font-size:20px;\" href='/'> Return to Homepage </a> ";
    ptr +="</body>\n";
    ptr +="</html>\n";
  }
  return ptr;
}


//=================================================================
//                      Functions for HTML
//=================================================================

//=================================================================
//                  Home Page --> Startup Page
//=================================================================
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
 previousTime = millis();
 Serial.println(previousTime); 
}

//========================================================================
// Redirectauthentication --> Page to direct to authentications status
//========================================================================
void handleredirectAuth(){
 String s = Redirect_page_authentication;
 MQTTmsg = device_id + " " + "Authenticated";
 client.publish("outAuth", (char*) MQTTmsg.c_str());
 Serial.print("OutAuth : "); 
 Serial.println(MQTTmsg);
 delay(latency); // Waiting/Latency time for MQTT tranmission
 server.send(200, "text/html", s);
 previousTime = millis();
 Serial.println(previousTime); 
}

//=================================================================
//                       Authentication page
//=================================================================
void handleAuth() {
 server.send(200, "text/html", Auth(Authentication));
 Serial.println("Authentication : "); 
 Serial.print(Authentication); 
 Authentication = "" ;
 previousTime = millis();
 Serial.println(previousTime);
}

//=================================================================
//                     Calorie Checker Interface
//=================================================================
void handleCalc() {
 String s = Calculate_page;
 server.send(200, "text/html", s); 
 previousTime = millis();
 Serial.println(previousTime);
}

//======================================================================
//  Redirectcaloriechecker --> Page to direct to the calorie values
//======================================================================
void handleredirectCalorie() {
 String s = Redirect_page_calorie;
 Quantity = server.arg("Quantity"); 
 Ingredient = server.arg("Ingredient");
 String Item = Quantity + " " + Ingredient ;
 Serial.println(Item) ;
 MQTTmsg = device_id + " " + Item;
 client.publish("outCalc", (char*) MQTTmsg.c_str());
 Serial.println();
 Serial.print("outCalc : "); 
 Serial.println(MQTTmsg);
 delay(2*latency); // Waiting/Latency time for MQTT tranmission
 server.send(200, "text/html", s); 
 previousTime = millis();
 Serial.println(previousTime);
 }

//=================================================================
//                   Calorie Checker Output page
//=================================================================
void handleCalorie() { 
 Serial.println();
 Serial.print(Quantity + " " + Ingredient) ;
 server.send(200, "text/html", Calc_calorie(Calories,Quantity,Ingredient));
 Serial.println();
 Serial.print("Calories : "); 
 Serial.println(Calories); 
 Calories = -1;
 previousTime = millis();
 Serial.println(previousTime);
}

//==============================================================================
// Redirectdailyconsumption --> Page to direct to the daily consumption value
//==============================================================================
void handleredirectdailyconsum() {
 String s = Redirect_page_dailyconsumption;
 MQTTmsg = device_id + " " + "dailyconsume";
 client.publish("outDailyconsume", (char*) MQTTmsg.c_str());
 Serial.println();
 Serial.print("outDailyconsume : "); 
 Serial.println(MQTTmsg);
 delay(latency); // Waiting/Latency time for MQTT tranmission
 server.send(200, "text/html", s); 
 previousTime = millis();
 Serial.println(previousTime);
}

//=================================================================
//                        Dailyconsumption
//=================================================================
void handledailyconsum() {
 server.send(200, "text/html", Calc_daily_consum(consum,date));
 Serial.println();
 Serial.print("Consumption : "); 
 Serial.println(consum); 
 Serial.print("Date : "); 
 Serial.println(date); 
 consum = -1;
 date = "" ;
 previousTime = millis();
 Serial.println(previousTime);
}

//=================================================================
//                     Power Off Confirmation
//=================================================================
void handlePoweroffconfirm(){
 String s = Poweroffconfirm;
 server.send(200, "text/html", s);
 previousTime = millis();
 Serial.println(previousTime);  
}


//=================================================================
//                          Power Off
//=================================================================
void handlePoweroff(){
 String s = Poweroff;
 server.send(200, "text/html", s);
 previousTime = millis();
 Serial.println(previousTime);
 Serial.println("NodeMCU sleep"); //Power off NodeMCU --> deep sleep mode
 delay(2000);
 ESP.deepSleep(0);
}

//=================================================================
//       WiFI Initialization - Automatic SSID and password
//=================================================================

void setup_wifi() {
  WiFiManager wifiManager;
  // wifiManager.resetSettings(); //Uncomment to delete the saved credentials
  Serial.println("Disconnected.");
  wifiManager.setConfigPortalTimeout(timeoutTime_min);
    
  if(!wifiManager.autoConnect("Caloriecount")) {
    Serial.println("failed to connect and hit timeout"); //Power off (Deep sleep) NodeMCU if WiFi is not connected within the predefined period
    Serial.println("NodeMCU sleep");
    delay(1000);
    ESP.deepSleep(0);
  }
  IP = WiFi.localIP().toString();
  Serial.println(IP);
  Serial.println("Connected.");
  server.begin();
}


//========================================================================================
//   Callback function for subscrbed topics --> inWiFi, inAuth, inCalc, inDailyconsume
//========================================================================================
void callback(String topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Global Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String state;
  String message;
  int timer;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    state += (char)payload[i];
  }
  Serial.println();
  
  if (getValue(state,' ',0)==device_id){

    message = state.substring(getValue(state,' ',0).length()+1);
    Serial.print("Local Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.print(message);
    Serial.println();
       
    if (topic == "inWiFi"){
      Serial.print(message);
    }
    else if (topic == "inAuth"){
      Serial.print(message);
      Authentication = message;
    }
    else if (topic == "inCalc"){
      Serial.print(message);
      Calories = message.toInt();
    }
    else if (topic == "inDailyconsume"){
      Serial.print(message);
      String consum_str = getValue(message,' ',1);
      date = getValue(message,' ',0);
      if (consum_str.length()>0){
        consum = consum_str.toInt();        
      }
      else{
        consum = -1;
      }
    }
  }
}


//=================================================================
//                           Reconnect
//=================================================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      MQTTmsg = device_id + " " + "WiFi is connected";
      client.publish("outWiFi", (char*) MQTTmsg.c_str());
      Serial.print("outWiFi : "); 
      Serial.println(MQTTmsg);
      MQTTmsg = device_id + " " + IP;
      client.publish("outWiFiIP", (char*) MQTTmsg.c_str());
      Serial.print("outWiFiIP : "); 
      Serial.println(MQTTmsg);
      // ... and resubscribe
      client.subscribe("inAuth");
      client.subscribe("inCalc");
      client.subscribe("inDailyconsume");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//==============================================================
//                      Splitting a String
//==============================================================

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}




//==============================================================
//                             SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);
  Serial.setTimeout(1000);
  setup_wifi();
  Serial.println(previousTime);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
  server.on("/", handleRoot);      
  server.on("/caloriechecker", handleCalc); 
  server.on("/authentication", handleAuth); 
  server.on("/poweroff", handlePoweroff); 
  server.on("/calorie", handleCalorie); 
  server.on("/dailyconsumption", handledailyconsum); 
  server.on("/poweroffconfirm", handlePoweroffconfirm); 
  server.on("/redirectauthentication", handleredirectAuth); 
  server.on("/redirectcalorie", handleredirectCalorie); 
  server.on("/redirectdailyconsumption", handleredirectdailyconsum); 

  
  server.begin();                  //Start server
  Serial.println("HTTP server started");
  previousTime = millis();
}
//==============================================================
//                              LOOP
//==============================================================
void loop(void){
  server.handleClient();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  currentTime = millis();
  if (currentTime - previousTime > timeoutTime){
    Serial.println(currentTime);
    Serial.println("NodeMCU sleep"); //Power off NodeMCU (Deep sleep) if it is inactive for more than the predefined period
    ESP.deepSleep(0);
  }
}
