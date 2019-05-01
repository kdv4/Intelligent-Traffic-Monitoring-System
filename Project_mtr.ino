//Average Car=12 m/sec
//Traffic Car=5 m/sec
//Ambulance =14 m/sec

#include <ESP8266WiFi.h>
#include "FirebaseArduino.h" 
#include <SimpleTimer.h>
#define FIREBASE_HOST "intel-traffic.firebaseio.com"
#define FIREBASE_AUTH "kNJYQAm7ye5j975kE27Q1Whn1Sbk0tnzHJbUOP6K"
#define WIFI_SSID "Kishan Vaishnani"
#define WIFI_PASSWORD "thankyou"

int come_state;
int go_state;
int come_last = HIGH;
int go_last = HIGH;
int pins[6]={D0,D1,D2,D3,D4,D5};
int Timer_flag=0;
int Delay_time=0;
int flag=0;
int come_pin = D6; //digital 2
 int go_pin = D7;
int distance[3]={3000,2000,1000};//ENter in Own,Left & Straight order
SimpleTimer timer;

void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
 pinMode(come_pin, INPUT);
 pinMode(go_pin, INPUT);
  for(int i=0;i<6;i++)
 {
  pinMode(pins[i],OUTPUT);
 }
 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setInt("Signal1",30);
  Firebase.setInt("Signal2",50);
  Firebase.setInt("Signal3",10);
  Firebase.setInt("Signal4",20);
  Firebase.setInt("Signal5",60);
  Firebase.setInt("Start_Distance",-1);
  Firebase.setInt("Coming Towards",0);
  Firebase.setInt("Status_need",0);
  Firebase.setInt("Status_route",-1);
}
void firebasereconnect(){
  Serial.println("Trying to reconnect");
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

int Density()
{
 static int come_cnt = 0;
 static int go_cnt = 0;
 static int dens = 0;
 come_state = digitalRead(come_pin);
 go_state = digitalRead(go_pin); 
    
     if ( come_state != come_last) {
       come_cnt=come_cnt+1;
     }
   come_last = come_state;
   
     if ( go_state != go_last) {
       go_cnt=go_cnt+1;
      }
   go_last = go_state;
   
   dens=come_cnt-go_cnt;
   Serial.print("Density: ");
   Serial.println(dens);//Send This too firebase
   Firebase.set("Signal1",dens);
   return dens;
}

void StatuS()
{
 int i;
 int density[3];
 int min_dis=999;
 int cal_distance[3];
 int temp;
 int posi;

   if (Firebase.failed()) {
      Serial.print("setting number failed:");
      Serial.println(Firebase.error());  
      firebasereconnect();
      return;
  }
 density[1]=Firebase.getInt("Signal4");
 density[2]=Firebase.getInt("Signal5");
 Serial.print("Status Of Left: ");
 Serial.println(density[1]);
 Serial.print("Status Of Straight: ");
 Serial.println(density[2]);

 for(i=1;i<3;i++) // Find Minimum distance using formula
 {
  temp=0.625*density[i];//(va-vr/va * s) & assume that we all vehicle have length of 1 m
  cal_distance[i]=distance[i]+temp;
 }

 for(i=1;i<3;i++) //just for check output, u can remove it too
 { 
   Serial.print(i+"= ");
   Serial.println(cal_distance[i]);
 }
 
 for(i=1;i<3;i++)
 {
  if(min_dis>cal_distance[i])
    {
      min_dis=cal_distance[i]; 
      posi=i;
    }
 }

 if(posi==1)
 {
 Firebase.set("Status_route","Left");
 Firebase.set("Coming Towards",4);
  //Serial.println("Driver Should Go Straight");
 }
 else if(posi==2)
 {
  Firebase.set("Status_route","Straight");
  Firebase.set("Coming Towards",5);
  //Serial.println("Driver Should Go Right");
 }
 else
 {
  Firebase.set("Status_route","Go Where a ever you want");
 }
 Firebase.set("Status_need",0);
}

////////////////////
void Round_trip()
{
 int i=0;
 int j=0;
 int density[3];
 int temp;
 int rou=0;
 int coming=0;
 density[0]=Density();
 density[1]=Firebase.getInt("Signal2");
 density[2]=Firebase.getInt("Signal3");

 for(i=0;i<5;i+=2)
 {
  digitalWrite(pins[i],LOW);
  digitalWrite(pins[i+1],HIGH);
 }
  for(i=0;i<3;i++)
  {
   rou=ceil(0.33*density[i]);
   Serial.println(rou);
   while(density[i]>=rou && rou!=0 )
   {
    density[0]=Density();
    density[1]=Firebase.getInt("Signal2");
    density[2]=Firebase.getInt("Signal3");
    coming=Firebase.getInt("Coming Towards");
    if(i==0)
    {
     digitalWrite(pins[0],HIGH);
     digitalWrite(pins[1],LOW);
    }
    else if(i==1)
    {
     digitalWrite(pins[2],HIGH);
     digitalWrite(pins[3],LOW); 
    }
    else if(i==2)
    {
     digitalWrite(pins[4],HIGH);
     digitalWrite(pins[5],LOW);
    }
   }
     
     for(j=0;j<5;j+=2)
    {
     digitalWrite(pins[j],LOW);
     digitalWrite(pins[j+1],HIGH);
    }
   }
  }

/*void Calculate()
{
 long TimE;
 int dis,dens;
 float a,b,c;
 int coming=0;
 dis=Firebase.getInt("Start_Distance");
 a=dis/13.9;//AMbulance REach Time;
 b=a*1000;
 Serial.println(b);
 delay(500);
 coming=Firebase.getInt("Coming Towards");
 if(coming==1)
 { 
  dens=Firebase.getInt("Signal1");
 }
 else if(coming==2)
 {
  dens=Firebase.getInt("Signal2");
 }
 else if(coming==3)
 {
  dens=Firebase.getInt("Signal3");
 }
 c=(dens/15);//Car Clearance Time
 Delay_time=c*1000;
 TimE=abs(b-Delay_time);
 Serial.print("Starting Time");
 Serial.println(TimE);
 delay(500);
 Serial.print("Delay Time");
 Serial.println(Delay_time);
 delay(500);
 timer.setTimer(TimE,call_me,1);
 timer.run();
}*/

void loop() {
  // put your main code here, to run repeatedly:
 int get_status=0;
 int coming=0;
 int temp;
 temp=Density();
 Round_trip();
 get_status=Firebase.getInt("Status_need");
 if(get_status==1)
 {
   StatuS();
 }
}

