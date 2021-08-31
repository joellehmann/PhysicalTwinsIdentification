  //######################################################################
  //Request Number of Metadata ###########################################
  //######################################################################

  k = 0;
  if (i2cCount > 1)
  {
    colreq = true;
  }

  if (i2cCount > 0)
  {
    while (k < i2cCount)
    {
      Wire.beginTransmission(iAttDevs[k]);
      Wire.write("reqMetaCount\n");
      Wire.endTransmission();
      delay(100);
      Wire.requestFrom(iAttDevs[k], 1);
      while (Wire.available()) { 
        c = Wire.read();
        metaCount = c;      
      }

      while (j < (int)metaCount-48)
        {
          Wire.beginTransmission(iAttDevs[k]);
          Wire.write((String(j)+"\n").c_str());
          Wire.endTransmission();  
          delay(100);
          Wire.requestFrom(iAttDevs[k], 23);
          while (Wire.available()) { 
          c = Wire.read();
          if (c != 255 && !bEOS) {   
            sMeta += c;     
          } else {
            bEOS = true;
          }
        }
        Serial.println("");
        Serial.print("KVP1 ");
        Serial.print(j);
        Serial.print(" -> ");
        META[j].key = sMeta.substring(0,sMeta.indexOf(":"));
        META[j].value = sMeta.substring(sMeta.indexOf(":")+1,sMeta.length());
        sMeta = "";
        bEOS = false;

        Serial.print(META[j].key);
        Serial.print(":");
        Serial.print(META[j].value);
        Serial.println("");
        j++;
        }
      j = 0;

      JsonObject devid = doc.createNestedObject(META[0].value);
      j = 1;
      
      while (j < (int)metaCount-48) 
      {
        devid[META[j].key] = META[j].value;
        j++;
      }

      serializeJson(doc, buffer);
      j = 0;
      doc.clear();

      strcat(jsonString,buffer); 
      memset(buffer, 0, sizeof buffer);

      if (colreq && k < i2cCount - 1)
      {
        strcat(jsonString,",");
      }
      k++;
    }
  }

  
  
  strcat(jsonString,R"=====(},
      "features": {
      }
    }
  )=====");

  Serial.println(jsonString);
  httpResponse = DigitalTwinInstance.createDittoThing(jsonString);

  memset(jsonString, 0, sizeof jsonString);
