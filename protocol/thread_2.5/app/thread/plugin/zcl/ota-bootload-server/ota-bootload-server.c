// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE

#ifdef EMBER_SCRIPTED_TEST
  #include "ota-bootload-server-test.h"
#else
  #include "thread-callbacks.h"
#endif

// TODO: do we need to support QuerySpecificFile command?
// TODO: add a callback for each block received so app can resize block?

// -----------------------------------------------------------------------------
// Constants

// See 11.11.1 (QueryJitter Parameter) from the OTA spec.
// "The server chooses the parameter value between 1 and 100 (inclusively) and
// includes it in the Image Notify Command. On receipt of the command, the
// client will examine other information (the manufacturer code and image type)
// to determine if they match its own values. If they do not, it SHALL discard
// the command and no further processing SHALL continue. If they do match then
// it will determine whether or not it SHOULD query the upgrade server. It does
// this by randomly choosing a number between 1 and 100 and comparing it to the
// value of the QueryJitter parameter received. If it is less than or equal to
// the QueryJitter value from the server, it SHALL continue with the query
// process. If not, then it SHALL discard the command and no further processing
// SHALL continue."
// I picked a value right in the middle of the range, because I am lazy.
#define DEFAULT_QUERY_JITTER 50

// See 11.13.4.2.1 (Query Next Image Request Command Field Control) from the OTA
// spec.
#define QUERY_NEXT_IMAGE_REQUEST_FIELD_CONTROL_HARDWARE_VERSION_PRESENT BIT(0)

// This is super duper arbitrary. It should be updated as needed. However, we
// should think about the MTU of the transport layer below us.
// Increased to 256 which results in a 2min OTA of a 250K file.
#define MAX_RESPONSE_BLOCK_SIZE 256

// -----------------------------------------------------------------------------
// Globals

EmberEventControl emZclOtaBootloadServerImageNotifyEventControl;

static EmberZclOtaBootloadFileSpec_t recentNextImageFileSpec;

// -----------------------------------------------------------------------------
// Private API

static uint8_t getPayloadTypeForFileSpec(const EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  // See section 11.13.3.2.1 ImageNotify Command Payload Type. This logic should
  // be communicated through the documentation for
  // emberZclOtaBootloadServerGetImageNotifyInfoCallback.
  uint8_t payloadType = 0x00;
  if (fileSpec->manufacturerCode != EMBER_ZCL_MANUFACTURER_CODE_NULL) {
    payloadType++;
    if (fileSpec->type != EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD) {
      payloadType++;
      if (fileSpec->version != EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL) {
        payloadType++;
      }
    }
  }
  return payloadType;
}

static void imageNotifyResponseHandler(EmberZclMessageStatus_t status,
                                       const EmberZclCommandContext_t *context,
                                       const EmberZclClusterOtaBootloadClientCommandImageNotifyResponse_t *response)
{
  // TODO: huh?
}

// -----------------------------------------------------------------------------
// Public API downward

void emZclOtaBootloadServerNetworkStatusCallback(EmberNetworkStatus newNetworkStatus,
                                                 EmberNetworkStatus oldNetworkStatus,
                                                 EmberJoinFailureReason reason)
{
  if (newNetworkStatus == EMBER_JOINED_NETWORK_ATTACHED) {
    emberEventControlSetActive(emZclOtaBootloadServerImageNotifyEventControl);
  } else if (newNetworkStatus == EMBER_NO_NETWORK) {
    emberEventControlSetInactive(emZclOtaBootloadServerImageNotifyEventControl);
  }
}

void emZclOtaBootloadServerImageNotifyEventHandler(void)
{
  // Get ImageNotify stuff from application.
  EmberIpv6Address address = { { 0 } };
  EmberZclOtaBootloadFileSpec_t fileSpec = emberZclOtaBootloadFileSpecNull;
  if (emberZclOtaBootloadServerGetImageNotifyInfoCallback(&address,
                                                          &fileSpec)) {
    EmberZclDestination_t destination = {
      .network = {
        .address = { { 0, } }, // filled in below
        .flags = EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG,
        .port = EMBER_COAP_PORT,
      },
      .application = {
        .data = {
          .groupId = EMBER_ZCL_GROUP_ALL_ENDPOINTS,
        },
        .type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP,
      },
    };
    destination.network.address = address;
    EmberZclClusterOtaBootloadClientCommandImageNotifyRequest_t request = {
      .payloadType = getPayloadTypeForFileSpec(&fileSpec),
      .queryJitter = DEFAULT_QUERY_JITTER,
      .manufacturerId = fileSpec.manufacturerCode,
      .imageType = fileSpec.type,
      .newFileVersion = fileSpec.version,
    };
    EmberStatus status
      = emberZclSendClusterOtaBootloadClientCommandImageNotifyRequest(&destination,
                                                                      &request,
                                                                      imageNotifyResponseHandler);
    // If we fail to send the message, then just try again later.
    (void)status;
  }

  emberEventControlSetDelayMinutes(emZclOtaBootloadServerImageNotifyEventControl,
                                   EMBER_AF_PLUGIN_OTA_BOOTLOAD_SERVER_IMAGE_NOTIFY_PERIOD_MINUTES);
}

// -------------------------------------
// Command handling

void emberZclClusterOtaBootloadServerCommandQueryNextImageRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageRequest_t *request)
{
  EmberZclOtaBootloadFileSpec_t currentFileSpec = {
    .manufacturerCode = request->manufacturerId,
    .type = request->imageType,
    .version = request->currentFileVersion,
  };
  EmberZclOtaBootloadFileSpec_t nextFileSpec = emberZclOtaBootloadFileSpecNull;
  EmberZclStatus_t zclStatus
    = emberZclOtaBootloadServerGetNextImageCallback(&context->remoteAddress,
                                                    &currentFileSpec,
                                                    &nextFileSpec);
  EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t response = {
    .status = zclStatus,
    .manufacturerId = nextFileSpec.manufacturerCode,
    .imageType = nextFileSpec.type,
    .fileVersion = nextFileSpec.version,
    .imageSize = 0, // conditionally filled in below
  };
  if (response.status == EMBER_ZCL_STATUS_SUCCESS) {
    EmberZclOtaBootloadStorageFileInfo_t storageFileInfo;
    EmberZclOtaBootloadStorageStatus_t storageStatus
      = emberZclOtaBootloadStorageFind(&nextFileSpec, &storageFileInfo);
    // TODO: handle this case more gracefully.
    assert(storageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    response.imageSize = storageFileInfo.size;

    recentNextImageFileSpec = nextFileSpec;
  }

  EmberStatus status
    = emberZclSendClusterOtaBootloadServerCommandQueryNextImageResponse(context,
                                                                        &response);
  // TODO: what if we fail to send this message?
  assert(status == EMBER_SUCCESS);
}

void emberZclClusterOtaBootloadServerCommandUpgradeEndRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndRequest_t *request)
{
  EmberZclOtaBootloadFileSpec_t fileSpec = {
    .manufacturerCode = request->manufacturerId,
    .type = request->imageType,
    .version = request->fileVersion,
  };
  uint32_t upgradeTime
    = emberZclOtaBootloadServerUpgradeEndRequestCallback(&context->remoteAddress,
                                                         &fileSpec,
                                                         request->status);
  EmberStatus status;
  if (request->status != EMBER_ZCL_STATUS_SUCCESS) {
    // See 11.13.6.4 for this default response mandate.
    status = emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  } else {
    // See 11.11.4 for discussion regarding these currentTime and upgradeTime
    // parameters.
    EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t response = {
      .manufacturerId = fileSpec.manufacturerCode,
      .imageType = fileSpec.type,
      .fileVersion = fileSpec.version,
      .currentTime = 0, // this means upgradeTime is an offset from now
      .upgradeTime = upgradeTime,
    };
    status
      = emberZclSendClusterOtaBootloadServerCommandUpgradeEndResponse(context,
                                                                      &response);
  }
  // TODO: what if we fail to send this message?
  assert(status == EMBER_SUCCESS);
}

// -------------------------------------
// Block transfer handling

void emZclOtaBootloadServerDownloadHandler(EmberCoapCode code,
                                           uint8_t *uri,
                                           EmberCoapReadOptions *options,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const EmberCoapRequestInfo *info)
{
  assert(code == EMBER_COAP_CODE_GET);
  assert(MEMCOMPARE(EM_ZCL_OTA_BOOTLOAD_UPGRADE_URI,
                    uri,
                    strlen(EM_ZCL_OTA_BOOTLOAD_UPGRADE_URI))
         == 0);

  EmberStatus status;
  EmberCoapCode responseCode;
  EmberCoapOption responseOption;
  uint8_t responseOptionCount = 0;
  uint8_t data[MAX_RESPONSE_BLOCK_SIZE];
  size_t dataSize = 0;

  EmberCoapBlockOption blockOption;
  if (!emberReadBlockOption(options, EMBER_COAP_OPTION_BLOCK2, &blockOption)) {
    responseCode = EMBER_COAP_CODE_400_BAD_REQUEST;
    goto sendResponse;
  }

  EmberZclOtaBootloadStorageFileInfo_t fileInfo;
  if (emberZclOtaBootloadStorageFind(&recentNextImageFileSpec, &fileInfo)
      != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    responseCode = EMBER_COAP_CODE_404_NOT_FOUND;
    goto sendResponse;
  }

  size_t blockOffset = emberBlockOptionOffset(&blockOption);
  if (blockOffset > fileInfo.size) {
    responseCode = EMBER_COAP_CODE_400_BAD_REQUEST;
    goto sendResponse;
  }

  size_t blockSize = emberBlockOptionSize(&blockOption);
  size_t undownloadedFileSize = fileInfo.size - blockOffset;
  dataSize = (undownloadedFileSize < blockSize
              ? undownloadedFileSize
              : blockSize);
  if (dataSize > MAX_RESPONSE_BLOCK_SIZE
      || (emberZclOtaBootloadStorageRead(&recentNextImageFileSpec,
                                         blockOffset,
                                         data,
                                         dataSize)
          != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS)) {
    responseCode = EMBER_COAP_CODE_500_INTERNAL_SERVER_ERROR;
    goto sendResponse;
  }

  responseCode = EMBER_COAP_CODE_205_CONTENT;
  emberInitCoapOption(&responseOption,
                      EMBER_COAP_OPTION_BLOCK2,
                      emberBlockOptionValue(undownloadedFileSize > blockSize,
                                            blockOption.logSize,
                                            blockOption.number));
  responseOptionCount = 1;

  sendResponse:
  status = emberCoapRespond(info,
                            responseCode,
                            &responseOption,
                            responseOptionCount,
                            data,
                            dataSize);
  // TODO: what if this fails?
  assert(status == EMBER_SUCCESS);
}
