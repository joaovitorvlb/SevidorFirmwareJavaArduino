#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

WiFiMulti wifiMulti;

#define WIFISSID "Joao Vitor"
#define PASSWORD "mj110032"

/*
 * Protótipos de Função
*/
void prvSetupHardware( void );
void vPrintString( const char *pcString);
void vPrintStringAndNumber( const char *pcString, uint32_t ulValue );
void vPrintTwoStrings(const char *pcString1, const char *pcString2);

void vTask1( void *pvParameters );
void vTask2( void *pvParameters );
#define CORE_0 0
#define CORE_1 1
/*
 * Global
*/
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
const char *pcTextForTask1 = "Task 1 is running\r\n";
const char *pcTextForTask2 = "Task 2 is running\r\n";
//OU tskNO_AFFINITY

static uint32_t ulIdleCycleCount = 0UL;

void setup()
{
    prvSetupHardware();

    xTaskCreatePinnedToCore( vTask1, "Task 1", configMINIMAL_STACK_SIZE+8000, (void*)pcTextForTask1, 2, NULL, CORE_0 );
    xTaskCreatePinnedToCore( vTask2, "Task 2", configMINIMAL_STACK_SIZE+5000, (void*)pcTextForTask2, 2, NULL, CORE_1 );
}

void loop()
{
    vTaskDelay( 100 / portTICK_PERIOD_MS );
}

void prvSetupHardware( void )
{
    Serial.begin( 9600 );
    for(uint8_t t = 4; t > 0; t--)
    {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(500);
    }
    wifiMulti.addAP( WIFISSID, PASSWORD );
}

void vPrintString( const char *pcString )
{
    taskENTER_CRITICAL( &myMutex );
    {
        Serial.println( (char*)pcString );
    }
    taskEXIT_CRITICAL( &myMutex );
}

void vPrintStringAndNumber( const char *pcString, uint32_t ulValue )
{
    taskENTER_CRITICAL( &myMutex );
    {
        char buffer [50];
        sprintf( buffer, "%s %lu\r\n", pcString, ulValue );
        Serial.println( (char*)buffer );
    }
    taskEXIT_CRITICAL( &myMutex );
}

void vPrintTwoStrings(const char *pcString1, const char *pcString2)
{
    taskENTER_CRITICAL( &myMutex  );
    {
        char buffer [50];
        sprintf(buffer, "%s %s\r\n", pcString1, pcString2);
        Serial.println( (char*)buffer );
    }
    taskEXIT_CRITICAL( &myMutex );
}

void vTask1( void *pvParameters )
{
vPrintString( "Task1 Init...\n" );

for( ;; )
{
    if( (wifiMulti.run() == WL_CONNECTED) )
    {
        HTTPClient http;

        vPrintString( "[HTTP] begin...\n" );

        //
        //http.begin( "http://192.168.0.25:8080/Java-ee_ex/eu.txt");
        http.begin( "http://192.168.0.25/site/query_insert_db.php");
        http.addHeader( "Content-Type", "application/x-www-form-urlencoded" );
        http.addHeader( "Connection", "close" );

        vPrintString( "[HTTP] POST...\n" );
        int httpCode = http.POST( "nome=teste&valor=120&timestamp=1525871035" );
        // httpCode will be negative on error
        if( httpCode > 0 )
        {
            // HTTP header has been send and Server response header has been handled
            vPrintStringAndNumber("[HTTP] POST... code: \n", httpCode);

            // file found at server
            if( httpCode == HTTP_CODE_OK )         //Code 200
            {
                String payload = http.getString();
                vPrintString(payload.c_str());
            }
        }
        else
        {
            vPrintTwoStrings( "[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str() );
        }

        http.end();
    }

    vTaskDelay( 10000 / portTICK_PERIOD_MS );
    }
}

void vTask2( void *pvParameters )
{
    char *pcTaskName;
    pcTaskName = ( char * ) pvParameters;
    volatile uint32_t ul;

    vPrintString( "Task2 Init...\n" );
    for( ;; )
    {
        vPrintString( pcTaskName );
        vTaskDelay( 500 / portTICK_PERIOD_MS );
    }
}
