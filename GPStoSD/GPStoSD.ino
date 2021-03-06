//-------------------------------------------------------------------------------------------------------------------------------
// File     :  GPStoSD.c
// Name     :  Stefan Steiner (@stektograph) & Marco Koch (@koma5)
// Date     :  07.02.2012
// Platform :  Arduino
// Function :  Get Data from the GPS modul pmb-248 and save them to a SD-Card
//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------
//Includes
//-------------------------------------------------------------------------------------------------------------------------------
#include <SoftwareSerial.h>
#include <SD.h>
#include <String.h>
#include <avr/pgmspace.h>
//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------
//Hardware Variables 
//-------------------------------------------------------------------------------------------------------------------------------
SoftwareSerial GPS = SoftwareSerial(2,3); //SoftwareSerial(RX,TX)
const int chipSelect = 4;                 //
const int StatusLED = 12;                 //
//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------
//Flags
//-------------------------------------------------------------------------------------------------------------------------------
boolean GPSrd = true;    //is GPS receive data
boolean NMEAstr = false;  //NMEA string received
boolean usData = false;   //are usefull Datas available
//-------------------------------------------------------------------------------------------------------------------------------
//Global Variables
//-------------------------------------------------------------------------------------------------------------------------------
int NMEAlevel = 0; //showes active level of array, active means the one who will be filled now... works as FILO
char NMEA[8][100]; //can save 8 NMEA sentence 

                           //in string -> we send string for calculating
	String time;              //long  //
	String latitude;          //float //
	String lenghtitude;      //float //
	String speed;             //float // in knot -> 1 knot = 1,852 km/h
	String  date;             //int   // day, month, year
	String  altitude;         //int   // above sea level

//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------
//setup
//-------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  
  GPS.begin(4800); //serial data configuration for pmb-248, baudrate 4800
  Serial.begin(9600); //serial data configuration for computer, baudrate: 9600
  
  pinMode(StatusLED, OUTPUT); //

  for(int y = 0; y<8;y++)                       //empty NMEA[][]
     for(int x = 0; x<100;x++)                  //
		 NMEA[y][x] = ' ';                      //


}
//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------
//loop
//-------------------------------------------------------------------------------------------------------------------------------
void loop()
{ 
  if ((GPS.available()) && (GPSrd == true)) //GPS data are available and are allowed to receive
  {
		  getGPSData();
  }

  if(NMEAstr == true) //NMEA String received -> Parse
  {
	  parseNMEA(); 
	  NMEAstr == false;   //wait for next NMEA string
  }
  
  if(usData == true) //usefull Data ready to send to SD
  {
	  //Send Data
	  usData = false; //wait for next Data
  }
}
//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------
//Function     : void getGPSData(void);
//task         : get an NMEA string 
//Parameter    : void
//Return       : void
//-------------------------------------------------------------------------------------------------------------------------------
void getGPSData()
{   
	int countNMEA = 0;

	while(GPS.available()) //get all sended data not just one char
	{
        NMEA[NMEAlevel][countNMEA] = GPS.read(); //Get one char
        
        if (NMEA[NMEAlevel][countNMEA] == '$') //if a dollarsign is received -> end of NMEA sentence
        {
           NMEA[NMEAlevel][countNMEA] = '\0';  //replace dollarsign with end of string
            
           NMEAstr = true; //NMEAstr received set flag
 
		   if(NMEAlevel >8) //no Overflow
			   NMEAlevel++; //change to next NMEA level. next string can be saved
  
          for(int x = 0; x<100;x++)NMEA[NMEAlevel][x] = ' '; //empty new NMEA level
          
          NMEA[NMEAlevel][0] = '$'; //new String beginns with $
          countNMEA = 0;            
        }

        countNMEA++;                 
	}
	NMEA[NMEAlevel][countNMEA+1] = '\0'; //to avoid endless String

}
//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------
//Function     : void parseNMEA(void)
//task         : parse an NMEA string and save data to a struct
//Parameter    : void
//Return       : void
//-------------------------------------------------------------------------------------------------------------------------------
void parseNMEA()
{
   
   while(NMEAlevel >=0)//get last saved string and check every string in the NMEA array
   {
	   String NMEAstring = NMEA[NMEAlevel];//copy string
	   NMEAlevel--;

	   if(NMEAstring.indexOf("$GPRMC") !=-1) 	//check string
	   {
		   if(getdatastr(NMEAstring, 2).equals("A"))//valid data
		   {
			  time = getdatastr(NMEAstring, 1);              
			  latitude = getdatastr(NMEAstring, 3);          
			  lenghtitude = getdatastr(NMEAstring, 4);     
			  speed = getdatastr(NMEAstring, 5);            
			  date = getdatastr(NMEAstring, 7);      
		   }
		  
		  usData = true;  //usefull data found
	   }

       if(NMEAstring.indexOf("$GPGGA") != -1)
	   {
          altitude = getdatastr(NMEAstring, 7);      
		  usData = true; //usefull data found
	   }
   }
}


//-------------------------------------------------------------------------------------------------------------------------------
//Function     : String getdatastr(String NMEAsente, int wdata)
//task         : receive a string and which data want to find out and return this data in another string
//Parameter    : StringNMEAsente : string which includes a NMEA sentence
//               int wdata       :  which data you want to find out, begins with one (in $GPRMC string, that's the time)     
//Return       : String          :  returns a string with the data
//                                  or 0 in an errorcase
//-------------------------------------------------------------------------------------------------------------------------------
String getdatastr(String NMEAsente, int wdata)
{
	int startplace = 0;
	int stopplace =0;
    
	startplace = NMEAsente.indexOf(',');//index of first ','
	
	if(startplace == -1) return 0; // errorcase no comma in the string

    while(wdata > 1) //find out first place of first string
	{
		startplace = NMEAsente.indexOf(',', startplace);
		if(startplace== -1) return 0;
	}
	startplace +=1; //place on first data not at comma

	stopplace = NMEAsente.indexOf(',', startplace); //end of data
	
	if(stopplace ==-1) //last data of NMEA sentence is searched
	{
		return NMEAsente.substring(startplace);
	}
	else
	{
		return NMEAsente.substring(startplace, stopplace);
	}


}