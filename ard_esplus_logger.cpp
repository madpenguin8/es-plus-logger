//  Arduino Projects
//
//  Created by Michael Diehl on 4/9/12.
//  Copyright (c) 2012 Mike Diehl. All rights reserved.
//
// Arduino based logger for operating data from ES+ controller.
// Only operating data is currently supported.

#include <SPI.h>
#include <SD.h>
#include <stdlib.h>

#define MAX_SAMPLE_COUNT 1440

// Request for operating data in ascii <STX>oo<ETX>
byte opRequest[] = {0x02, 0x6F, 0x6F, 0x03};

// char arrays for parsed operating data
char sysPress[5];
char resPress[5];
char disTemp[5];
char resTemp[5];
char ampDraw[4];

// char array sized for the largest data.
char databuf[22];

// Previous uptime value for timer.
long previousMillis = 0;

// Timer interval in milliseconds
long interval = 60000;

// Max line count for files
unsigned int lineCount = 0;

// Log file name suffix.
unsigned int logSuffix = 0;


void setup()
{
	//Set serial port baud rate to 9600
	Serial.begin(9600);
	
	//Set serial port to 7n1
	UCSR0C = B00000100;
	
	//Setup SD card on ethernet shield  on pin 4
	SD.begin(4);
}

void loop() // run over and over
{
	// Get the current uptime
	unsigned long currentMillis = millis();
	
	// Reset previousMillis in the event of a rollover.
	if(previousMillis > currentMillis){
		previousMillis = 0;
	}
	
	// Send request if the timer expires
	if(currentMillis - previousMillis > interval) {
		// Save the uptime of the last request
		previousMillis = currentMillis;   
		requestOperatingData();
	}
	
	int count = 0;
	
	// Read in the serial data if it is available
	while(Serial.available()) {
		delay(1);
		char c = Serial.read();
		databuf[count] = c;
		count++;
	}
	
	// Parse our data if we actually have any.
	if(count > 0){
		parseOpData();
	}
}

void requestOperatingData()
{
	Serial.write(opRequest, 4);
	Serial.flush();
}


void parseOpData(){
	// Split buffer into our data arrays.
	sysPress[0] = databuf[1];
	sysPress[1] = databuf[2];
	sysPress[2] = databuf[3];
	sysPress[3] = databuf[4];
	
	resPress[0] = databuf[5];
	resPress[1] = databuf[6];
	resPress[2] = databuf[7];
	resPress[3] = databuf[8];
	
	disTemp[0] = databuf[9];
	disTemp[1] = databuf[10];
	disTemp[2] = databuf[11];
	disTemp[3] = databuf[12];
	
	resTemp[0] = databuf[13];
	resTemp[1] = databuf[14];
	resTemp[2] = databuf[15];
	resTemp[3] = databuf[16];
	
	ampDraw[0] = databuf[17];
	ampDraw[1] = databuf[18];
	ampDraw[2] = databuf[19];
	
	// Convert ascii hex to decimal values
	long x;
	
	x = strtol(sysPress, NULL, 16);
	float press1 = x / 16.00;
	
	x = strtol(resPress, NULL, 16);
	float press2 = x / 16.00;
	
	x = strtol(disTemp, NULL, 16);
	float temp1 = x / 16.00;
	
	x = strtol(resTemp, NULL, 16);
	float temp2 = x / 16.00;
	
	x = strtol(ampDraw, NULL, 16);
	float amps = x;
	
	// Create a data string for log file from values 
	String dataString = "";
	char val[8];    // string buffer

	// dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
	dataString += dtostrf(press1,6,2,val);
	dataString += ",";
	dataString += dtostrf(press2,6,2,val);
	dataString += ",";
	dataString += dtostrf(temp1,6,2,val);
	dataString += ",";
	dataString += dtostrf(temp2,6,2,val);
	dataString += ",";
	dataString += dtostrf(amps,3,0,val);

	// Increment the log suffix 
	if(lineCount > MAX_SAMPLE_COUNT){
		lineCount = 0;
		logSuffix++;
	}

	// Construct log filename i.e. ES_LOG_0.CSV, ES_LOG_1.CSV and so on
	String fileName = "ES_LOG_";
	fileName += logSuffix;
	fileName += ".CSV";

	// Open the file.
	File dataFile = SD.open(fileName, FILE_WRITE);

	// If the file is available, write to it:
	if (dataFile) {
		dataFile.println(dataString);
		dataFile.close();
	}

	// Increment line count
	lineCount++;
}

