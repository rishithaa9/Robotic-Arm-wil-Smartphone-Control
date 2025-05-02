#include <Arduino.h> //basic functions 
#include <WiFi.h> 

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h> //Used for creating web servers

#include <ESP32Servo.h>
#include <iostream>  //used for outputting data to the screen and also getting input data from the device
#include <sstream>   //enables manipulation and conversion between strings and other data types. 

struct ServoPins
{
  Servo servo;          //defines servo motors
  int servoPin;         //Defines the pins to which the motors are connected
  String servoName;     //Defines the names of each motor
  int initialPosition;  //Defines the initial position of each and every motor
};
std::vector<ServoPins> servoPins =      //This is vector made up of many structures for each motor
{
  { Servo(), 27 , "Base", 90},
  { Servo(), 26 , "Shoulder", 90},
  { Servo(), 25 , "Elbow", 90},
  { Servo(), 33 , "Wristroll", 90},
  { Servo(), 32 , "wristpitch", 90},
  { Servo(), 14 , "Grip", 90},
};

struct RecordedStep     //initialising a structure for the recorded steps
{
  int servoIndex;
  int value;
  int delayInStep;
};
std::vector<RecordedStep> recordedSteps;  //creating a vector to store the recorded steps of the structure Recordedstep

bool recordSteps = false;       //Initially setting the recordedSteps to False
bool playRecordedSteps = false; //Initially setting the PlayrecordedSteps to False

unsigned long previousTimeInMilli = millis();     //used to calculate the time delay in each of the recorded step

const char* ssid     = "RobotArm";    //SSID - Service Set Identifier i.e., the name of the wifi network
const char* password = "12345678";    //password for connecting to the wifi network

AsyncWebServer server(80);   //80 is the default port for HTTP communication
AsyncWebSocket wsRobotArmInput("/RobotArmInput");   //enabling a communication channel to receive info

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no"> //viewport properties means the
    <style>                                                                                               //content visible on a web page

    input[type=button]      //This attributes only apply to the button types
    {
      background-color:gray;
      color:white;
      border-radius:30px;
      width:100%;
      height:40px;
      font-size:20px;
      text-align:center;
    }
        
    .noselect {           //This block of code enables us to view the web page in all different web browsers
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: safeseq;
      width: 100%;
      height: 25px;
      border-radius: 5px;
      background: #d3d3d3;        //Light gray colour
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }
  
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    </style>
  
  </head>
  <body class="noselect" align="center" style="background-color:black">
     
    <h1 style="color: teal;text-align:center;">Synergy creators</h1>
    <h2 style="color: teal;text-align:center;">Robot Arm Control</h2>
    
    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr/><tr/>
      <tr/><tr/>
      <tr>
        <td style="text-align:left;font-size:25px"><b style="color: white">Grip:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Grip" oninput='sendButtonInput("Grip",value)'>
          </div>
        </td>
      </tr> 
      <tr/><tr/>
      <tr>
        <td style="text-align:left;font-size:25px"><b style="color: white">Wrist pitch:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Wristpitch" oninput='sendButtonInput("Wristpitch",value)'>
          </div>
        </td>
      </tr> 
      <tr/><tr/>      
      <tr>
        <td style="text-align:left;font-size:25px"><b style="color: white">Wrist roll:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="wristroll" oninput='sendButtonInput("wristroll",value)'>
          </div>
        </td>
      </tr>  
      <tr/><tr/>      
      <tr>
        <td style="text-align:left;font-size:25px"><b style="color: white">Elbow:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Elbow" oninput='sendButtonInput("Elbow",value)'>
          </div>
        </td>
      </tr> 
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b style="color: white">Shoulder:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Shoulder" oninput='sendButtonInput("Shoulder",value)'>
          </div>
        </td>
      </tr> 
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b style="color: white">Base:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Base" oninput='sendButtonInput("Base",value)'>
          </div>
        </td>
      </tr> 
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b style="color: white">Record:</b></td>
        <td><input type="button" id="Record" value="OFF" ontouchend='onclickButton(this)'></td>
        <td></td>
      </tr>
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b style="color: white">Play:</b></td>
        <td><input type="button" id="Play" value="OFF" ontouchend='onclickButton(this)'></td>
        <td></td>
      </tr>       
    </table>
  
    <script>
      var webSocketRobotArmInputUrl = "ws:\/\/" + window.location.hostname + "/RobotArmInput";      //Defines a url to open the web page
      var websocketRobotArmInput;
      
      function initRobotArmInputWebSocket() 
      {
        // Establishes a new WebSocket connection using the previously defined URL
        websocketRobotArmInput = new WebSocket(webSocketRobotArmInputUrl);
        // Defines an event handler for when the WebSocket connection is opened
        websocketRobotArmInput.onopen    = function(event){};
        // Defines an event handler for when the WebSocket connection is closed
        websocketRobotArmInput.onclose   = function(event){
          // If the WebSocket connection is closed, it attempts to re-establish the connection after a 2000ms delay
          setTimeout(initRobotArmInputWebSocket, 2000);};
          // Defines an event handler for when a message is received over the WebSocket connection
        websocketRobotArmInput.onmessage    = function(event)
        {
          var keyValue = event.data.split(",");
          var button = document.getElementById(keyValue[0]);    // Retrieves an element by its ID
          // Sets the value of the button to the received value
          button.value = keyValue[1];
          // Checks if the received button ID is "Record" or "Play"
          if (button.id == "Record" || button.id == "Play")
          {
            // Changes the background color of the button based on its value
            button.style.backgroundColor = (button.value == "ON" ? "green" : "red");  
            // Calls a function to enable/disable buttons and sliders based on the button's state
            enableDisableButtonsSliders(button);
          }
        };
      }
      
      function sendButtonInput(key, value) 
      {
        // Combines the key and value into a string with a comma separator
        var data = key + "," + value;
        // Sends the composed data over the WebSocket connection
        websocketRobotArmInput.send(data);
      }
      
      function onclickButton(button) 
      {
        // Toggles the value of the button from "ON" to "OFF" or vice versa
        button.value = (button.value == "ON") ? "OFF" : "ON" ;        
        // Changes the background color of the button based on its current value
        button.style.backgroundColor = (button.value == "ON" ? "green" : "red");    
        // Converts the button's value ("ON" or "OFF") to a numeric value (1 or 0)      
        var value = (button.value == "ON") ? 1 : 0 ;
        // Sends the updated button state (as a numeric value) over the WebSocket
        sendButtonInput(button.id, value);
        // Calls a function to enable/disable buttons and sliders based on the button's state
        enableDisableButtonsSliders(button);
      }
      
     function enableDisableButtonsSliders(button) {
  // Handles behavior based on the ID of the clicked button

  // If the clicked button is "Play"
  if (button.id == "Play") {
    var disabled = "auto";

    // If the value of the "Play" button is "ON"
    if (button.value == "ON") {
      disabled = "none"; // Sets the 'disabled' value to "none" to enable interaction
    }

    // Modifies pointer events of specific elements based on the 'disabled' value
    document.getElementById("Gripper").style.pointerEvents = disabled;
    document.getElementById("Elbow").style.pointerEvents = disabled;
    document.getElementById("Shoulder").style.pointerEvents = disabled;
    document.getElementById("Base").style.pointerEvents = disabled;
    document.getElementById("Record").style.pointerEvents = disabled;
  }

  // If the clicked button is "Record"
  if (button.id == "Record") {
    var disabled = "auto";

    // If the value of the "Record" button is "ON"
    if (button.value == "ON") {
      disabled = "none"; // Sets the 'disabled' value to "none" to enable interaction
    }

    // Modifies pointer events of the "Play" element based on the 'disabled' value
    document.getElementById("Play").style.pointerEvents = disabled;
  }
}

// Runs the function to initialize the WebSocket connection when the window loads
window.onload = initRobotArmInputWebSocket;

// Adds an event listener to the "mainTable" element to handle touchend events and prevent default behavior
document.getElementById("mainTable").addEventListener("touchend", function(event) {
  event.preventDefault();
});

    </script>
  </body>    
</html>
)HTMLHOMEPAGE";

void handleRoot(AsyncWebServerRequest *request) 
{
  //to serve the HTML content of the homepage when a client sends an HTTP request to the root URL.
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
  //a handler for HTTP requests that are not matched by any defined routes or endpoints on the server
    request->send(404, "text/plain", "File Not Found");
}

void onRobotArmInputWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      // Handles a WebSocket client connection event
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      sendCurrentRobotArmState(); // Sends the current state of the robot arm to the client
      break;
    case WS_EVT_DISCONNECT:
      // Handles a WebSocket client disconnection event
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      // Handles data received from WebSocket clients
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;

      // Processing text data from WebSocket frames
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        // Extracts key-value pairs from the received data
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str()); 

        int valueInt = atoi(value.c_str()); // Converts the value to an integer
        
        // Handles different keys received and performs corresponding actions
        if (key == "Record")
        {
          recordSteps = valueInt;
          if (recordSteps)
          {
            recordedSteps.clear(); // Clears previously recorded steps
            previousTimeInMilli = millis(); // Records the current time
          }
        }  
        else if (key == "Play")
        {
          playRecordedSteps = valueInt; // Sets the flag to play recorded steps
        }
        // Handles servo movements based on received key-value pairs
        else if (key == "Base")
        {
          writeServoValues(0, valueInt); // Moves the 'Base' servo to the specified value           
        } 
        // Similar handling for other servos (Shoulder, Elbow, wristroll, Wristpitch, Grip)
        // writeServoValues function is responsible for controlling servo movements
        // (The function might use servoPins and recordedSteps to control servo positions)
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      // Handles ping-pong and error events, if necessary
      break;
    default:
      break;  
  }
}


void sendCurrentRobotArmState()
{
  // Loop through each servo in the servoPins vector
  for (int i = 0; i < servoPins.size(); i++)
  {
    // Construct a message containing servo name and its current position
    // Send this message to all WebSocket clients
    wsRobotArmInput.textAll(servoPins[i].servoName + "," + servoPins[i].servo.read());
  }

  // Send the state of the recordSteps boolean variable to all clients
  wsRobotArmInput.textAll(String("Record,") + (recordSteps ? "ON" : "OFF"));

  // Send the state of the playRecordedSteps boolean variable to all clients
  wsRobotArmInput.textAll(String("Play,") + (playRecordedSteps ? "ON" : "OFF"));
}

void writeServoValues(int servoIndex, int value)
{
  // Check if recording of steps is enabled
  if (recordSteps)
  {
    RecordedStep recordedStep;

    // Check if no steps are recorded yet
    if (recordedSteps.size() == 0)
    {
      // Record the initial position of all servos if no steps are recorded yet
      for (int i = 0; i < servoPins.size(); i++)
      {
        recordedStep.servoIndex = i;
        recordedStep.value = servoPins[i].servo.read();
        recordedStep.delayInStep = 0;
        recordedSteps.push_back(recordedStep);
      }
    }

    // Record the current servo movement details
    unsigned long currentTime = millis();
    recordedStep.servoIndex = servoIndex;
    recordedStep.value = value;
    recordedStep.delayInStep = currentTime - previousTimeInMilli;
    recordedSteps.push_back(recordedStep);
    previousTimeInMilli = currentTime;
  }

  // Write the new value to the specified servo
  servoPins[servoIndex].servo.write(value);
}


void playRecordedRobotArmSteps()
{
  if (recordedSteps.size() == 0)
  {
    return; // If there are no recorded steps, exit the function
  }

  // Moves servos slowly to their initial position (first 4 steps are initial positions)
  for (int i = 0; i < 4 && playRecordedSteps; i++)
  {
    RecordedStep &recordedStep = recordedSteps[i];
    int currentServoPosition = servoPins[recordedStep.servoIndex].servo.read();

    // Gradually moves the servo to the recorded initial position
    while (currentServoPosition != recordedStep.value && playRecordedSteps)  
    {
      currentServoPosition = (currentServoPosition > recordedStep.value ? currentServoPosition - 1 : currentServoPosition + 1); 
      servoPins[recordedStep.servoIndex].servo.write(currentServoPosition);
      wsRobotArmInput.textAll(servoPins[recordedStep.servoIndex].servoName + "," + currentServoPosition);
      delay(50);
    }
  }

  delay(2000); // Delay before starting the actual steps

  // Playback the recorded steps after the initial positions
  for (int i = 4; i < recordedSteps.size() && playRecordedSteps; i++)
  {
    RecordedStep &recordedStep = recordedSteps[i];
    delay(recordedStep.delayInStep);
    servoPins[recordedStep.servoIndex].servo.write(recordedStep.value);
    wsRobotArmInput.textAll(servoPins[recordedStep.servoIndex].servoName + "," + recordedStep.value);
  }
}


void setUpPinModes()
{
  for (int i = 0; i < servoPins.size(); i++)
  {
    // Attaches each servo to its respective pin
    servoPins[i].servo.attach(servoPins[i].servoPin);

    // Sets the initial position for each servo
    servoPins[i].servo.write(servoPins[i].initialPosition);
  }
}



void setup(void) 
{
  setUpPinModes(); // Initializes servo pin modes and positions

  Serial.begin(115200); // Initializes the serial communication at a baud rate of 115200

  WiFi.softAP(ssid, password); // Sets up a Wi-Fi access point with the given credentials
  IPAddress IP = WiFi.softAPIP(); // Retrieves the IP address of the access point
  Serial.print("AP IP address: ");
  Serial.println(IP); // Prints the IP address of the access point

  // Defines handlers for specific routes/endpoints on the HTTP server
  server.on("/", HTTP_GET, handleRoot); // Handles the root endpoint
  server.onNotFound(handleNotFound); // Handles requests for unknown routes

  // Assigns an event handler for WebSocket events and adds it to the server
  wsRobotArmInput.onEvent(onRobotArmInputWebSocketEvent);
  server.addHandler(&wsRobotArmInput); // Adds WebSocket support to the server

  server.begin(); // Starts the HTTP server
  Serial.println("HTTP server started"); // Indicates that the server has started
}


void loop() 
{
  wsRobotArmInput.cleanupClients(); // Cleans up inactive WebSocket clients

  if (playRecordedSteps)
  { 
    playRecordedRobotArmSteps(); // Executes a function to play recorded robot arm steps
  }
}
