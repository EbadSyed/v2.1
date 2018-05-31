// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#else
  #define emberAfPluginOtaBootloadClientPolicyPrint(...)
  #define emberAfPluginOtaBootloadClientPolicyPrintln(...)
  #define emberAfPluginOtaBootloadClientPolicyFlush()
  #define emberAfPluginOtaBootloadClientPolicyDebugExec(x)
  #define emberAfPluginOtaBootloadClientPolicyPrintBuffer(buffer, len, withSpace)
  #define emberAfPluginOtaBootloadClientPolicyPrintString(buffer)
#endif
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CLIENT

enum {
  EMBER_ZCL_OTA_STATIC_IP   = 0,
  EMBER_ZCL_OTA_DNS_LOOKUP  = 1,
  EMBER_ZCL_OTA_DISCOVER    = 2
};

// These values are communicated to the user through the description of the
// plugin options for this plugin.
#define INVALID_PORT 0xFFFF
#define INVALID_ENDPOINT 0xFF

uint8_t const serverName[] = EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_NAME;

#ifdef EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_DELETE_EXISTING_IMAGES_BEFORE_DOWNLOAD
static bool deletingImage = false;

static void storageDeleteCallback(EmberZclOtaBootloadStorageStatus_t storageStatus)
{
  assert(deletingImage);
  emberAfPluginOtaBootloadClientPolicyPrintln("DeleteImage COMPLETE s=0x%1X",
                                              storageStatus);
  deletingImage = false;
}
#endif

bool emberZclOtaBootloadClientSetVersionInfoCallback()
{
  bool ret = false;
  const EmberZclEndpointId_t endpoint = 1;
  const EmberZclClusterSpec_t emberZclClusterBootloadClientSpec = {
    EMBER_ZCL_ROLE_CLIENT,
    EMBER_ZCL_MANUFACTURER_CODE_NULL,
    EMBER_ZCL_CLUSTER_OTA_BOOTLOAD,
  };
  uint32_t currentFileVersion = EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_VERSION;
  if (emberZclWriteAttribute(endpoint,
                             &emberZclClusterBootloadClientSpec,
                             EMBER_ZCL_CLUSTER_OTA_BOOTLOAD_CLIENT_ATTRIBUTE_CURRENT_FILE_VERSION,
                             &currentFileVersion,
                             sizeof(currentFileVersion)) != EMBER_SUCCESS) {
    emberAfPluginOtaBootloadClientPolicyPrintln("Can't set Current File Version attribute");
    ret = true;
  }
  uint16_t manufacturerId = EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_MANUFACTURER_CODE;
  if (emberZclWriteAttribute(endpoint,
                             &emberZclClusterBootloadClientSpec,
                             EMBER_ZCL_CLUSTER_OTA_BOOTLOAD_CLIENT_ATTRIBUTE_MANUFACTURER_ID,
                             &manufacturerId,
                             sizeof(manufacturerId)) != EMBER_SUCCESS) {
    emberAfPluginOtaBootloadClientPolicyPrintln("Can't set Manufacturer ID attribute");
    ret = true;
  }
  uint16_t imageType = EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_TYPE;
  if (emberZclWriteAttribute(endpoint,
                             &emberZclClusterBootloadClientSpec,
                             EMBER_ZCL_CLUSTER_OTA_BOOTLOAD_CLIENT_ATTRIBUTE_IMAGE_TYPE_ID,
                             &imageType,
                             sizeof(imageType)) != EMBER_SUCCESS) {
    emberAfPluginOtaBootloadClientPolicyPrintln("Can't set Image Type ID attribute");
    ret = true;
  }
  emberAfPluginOtaBootloadClientPolicyPrintln("Firmware information: %2x-%2x-%4x", manufacturerId, imageType, currentFileVersion);
  return ret;
}

static bool isWildcard(uint8_t *bytes, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    if (bytes[i] != 0xFF) {
      return false;
    }
  }
  return true;
}

bool emberZclOtaBootloadClientServerHasStaticAddressCallback(EmberZclOtaBootloadClientServerInfo_t *serverInfo)
{
  #define O(x) EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  if (O(LOOKUP_TYPE) != EMBER_ZCL_OTA_STATIC_IP) {
    return false;
  }

  EmberIpv6Address address = { O(ADDRESS_PARAMETER) };
  EmberZclUid_t uid = { O(UID_PARAMETER) };

  if ( !isWildcard(address.bytes, sizeof(address.bytes)) && (O(PORT_PARAMETER) != 0xFFFF)
       && (!isWildcard(uid.bytes, sizeof(uid.bytes))) && (O(ENDPOINT_PARAMETER) != 0xFF)) {
    serverInfo->scheme = O(SCHEME_PARAMETER);
    serverInfo->name = NULL;
    serverInfo->nameLength = 0;
    serverInfo->address = address;
    serverInfo->port = O(PORT_PARAMETER);
    serverInfo->uid = uid;
    serverInfo->endpointId = O(ENDPOINT_PARAMETER);
    return true;
  }
  #undef O
  return false;
}

bool emberZclOtaBootloadClientServerHasDnsNameCallback(EmberZclOtaBootloadClientServerInfo_t *serverInfo)
{
  #define O(x) EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  if (O(LOOKUP_TYPE) != EMBER_ZCL_OTA_DNS_LOOKUP) {
    return false;
  }

  EmberIpv6Address address = { O(ADDRESS_PARAMETER) };
  EmberZclUid_t uid = { O(UID_PARAMETER) };

  if ( (O(PORT_PARAMETER) != 0xFFFF) && (!isWildcard(uid.bytes, sizeof(uid.bytes))) && (O(ENDPOINT_PARAMETER) != 0xFF)) {
    serverInfo->scheme = O(SCHEME_PARAMETER);
    serverInfo->name = serverName;
    serverInfo->nameLength = O(NAME_LENGTH);
    serverInfo->address = address;
    serverInfo->port = O(PORT_PARAMETER);
    serverInfo->uid = uid;
    serverInfo->endpointId = O(ENDPOINT_PARAMETER);
    return true;
  }
  #undef O
  return false;
}

bool emberZclOtaBootloadClientServerHasDiscByClusterId(const EmberZclClusterSpec_t *clusterSpec, EmberCoapResponseHandler responseHandler)
{
  #define O(x) EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  if (O(LOOKUP_TYPE) != EMBER_ZCL_OTA_DISCOVER) {
    return false;
  }

  #undef O
  return emberZclDiscByClusterId(clusterSpec, responseHandler);
}

static void printFileSpec(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                          bool newline)
{
  emberAfPluginOtaBootloadClientPolicyPrint(" m=0x%2X t=0x%2X v=0x%4X",
                                            fileSpec->manufacturerCode,
                                            fileSpec->type,
                                            fileSpec->version);
  if (newline) {
    emberAfPluginOtaBootloadClientPolicyPrintln("");
  }
}

#ifdef EMBER_AF_PRINT_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY
typedef struct {
  EmberZclScheme_t scheme;
  const uint8_t * const name;
} SchemeNameInfo_t;
static const SchemeNameInfo_t schemeNameInfo[] = {
  { EMBER_ZCL_SCHEME_COAP, (const uint8_t *)"COAP", },
  { EMBER_ZCL_SCHEME_COAPS, (const uint8_t *)"COAPS", },
};
static const uint8_t *getSchemeName(EmberZclScheme_t scheme)
{
  for (size_t i = 0; i < COUNTOF(schemeNameInfo); i++) {
    if (schemeNameInfo[i].scheme == scheme) {
      return schemeNameInfo[i].name;
    }
  }
  return (const uint8_t *)"?????";
}
#endif

bool emberZclOtaBootloadClientServerDiscoveredCallback(const EmberZclOtaBootloadClientServerInfo_t *serverInfo)
{
  #define O(x) EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  EmberIpv6Address address = { O(ADDRESS_PARAMETER) };
  EmberZclUid_t uid = { O(UID_PARAMETER) };
  bool accept = (serverInfo->scheme == O(SCHEME_PARAMETER)
                 && (MEMCOMPARE(serverInfo->address.bytes,
                                address.bytes,
                                sizeof(serverInfo->address.bytes))
                     || isWildcard(address.bytes, sizeof(serverInfo->address.bytes)))
                 && (serverInfo->port == O(PORT_PARAMETER)
                     || O(PORT_PARAMETER) == 0xFFFF)
                 && (MEMCOMPARE(serverInfo->uid.bytes,
                                uid.bytes,
                                sizeof(serverInfo->uid.bytes))
                     || isWildcard(uid.bytes, sizeof(serverInfo->uid.bytes)))
                 && (serverInfo->endpointId == O(ENDPOINT_PARAMETER)
                     || O(ENDPOINT_PARAMETER) == 0xFF));
  #undef O

  emberAfPluginOtaBootloadClientPolicyPrint("ServerDiscovered");
  emberAfPluginOtaBootloadClientPolicyPrint(" s=%s a=", (const char *)getSchemeName(serverInfo->scheme));
  emberAfPluginOtaBootloadClientPolicyDebugExec(emberAfPrintIpv6Address(&serverInfo->address));
  emberAfPluginOtaBootloadClientPolicyPrint(" p=%d u=", serverInfo->port);
  emberAfPluginOtaBootloadClientPolicyPrintBuffer(serverInfo->uid.bytes, EMBER_ZCL_UID_SIZE, true); // withSpace?
  emberAfPluginOtaBootloadClientPolicyPrintln("e=%d a=%c", serverInfo->endpointId, (accept ? 'y' : 'n'));

  return accept;
}

bool emberZclOtaBootloadClientGetQueryNextImageParametersCallback(EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                  EmberZclOtaBootloadHardwareVersion_t *hardwareVersion)
{
  fileSpec->manufacturerCode
    = EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_MANUFACTURER_CODE;
  fileSpec->type
    = EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_TYPE;
  fileSpec->version
    = EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_VERSION;
  *hardwareVersion
    = EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_HARDWARE_VERSION;

  emberAfPluginOtaBootloadClientPolicyPrint("GetQueryNextImageParameters");
  printFileSpec(fileSpec, false); // newline?
  emberAfPluginOtaBootloadClientPolicyPrintln(" h=0x%2X", *hardwareVersion);

  return true;
}

bool emberZclOtaBootloadClientStartDownloadCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                    bool existingFile)
{
  bool startDownload = true;

  emberAfPluginOtaBootloadClientPolicyPrint("StartDownload e=%s",
                                            (existingFile ? "true" : "false"));
  printFileSpec(fileSpec, true); // newline?

#ifdef EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_DELETE_EXISTING_IMAGES_BEFORE_DOWNLOAD
  // To make sure that there is nothing in storage that would prevent us from
  // writing a new downloaded OTA file, we delete all files here (by passing the
  // special "null" file spec value below). The existingFile bool parameter will
  // tell us if there is a file in storage matching the fileSpec parameter, but
  // it still may be the case that there are other files (with different file
  // specs) in storage. We should update the existingFile parameter to be more
  // helpful.
  EmberZclOtaBootloadStorageInfo_t info;
  emberZclOtaBootloadStorageGetInfo(&info, NULL, 0);
  if (info.fileCount > 0 && !deletingImage) {
    deletingImage = true;
    #ifdef EMBER_AF_PRINT_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY
    EmberZclOtaBootloadStorageStatus_t storageStatus =
    #endif
    emberZclOtaBootloadStorageDelete(&emberZclOtaBootloadFileSpecNull,
                                     storageDeleteCallback);
    emberAfPluginOtaBootloadClientPolicyPrintln("DeleteImage START s=0x%1X",
                                                storageStatus);
  }
  startDownload = !deletingImage;
#endif

  return startDownload;
}

EmberZclStatus_t emberZclOtaBootloadClientDownloadCompleteCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                   EmberZclStatus_t status)
{
  emberAfPluginOtaBootloadClientPolicyPrint("DownloadComplete");
  printFileSpec(fileSpec, false); // newline?
  emberAfPluginOtaBootloadClientPolicyPrintln(" s=0x%1X", status);
  return status;
}

void emberZclOtaBootloadClientPreBootloadCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  emberAfPluginOtaBootloadClientPolicyPrint("PreBootload");
  printFileSpec(fileSpec, true); // newline?
}
