#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ElegantOTA.h>

const char *ssid = "Relay_module";
const char *password = "80100000";
boolean update = false;
boolean setup_done = false;

WebServer server(80);

unsigned long ota_progress_millis = 0;

void onOTAStart()
{
    // Log when OTA has started
    Serial.println("OTA update started!");
    // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final)
{
    // Log every 1 second
    if (millis() - ota_progress_millis > 1000)
    {
        ota_progress_millis = millis();
        Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
    }
}

void onOTAEnd(bool success)
{
    // Log when OTA has finished
    if (success)
    {
        Serial.println("OTA update finished successfully!");
    }
    else
    {
        Serial.println("There was an error during OTA update!");
    }
    // <Add your own code here>
}

void ota_setup(void)
{
    if (!WiFi.softAP(ssid, password))
    {
        Serial.print(".");
        while (1)
            ;
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    server.on("/", []()
              { server.send(200, "text/plain", "Hi! This is ElegantOTA Demo."); });

    ElegantOTA.begin(&server); // Start ElegantOTA
                               // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);

    server.begin();
    Serial.println("HTTP server started");
    update = true;
}

void ota_loop(void)
{
    if (update)
    {
        if (!setup_done)
        {
            ota_setup();
            setup_done = true;
        }
        server.handleClient();
        ElegantOTA.loop();
    }
}
