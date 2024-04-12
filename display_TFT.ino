void Dispaly (void* pvParameters)
{
  // Localiza e mostra enderecos dos sensores
  Serial.println("Localizando sensores DS18B20...");
  Serial.print("Foram encontrados ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensores.");
  
  if (!sensors.getAddress(sensor1, 0)) 
    Serial.println("Sensores nao encontrados !"); 
    
  // Mostra o endereco do sensor encontrado no barramento
  // Serial.print("Endereco sensor: ");
  // mostra_endereco_sensor(sensor1);
  
  //IMPORTANTE: A tarefa não pode terminar, deve ficar presa em um loop infinito
  while(true){
    sensors.requestTemperatures();
    tempC = sensors.getTempC(sensor1);
    if(digitalRead(4) == HIGH){
      
      tft.fillScreen(0x408E2F);
      tft.setTextSize(1);
      tft.setCursor(12, 70);
      tft.print("IP: ");
      tft.println(WiFi.localIP());
      TickType_t taskDelay = 500 / portTICK_PERIOD_MS;
      vTaskDelay(taskDelay);
      tft.fillScreen(0x408E2F);
    
    }else {
      
      tft.setTextSize(3);
      tft.setCursor(30, 10);
      tft.println("TEMP");
      
      tft.setTextSize(1);
      tft.setCursor(12, 70);
      tft.println("-- Temperatura --");

      tft.setTextSize(2);
      tft.setCursor(20, 120);
      
      tft.fillRect(42,120, 100 ,20,0x408E2F);
      tft.print("C:");
      tft.print(tempC);
      TickType_t taskDelay = 2 / portTICK_PERIOD_MS;
    vTaskDelay(taskDelay);
      
    }
  }
}
