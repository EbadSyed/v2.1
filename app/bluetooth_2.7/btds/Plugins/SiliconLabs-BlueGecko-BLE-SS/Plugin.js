/*
    This plug-in generates BGLIB based C project for Silicon Labs' Blue Gecko modules and SoCs.
    It enumerates all data and outputs them to a file.
*/

/*
 * Hex to String conversion
 */
 function convertToHex(str) {
    var hex = '';
	
    for(var i=0;i<str.length;i++) {
        hex += ''+str.charCodeAt(i).toString(16);
    }
    return hex;
}

/* 
 * Checks if the value is null
 */
function OmitNull(value)
{
    if( value == null )
    {
        return "";
    }
    else
    {
        return value;
    }
}

/*
 * Generate Line Break
 */
function LineBreak()
{
    return "\r\n";
}

/*
 * Return plug-in information to be shown in BTDS
 */
function GetInfo(infoObject) {
    infoObject.Name = "Silicon Labs - SS Project Generator";
    infoObject.Description = "Generates Simplicity Studio application project from standard Bluetooth Profiles and Services for Blue Gecko MCUs";
    infoObject.Author = "Silicon Labs";
    infoObject.Version = "1.0.2";
    infoObject.IsClient = false;
	
    return infoObject;
}

/*
 * Calling this runs the actual plug-in
 */
function RunPlugin(profiledata)
{
  log("Generating a project");

  // Generate GATT data base
  var output = "";
  var filename = "gatt.xml";
  
  // Create folders
  FileManager.CreateFolder("src");
  FileManager.CreateFolder("bgbuild");
  
  log("1. Generating GATT database");
  output = output + GenerateGATT(profiledata);
  FileManager.CreateFile("bgbuild/" + filename, output);
  
  log("2. Generating header files");
  GenerateHeaderFilesCode(profiledata);
  
  log("3. Generating source files");
  GenerateSourceFilesCode(profiledata);
  
  log("4. Generating main.c");
  GenerateMainFileCode(profiledata);

  log("5. Generating SS project file");
  GenerateProjectFilesCode(profiledata);
  
  log("Done. All OK!");
}

// Generate GAP service for GATT.xml
function GenerateGAPService(profiledata)
{
	log("Generating Generic Access Service");
	
	var GAPService;
	
	// Generate GATT for GAP service
	GAPService = "\t<!-- Generic Access Service -->" + LineBreak();
	GAPService = GAPService + "\t<!-- https://developer.bluetooth.org/gatt/services/Pages/ServiceViewer.aspx?u=org.bluetooth.service.generic_access.xml -->" + LineBreak();
	GAPService = GAPService + "\t<service uuid=\"1800\">" + LineBreak() + LineBreak();

	// Generate Device Name characteristic
	GAPService = GAPService + "\t\t<!-- Device Name -->" + LineBreak();
	GAPService = GAPService + "\t\t<!-- https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.device_name.xml -->" + LineBreak();
	GAPService = GAPService + "\t\t<characteristic uuid=\"2a00\" id=\"device_name\">" + LineBreak();
	GAPService = GAPService + "\t\t\t<properties read=\"true\" const=\"true\" />" + LineBreak();
  if (profiledata.GAPProperties != null)
  {
    GAPService = GAPService + "\t\t\t<value>" + profiledata.GAPProperties.DeviceName.substr(0,20) + "</value>" + "<!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
  }
  else
  {
    GAPService = GAPService + "\t\t\t<value>" + "Blue Gecko" + "</value>" + "<!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
  }
	GAPService = GAPService + "\t\t</characteristic>" + LineBreak() + LineBreak();
	
	// Generate Appearance characteristic
	GAPService = GAPService + "\t\t<!-- Appearance -->" + LineBreak();
	GAPService = GAPService + "\t\t<!-- https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml -->" + LineBreak();
	GAPService = GAPService + "\t\t<characteristic uuid=\"2a01\">" + LineBreak();
	GAPService = GAPService + "\t\t\t<properties read=\"true\" const=\"true\" />" + LineBreak();
  if (profiledata.GAPProperties != null)
  {
    GAPService = GAPService + "\t\t\t<value type=\"hex\">" + profiledata.GAPProperties.Appearance + "</value>" + "<!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
	}
  else
  {
    GAPService = GAPService + "\t\t\t<value type=\"hex\">" + "0000" + "</value>" + "<!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
  }  
  GAPService = GAPService + "\t\t</characteristic>" + LineBreak() + LineBreak();
	
	GAPService = GAPService + "\t</service>" + LineBreak() + LineBreak();
	
  return GAPService;
}

// Generate GAT service for GATT.xml
function GenerateGATTService(profiledata)
{
	log("Generating Generic Attribute Service");
	
	var GATTService;
	
	// Generate GATT for Generic Attribute Service service
	GATTService = "\t<!-- Generic Attribute Service -->" + LineBreak();
	GATTService = GATTService + "\t<!-- https://developer.bluetooth.org/gatt/services/Pages/ServiceViewer.aspx?u=org.bluetooth.service.generic_attribute.xml -->" + LineBreak();
	GATTService = GATTService + "\t<service uuid=\"1801\">" + LineBreak() + LineBreak();

	// Generate Service Changed characteristic
	GATTService = GATTService + "\t\t<!-- Service Changed -->" + LineBreak();
	GATTService = GATTService + "\t\t<!-- https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gatt.service_changed.xml -->" + LineBreak();
	GATTService = GATTService + "\t\t<characteristic uuid=\"2a05\">" + LineBreak();
	GATTService = GATTService + "\t\t\t<properties read=\"true\" indicate=\"true\" />" + LineBreak();
	GATTService = GATTService + "\t\t\t<value type=\"hex\">" + "00000000" + "</value>" + "<!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
	GATTService = GATTService + "\t\t</characteristic>" + LineBreak() + LineBreak();
	
	GATTService = GATTService + "\t</service>" + LineBreak() + LineBreak();
  return GATTService;
}

function GenerateOtaService(profiledata)
{
	log("Generating OTA Service");
	
	var OtaService;
	
	// Generate GATT for Generic Attribute Service service
	OtaService = "\t<!-- OTA Service -->" + LineBreak();
	OtaService = OtaService + "\t<service uuid=\"1d14d6ee-fd63-4fa1-bfa4-8f47b42119f0\">" + LineBreak() + LineBreak();

	// Generate Service Changed characteristic
	OtaService = OtaService + "\t\t<characteristic uuid=\"f7bf3564-fb6d-4e53-88a4-5e37e0326063\" id=\"OTA_CONTROL\">" + LineBreak();
	OtaService = OtaService + "\t\t\t<properties write=\"true\" />" + LineBreak();
	OtaService = OtaService + "\t\t\t<value length=\"1\" type=\"user\" />" + LineBreak();
	OtaService = OtaService + "\t\t</characteristic>" + LineBreak();
	
	OtaService = OtaService + "\t</service>" + LineBreak() + LineBreak();
  return OtaService;
}

// Generate service
function GenerateGATTXmlService(service, profiledata)
{
  //Log
  var debug = "";
  //Auto-generated GATT
  var output 	= "";
  
  debug = "<" + service.Name + ">";
  log(debug);

  output = output + "\t<!-- " + service.Name + "-->" + LineBreak();
  var sigService = service.Type.search("org.bluetooth.service.");
  
  if (sigService != -1)
  {
    output = output + "\t<!-- https://developer.bluetooth.org/gatt/services/Pages/ServiceViewer.aspx?u=" + service.Type + ".xml -->"  + LineBreak();
  }
  output = output + "\t<service ";

  //Generate Primary or Secondary
  if (service.Declaration == "Primary") {
    output = output + "type=\"primary=\"";
  }
  if (service.Declaration == "Secondary") {
    output = output + "type=\"secondary=\"";
  }

  // Generate UUID
  var uuid = service.UUID;
  // use 16bit UUID format
  if (uuid.substr(0,4) == "0000")
  {
      uuid = uuid.substr(4,4);
  }    
  output = output + " uuid=\"" + uuid + "\"";

  output = output + ">" + LineBreak() + LineBreak();

  // Determine whether Optional features should be generated or not - global project setting
  var globalEnableOptionalFeatures = 1;
  for (var c = 0; c < profiledata.CustomProperties.length; c++) 
  {
    var proKey = profiledata.CustomProperties.GetKey(c);
    var proValueByKey = profiledata.CustomProperties.GetValue(c);
    
    if (proKey == "DisableOptionalFeatures" && proValueByKey == '1')
    {
      globalEnableOptionalFeatures = 0;
    }
  }
  
  // Set global setting for service
  var serviceEnableOptionalFeatures = globalEnableOptionalFeatures; //Set default value from global properties

  for (var c = 0; c < service.CustomProperties.length; c++) 
  {
    var proKey = service.CustomProperties.GetKey(c);
    var proValueByKey = service.CustomProperties.GetValue(c);
    
    if (proKey == "DisableOptionalFeatures" && proValueByKey == '1')
    {
      serviceEnableOptionalFeatures = 0;
    }
  }
  
  // 2. Generate GATT characteristics
  // --------------------------------
  for( var y = 0; y < service.Characteristics.length; y++)
  {
    var characteristic = service.Characteristics[y];

 // Determine whether Optional features should be generated or not.
    var enableOptionalFeatures = serviceEnableOptionalFeatures; //Set default value from service properties
    for (var c = 0; c < characteristic.CustomProperties.length; c++) 
    {
      var proKey = characteristic.CustomProperties.GetKey(c);
      var proValueByKey = characteristic.CustomProperties.GetValue(c);
      
      if (proKey == "DisableOptionalFeatures" && proValueByKey == '1')
      {
        enableOptionalFeatures = 0;
      }
    }

    debug = "   <<" + characteristic.Name.substr(0,40) + ">>";
    log(debug);		
    
    if (service.UUID == "1800" && characteristic.UUID == "2A00")
    {
      // Generate Device Name characteristic
      output = output + "\t\t<!-- Device Name -->" + LineBreak();
      output = output + "\t\t<!-- https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.device_name.xml -->" + LineBreak();
      output = output + "\t\t<characteristic uuid=\"2a00\">" + LineBreak();
      output = output + "\t\t\t<properties read=\"true\" const=\"true\" />" + LineBreak();
      if (profiledata.GAPProperties != null)
      {
        output = output + "\t\t\t<value>" + profiledata.GAPProperties.DeviceName.substr(0,20) + "</value>" + "<!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
      }
      else
      {
        output = output + "\t\t\t<value>" + "BLE Device" + "</value>" + "<!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
      }
      output = output + "\t\t</characteristic>" + LineBreak() + LineBreak();
      continue;
    }
  
    output = output + "\t\t<!-- " + characteristic.Name + " : " + characteristic.Requirement + " -->" + LineBreak();
    var sigCharacteristic = characteristic.Type.search("org.bluetooth.characteristic.");

    if (sigCharacteristic != -1)
    {
      output = output + "\t\t<!-- https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=" + characteristic.Type + ".xml -->"  + LineBreak();
    }
    output = output + "\t\t<characteristic";
    
    //Generate UUID
    output = output + " uuid=\"" + characteristic.UUID + "\"";
    // Generate ID
    var id = service.Name + "_" + characteristic.Name;
    id = id.replace(/#|-| |:|\.|'|’|,|\(|\)|\//g, '_');
    id = id.toUpperCase();
    output = output + " id=\"" + id + "\"";
    //Close characteristic
    output = output + " >"+ LineBreak();

    // Generate characteristic Properties
    output = output + "\t\t\t<properties";
    
    // 3. Generate characteristics properties
    // --------------------------------------
    var indicateNotifyProperty = 0;
    for (var pr = 0; pr < characteristic.Properties.length; pr++)
    {
      var charProps = characteristic.Properties[pr];

      if ((charProps.Read == "Mandatory") || ((charProps.Read == "Optional") && (enableOptionalFeatures == 1) )) {
        output = output + " read=\"true\"";
      }
      if ((charProps.Write == "Mandatory") || ((charProps.Write == "Optional") && (enableOptionalFeatures == 1) )) {
        output = output + " write=\"true\"";
      }
      if ((charProps.Indicate == "Mandatory") || ((charProps.Indicate == "Optional") && (enableOptionalFeatures == 1) )) {
        output = output + " indicate=\"true\"";
        indicateNotifyProperty = 1;
      }
      if ((charProps.Notify == "Mandatory") || ((charProps.Notify == "Optional") && (enableOptionalFeatures == 1) )) {
        output = output + " notify=\"true\"";
        indicateNotifyProperty = 1;
      }
      if ((charProps.WriteWithoutResponse == "Mandatory") || ((charProps.WriteWithoutResponse == "Optional") && (enableOptionalFeatures == 1) )) {
        output = output + " write_no_response=\"true\"";
      }
    }	
    // Close properties
    output = output + " />" + LineBreak();
    
    // 4. Generate characteristics value
    // ---------------------------------
    output = output + "\t\t\t<value";
    
    // Variable for multi-field characteristics
    var length = 0;
    
    var fields = characteristic.Fields
          
    for( var z = 0; z < fields.length; z++ )
    {
      var field = fields[z];
      var utf8 = 0;
              
      if (field.Format == "utf8s") {
        utf8 = 1;
      }
      if (field.Format == "uint8") {
        length = length + 1;
      }
      if (field.Format == "uint16") {
        length = length + 2;
      }
      if (field.Format == "uint24") {
        length = length + 4;
      }
      if (field.Format == "uint40") {
        length = length + 5;
      }
      if (field.Format == "uint48") {
        length = length + 6;
      }
      if (field.Format == "8bit") {
        length = length + 1;
      }
      if (field.Format == "16bit") {
        length = length + 2;
      }
      if (field.Format == "boolean") {
        length = length + 1;
      }
      if (field.Format == "struct") {
        length = length + 512;
      }
      if (field.Format == "FLOAT") {
        length = length + 4;
      }
      
      if (z == (fields.length - 1)) 
      {
        // Add type field

        
          if (utf8 == 1) 
          {
            output = output + " type=\"utf-8\" length=\"20\"" + ">";
            output = output + "</value>" + "    <!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
          }
          else 
          {
            output = output + " type=\"hex\" length=\"" + length + "\"" + ">";
            output = output + "</value>" + "    <!-- TODO:: Set value or change type to \"user\" and add handling for it -->" + LineBreak();
          }

      }
    }  
    
    // 5. Generate characteristic descriptors
    // ---------------------------------
    for( var c = 0; c < characteristic.Descriptors.length; c++ )
    {
      var descriptor = characteristic.Descriptors[c];
      
      if (descriptor.UUID == "2902")
      {
        //Ignore descriptor for indicate and notify - it will be automatically handled by GATT xml parser
        continue;
      }
      
      output = output + "\t\t\t<descriptor uuid=\"" + descriptor.UUID + "\">" + LineBreak();
      output = output + "\t\t\t\t<properties";
      if ((descriptor.Properties.Read == "Mandatory") || ((descriptor.Properties.Read == "Optional") && (enableOptionalFeatures == 1)) )
      {
          output = output + " read=\"true\"";
      }
      if ((descriptor.Properties.Write == "Mandatory") || ((descriptor.Properties.Write == "Optional") && (enableOptionalFeatures == 1)) )
      {
          output = output + " write=\"true\"";
      }
      output = output + " />" + LineBreak();
      
      if (descriptor.UUID == "DC46F0FE81D24616B5D96ABDD796939A")
      {
        output = output + "\t\t\t\t<value type=\"user\" />" + LineBreak();
      }
      else
      {
        var fieldsDescriptor = descriptor.Fields
          
        length = 0;  
        for( var z = 0; z < fieldsDescriptor.length; z++ )
        {
          var field = fieldsDescriptor[z];
          var utf8 = 0;
                  
          if (field.Format == "utf8s") {
            utf8 = 1;
          }
          if (field.Format == "uint8") {
            length = length + 1;
          }
          if (field.Format == "uint16") {
            length = length + 2;
          }
          if (field.Format == "uint24") {
            length = length + 4;
          }
          if (field.Format == "uint40") {
            length = length + 5;
          }
          if (field.Format == "uint48") {
            length = length + 6;
          }
          if (field.Format == "8bit") {
            length = length + 1;
          }
          if (field.Format == "16bit") {
            length = length + 2;
          }
          if (field.Format == "boolean") {
            length = length + 1;
          }
          if (field.Format == "struct") {
            length = length + 512;
          }
          if (field.Format == "FLOAT") {
            length = length + 4;
          }
          
          if (z == (fieldsDescriptor.length - 1)) 
          {
            if (utf8 == 1) 
            {
              output = output + "\t\t\t\t<value type=\"utf-8\" length=\"20\"" + ">";
              output = output + "</value>" + "    <!-- TODO:: Set descriptor value or change type to \"user\" and add handling for it -->" + LineBreak();
            }
            else 
            {
              output = output + "\t\t\t\t<value type=\"hex\" length=\"" + length + "\"" + ">";
              output = output + "</value>" + "    <!-- TODO:: Set descriptor value or change type to \"user\" and add handling for it -->" + LineBreak();
            }
          }
        }  
      }
      
      output = output + "\t\t\t</descriptor>" + LineBreak();
    }  
    
    // Close characteristic
    output = output + "\t\t</characteristic>" + LineBreak() + LineBreak();
  }
  // Close service
  output = output + "\t</service>" + LineBreak() + LineBreak();
  
  return output;
}

// Generate GATT.xml
function GenerateGATT(profiledata)
{
  //Log
  var debug = "";
  //Auto-generated GATT
  var output 	= "";
	
  // 1. Generate XML header
  // ----------------------
  output = output + "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + LineBreak() + LineBreak() + "<project>" + LineBreak() + LineBreak();
  output = output + "<gatt prefix=\"GATTDB_\">" + LineBreak();
  output = output + "\t<!-- " + "ProfileName: " + profiledata.ProfileName + "-->" + LineBreak() + LineBreak();
	
  // Generate GAP, GATT service separately if it is not defined in project
  var gapService = 0;
  var gattService = 0;
  for (var x = 0; x < profiledata.Services.length; x++) 
  {
    var service = profiledata.Services[x];
    if (service.UUID == "1800")
    {
      gapService = x + 1;
    }
    else if (service.UUID == "1801")
    {
      gattService = x + 1;
    }
  }
  
  output = output + GenerateOtaService(profiledata)

  // Add GAP service
  if (gapService == 0)
  {
    output = output + GenerateGAPService(profiledata)
  }
  else
  {
    output = output + GenerateGATTXmlService(profiledata.Services[gapService - 1], profiledata);
  }
  // Add GATT service
  if (gattService == 0)
  {
    output = output + GenerateGATTService(profiledata)
  }
  else
  {
    output = output + GenerateGATTXmlService(profiledata.Services[gattService - 1], profiledata);
  }
      
  // 2. Generate GATT services
  // -------------------------
  for (var x = 0; x < profiledata.Services.length; x++) 
  {
    var service = profiledata.Services[x];
    
    if (service.UUID == "1800" || service.UUID == "1801" )
    {
      // GAP, GATT services are handled in another place
      continue;
    }
    
    output = output + GenerateGATTXmlService(service, profiledata);
    continue;
  }
  // Close configuration
  output = output + "</gatt>" + LineBreak() + "</project>" + LineBreak();

  debug = "GATT generation OK!";
  log(debug);
  return output;
}

// Generate header files code
function GenerateHeaderFilesCode(profiledata)
{
	for (var x = 0; x < profiledata.Services.length; x++) 
  {
	var service = profiledata.Services[x];
    var generateFile = 1;
   
	// Creating header file
	generateFile = 1;
    if (generateFile)
    {
      var serviceHeaderTemplate = FileManager.ReadFile("service.h.template");
      var serviceHeader = ProcessTemplate(serviceHeaderTemplate, service);
      var serviceHeaderFileName = service.Name.replace(/ /g,'_');
      serviceHeaderFileName = serviceHeaderFileName.replace(/-/g,'_');
      serviceHeaderFileName = serviceHeaderFileName + ".h";
      serviceHeaderFileName = serviceHeaderFileName.toLowerCase();
      log("Creating " + serviceHeaderFileName);
      FileManager.CreateFile("src/" + serviceHeaderFileName, serviceHeader);
    }
	}
}

// Generate source files code
function GenerateSourceFilesCode(profiledata)
{
	for (var x = 0; x < profiledata.Services.length; x++) 
  {
	var service = profiledata.Services[x];
    var generateFile = 1;


	// Creating c file
    generateFile = 1;
    if (generateFile)
    {
      var serviceSourceTemplate = FileManager.ReadFile("service.c.template");
      var serviceSource = ProcessTemplate(serviceSourceTemplate, service);
      var serviceSourceFileName = service.Name.replace(/ /g,'_');
      serviceSourceFileName = serviceSourceFileName.replace(/-/g,'_');
      serviceSourceFileName = serviceSourceFileName + ".c";
      serviceSourceFileName = serviceSourceFileName.toLowerCase();
      log("Creating "+serviceSourceFileName);
      FileManager.CreateFile("src/" + serviceSourceFileName, serviceSource);
    }
	}
}

// Generate code for source files
function GenerateMainFileCode(profiledata)
{
  // Creating main.c file
  var mainSourceTemplate = FileManager.ReadFile("main.c.template");
  var mainSource = ProcessTemplate(mainSourceTemplate, profiledata);
  var mainSourceFileName = "main.c";
  log("Creating "+mainSourceFileName);
  FileManager.CreateFile("src/" + mainSourceFileName, mainSource);
  
   // Creating ble_att_handler.c file
   var sourceTemplate = FileManager.ReadFile("ble_att_handler.c.template");
   var source = ProcessTemplate(sourceTemplate, profiledata);
   var sourceFileName = "ble_att_handler.c";
   log("Creating "+sourceFileName);
   FileManager.CreateFile("src/" + sourceFileName, source);
  
  // Creating ble_att_handler.h file
  var headerTemplate = FileManager.ReadFile("ble_att_handler.h.template");
  var header = ProcessTemplate(headerTemplate, profiledata);
  var headerFileName = "ble_att_handler.h";
  log("Creating "+headerFileName);
  FileManager.CreateFile("src/" + headerFileName, header);
  
  // Creating InitDevice.c file
  var sourceInitDeviceTemplate = FileManager.ReadFile("InitDevice.c.template");
  var sourceInitDevice = ProcessTemplate(sourceInitDeviceTemplate, profiledata);
  var sourceInitDeviceFileName = "InitDevice.c";
  log("Creating "+sourceInitDeviceFileName);
  FileManager.CreateFile("src/" + sourceInitDeviceFileName, sourceInitDevice);
  
  // Creating InitDevice.h file
  var headerInitDeviceTemplate = FileManager.ReadFile("InitDevice.h.template");
  var headerInitDevice = ProcessTemplate(headerInitDeviceTemplate, profiledata);
  var headerInitDeviceFileName = "InitDevice.h";
  log("Creating "+headerInitDeviceFileName);
  FileManager.CreateFile("src/" + headerInitDeviceFileName, headerInitDevice);
  
}

// Generate project files
function GenerateProjectFilesCode(profiledata)
{
  // Creating slsproj file
  var slsprojTemplate = FileManager.ReadFile("ble_device.slsproj.template");
  var slsprojSource = ProcessTemplate(slsprojTemplate, profiledata);
  var slsprojFileName = "ble_device.slsproj";
  log("Creating "+slsprojFileName);
  FileManager.CreateFile(slsprojFileName, slsprojSource);
    

  // Creating gatt_db.c file
  var gattDbSourceTemplate = FileManager.ReadFile("gatt_db.c.template");
  var gattDbSource = ProcessTemplate(gattDbSourceTemplate, profiledata);
  var gattDbSourceFileName = "bgbuild/gatt_db.c";
  log("Creating "+gattDbSourceFileName);
  FileManager.CreateFile(gattDbSourceFileName, gattDbSource);
  
    // Creating gatt_db.h file
  var gattDbHeaderTemplate = FileManager.ReadFile("gatt_db.h.template");
  var gattDbHeader = ProcessTemplate(gattDbHeaderTemplate, profiledata);
  var gattDbHeaderFileName = "bgbuild/gatt_db.h";
  log("Creating "+gattDbHeaderFileName);
  FileManager.CreateFile(gattDbHeaderFileName, gattDbHeader);

  
  // Creating makefile for GCC
  // var makefileTemplate = FileManager.ReadFile("Makefile.template");
  // var makefileSource = ProcessTemplate(makefileTemplate, profiledata);
  // var makefileFileName = "gcc/Makefile";
  // log("Creating "+makefileFileName);
  // FileManager.CreateFile(makefileFileName, makefileSource);
  
  // Creating belDemo.icf file
  var icfTemplate = FileManager.ReadFile("bleDemo.icf.template");
  var icfSource = ProcessTemplate(icfTemplate, profiledata);
  var icfFileName = "bleDemo.icf";
  log("Creating "+icfFileName);
  FileManager.CreateFile(icfFileName, icfSource);
}
