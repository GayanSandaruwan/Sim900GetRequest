#include<SoftwareSerial.h>

#include <Wire.h>
#include <DFRobot_SIM7000.h>

SoftwareSerial client(7,8);
DFRobot_SIM7000      sim7000;


String data="{\"lat\":0,\"long\":0,\"direction\":0,\"sat\":0,\"speed\":150,\"mode\":\"string\"}";

//String readInFromSIM = "";

void setup()
{
  Serial.begin(115200);
  client.begin(19200);
  delay(500);

  initSIM();
  connectGPRS();
//  connectHTTP();
}

void loop()
{
    sendGETRequest("http://ancient-fjord-80490.herokuapp.com/device-pos-details?lat=3.652456&long=2.542663&direction=25.455&sat=12&speed=120.25&mode=self");
//    sendPOSTRequest("\"http://ancient-fjord-80490.herokuapp.com/device-pos\"",data);
//    delay(2000);
}


void connectGPRS()
{ 
  Serial.println("connection type");
  sendCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"",100);

  Serial.println("Set APN");
  sendCommand("AT+SAPBR=3,1,\"APN\",\"ppwap\"",100);//APN
  
  sendCommand("AT+SAPBR=1,1",100);

  sendCommand("AT+SAPBR=2,1",100);
}

void connectHTTP()
{
  Serial.println("Initi HTTP");
  sendCommand("AT+HTTPINIT",10);
  
  sendCommand("AT+HTTPPARA=\"CID\",1",10);

  Serial.println("server address");
//  sendCommand("AT+HTTPPARA=\"URL\",\"http://xxx.xxx.xx/Listener/\"");//Public server IP address
  sendCommand("AT+HTTPPARA=\"URL\",\"http://ancient-fjord-80490.herokuapp.com/device-pos-details?lat=3.652456&long=2.542663&direction=25.455&sat=12&speed=120.25&mode=self\"",10);//Public server address
  
  Serial.println("Set content type");
  sendCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"",10);
}


  void sendPOSTRequest(String URL, String data){

    Serial.println("Initi HTTP");
    sendCommand("AT+HTTPINIT",10);
  
    sendCommand("AT+HTTPPARA=\"CID\",1",10);

    Serial.println("server address");
    sendCommand("AT+HTTPPARA=\"URL\","+URL,10);//Public server address

    Serial.println("Set content type");
    sendCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"",10);

    Serial.println("Set Data length");
    sendCommand("AT+HTTPDATA=" + String(data.length()) + ",100000",10);

    Serial.println("Set Data");  
    sendCommand(data,200);

    Serial.println("send post");
    sendCommand("AT+HTTPACTION=1",2000);
      
//    Serial.println("content");
//    sendCommand("AT+HTTPREAD=0,600",3000);
    readResponse();

    Serial.println("Terminating");
    sendCommand("AT+HTTPTERM",100);
  }
  
void sendGETRequest(String URL){

  Serial.println("Initi HTTP");
  sendCommand("AT+HTTPINIT",100);
  
  sendCommand("AT+HTTPPARA=\"CID\",1",100);

  Serial.println("server address");
//  sendCommand("AT+HTTPPARA=\"URL\",\"http://xxx.xxx.xx/Listener/\"");//Public server IP address
  sendCommand("AT+HTTPPARA=\"URL\",\""+URL+"\"",200);//Public server address
  
//  Serial.println("Set content type");
//  sendCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"",100);
  
//  client.println("AT+HTTPACTION=0");
  sendCommand("AT+HTTPACTION=0",2000);
  Serial.println("send post");

//  sendCommand("AT+HTTPREAD=0,142",1000);

  readGetResponse();

  Serial.println("Terminating");
  sendCommand("AT+HTTPTERM",100);
  }


void readGetResponse(){

  client.println("AT+HTTPREAD=0,142");
  delay(1000);

  String resp = "|";
  while(client.available()!=0){
    
    resp += client.readStringUntil('\n')+"|";
    }
    Serial.println(resp);
    resp = getValue(resp,'|',3);
    if(!resp.equals("")){
      float destLat = getValue(resp,',',0).toFloat();
      float destLong = getValue(resp,',',1).toFloat();
      String mode = getValue(resp,',',2);
      String command = getValue(resp,',',3);
      Serial.println(destLat,10);
      Serial.println("dl : "+String(destLat) + " DLo : "+ String(destLong) + " Mode : " + mode + " Command : " + command);
      }

  }
  
void readResponse(){

  int readLen = 20;
  delay(2000);
  String resp = "|";
  while(client.available()!=0){
      resp += client.readStringUntil('\n') + '|';
    }
  Serial.println(resp);
  String respStatus = getValue(resp,'|',4);

   if(respStatus.endsWith("|")){
    respStatus = respStatus.substring(0,respStatus.length()-2);
    }
    
  String respLenStr = getValue(respStatus,',',2);

 
  Serial.println(respStatus);
  Serial.println(respLenStr);
  int respLen =0;
  if(respLenStr.equals("")){
    Serial.println("Error in the response");
    }
  else{
      respLen = respLenStr.toInt();
    }
  
   if(respLen >0){
    int itr=respLen/readLen;

    Serial.println(itr);
    
    int i=0;
    String totResponse = "";
    String asd = "";
    while(i<=itr){
       client.println("AT+HTTPREAD="+String(readLen*i)+","+String(readLen*(i+1)));
       delay(50);
       String tempResp = "";
       while(client.available()!=0){
            tempResp += client.readStringUntil('\n') + "|";
            Serial.println(tempResp);
            asd+=tempResp;
//            totResponse += client.readStringUntil('\n') + "|";
            delay(15);
          }
       totResponse += getValue(tempResp,'|',2);
       i++;
      }
    Serial.println(asd);
    }
  }
void ShowSerialData()
{
//  readInFromSIM = "";
 while(client.available()!=0)
  {
//    readInFromSIM+= client.readString();
//  Serial.println(client.readStringUntil('\n'));
    Serial.write(client.read());
  }
//  Serial.println(readInFromSIM);
}


void initSIM(){

//  turnOnBoard();

  //Set Baud rate to 19200 for reliable communication
//  sendCommand("AT+IPR=19200",200);
    sim7000.begin(client);
    Serial.println("Turn ON SIM7000......");
    if(sim7000.turnON()){                                       //Turn ON SIM7000
        Serial.println("Turn ON !");
    }

    Serial.println("Check SIM card......");
    if(sim7000.checkSIMStatus()){                               //Check SIM card
        Serial.println("SIM card READY");
    }else{
        Serial.println("SIM card ERROR, Check if you have insert SIM card and restart SIM7000");
        while(1);
    }

//    Serial.println("Set net mode......");
//    while(1){
//        if(sim7000.setNetMode(GPRS)){                             //Set net mod NB-IOT
//            Serial.println("Set GPRS mode");
//            break;
//        }else{
//            Serial.println("Fail to set mode");
//            delay(1000);
//        }
//    }

//    Serial.println("Get signal quality......");
//    delay(500);
//    int signalStrength=sim7000.checkSignalQuality();                //Check signal quality from (0-30)
//    Serial.print("signalStrength =");
//    Serial.println(signalStrength);
//    delay(500);
//
//    Serial.println("Attaching service......");
//    while(1){
//        if(sim7000.attacthService()){                           //Open the connection
//            Serial.println("Attach service");
//            break;
//        }else{
//            Serial.println("Fail to Attach service");
//            delay(1000);
//        }
//    }
  }

void sendCommand(String command,int delayTime){
  
  client.println(command);
  delay(delayTime);
//  readInFromSIM = "";
  ShowSerialData();
  }

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


// boolean checkSendCommand(String command, String check, int delayTime){
//    
//      client.println(command);
//      delay(delayTime);
//
//      readInFromSIM = "";
//      
//      boolean checkStatus = false;
//      while(client.available()!=0)
//        {
//          String readInFromSIMTemp = client.readString();
////          Serial.print(readInFromSIMTemp + "CHeck");
//
//          String responseFilt1 = getValue(readInFromSIMTemp, '\n',1);
//          String response = getValue(responseFilt1,'\n',0);
//          if(response.equals(check)){
//              checkStatus = true;
//              Serial.println("Check Success");
//            }
//         Serial.println("Response :" +response);
//        checkStatus = true;
//         readInFromSIM += readInFromSIMTemp;
//      //  Serial.write(client.read());
//        delay(50);
//        }
//        return checkStatus;
//    }

