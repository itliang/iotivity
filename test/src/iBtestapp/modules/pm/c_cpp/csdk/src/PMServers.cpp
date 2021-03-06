/******************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      LICENSE-2.0" target="_blank">http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 ******************************************************************/
///////////////////////////////////////////////////////////////////////
//NOTE :  This sample server is generated based on ocserverbasicops.cpp
///////////////////////////////////////////////////////////////////////
#include "../include/PMCsdkAppHelper.h"

#define TAG "SAMPLE_JUSTWORKS"
#define LED_RESOURCE_URI "/a/led/"
#define RESOURCE_TYPE_LED "core.led"
#define KEY_URI "createduri"
int gQuitFlag = 0;
const char* g_ledResURI_01 = "/a/led/0";
const char* g_ledResURI_02 = "/a/led/1";

/* Structure to represent a LED resource */
typedef struct LEDRESOURCE
{
    OCResourceHandle handle;bool state;
    int power;
} LEDResource;

static LEDResource LED;
// This variable determines instance number of the LED resource.
// Used by POST method to create a new instance of LED resource.
static int gCurrLedInstance = 0;
#define SAMPLE_MAX_NUM_POST_INSTANCE  2
static LEDResource gLedInstance[SAMPLE_MAX_NUM_POST_INSTANCE];

char *gResourceUri = (char *) "/a/led";

//Secure Virtual Resource database for Iotivity Server
//It contains Server's Identity and the PSK credentials
//of other devices which the server trusts
static char CRED_FILE1[] = "oic_svr_db_server_justworks.dat";
static char CRED_FILE2[] = "oic_svr_db_server.dat"; // Direct Pairing Not Supported
static char CRED_FILE3[] = "oic_svr_db_server_randompin.dat";
static char CRED_FILE4[] = "preconfig_server_1.dat";
static char CRED_FILE5[] = "oic_svr_db_server_mvjustworks.dat";
static int gCurrentIndex = 0;

/* Function that creates a new LED resource by calling the
 * OCCreateResource() method.
 */
int createLEDResource(char *uri, LEDResource *ledResource, bool resourceState, int resourcePower);

/* This method converts the payload to JSON format */
OCRepPayload* constructResponse(OCEntityHandlerRequest *ehRequest);

/* Following methods process the PUT, GET, POST
 * requests
 */
OCEntityHandlerResult ProcessGetRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload **payload);
OCEntityHandlerResult ProcessPutRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload **payload);
OCEntityHandlerResult ProcessPostRequest(OCEntityHandlerRequest *ehRequest,
        OCEntityHandlerResponse *response, OCRepPayload **payload);

/* Entity Handler callback functions */
OCEntityHandlerResult
OCEntityHandlerCb(OCEntityHandlerFlag flag, OCEntityHandlerRequest *entityHandlerRequest,
        void* callbackParam);

void duplicateString(char ** targetString, std::string sourceString)
{
    *targetString = new char[sourceString.length() + 1];
    strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}

const char *getResult(OCStackResult result)
{
    switch (result)
    {
        case OC_STACK_OK:
            return "OC_STACK_OK";
        case OC_STACK_RESOURCE_CREATED:
            return "OC_STACK_RESOURCE_CREATED";
        case OC_STACK_RESOURCE_DELETED:
            return "OC_STACK_RESOURCE_DELETED";
        case OC_STACK_INVALID_URI:
            return "OC_STACK_INVALID_URI";
        case OC_STACK_INVALID_QUERY:
            return "OC_STACK_INVALID_QUERY";
        case OC_STACK_INVALID_IP:
            return "OC_STACK_INVALID_IP";
        case OC_STACK_INVALID_PORT:
            return "OC_STACK_INVALID_PORT";
        case OC_STACK_INVALID_CALLBACK:
            return "OC_STACK_INVALID_CALLBACK";
        case OC_STACK_INVALID_METHOD:
            return "OC_STACK_INVALID_METHOD";
        case OC_STACK_NO_MEMORY:
            return "OC_STACK_NO_MEMORY";
        case OC_STACK_COMM_ERROR:
            return "OC_STACK_COMM_ERROR";
        case OC_STACK_INVALID_PARAM:
            return "OC_STACK_INVALID_PARAM";
        case OC_STACK_NOTIMPL:
            return "OC_STACK_NOTIMPL";
        case OC_STACK_NO_RESOURCE:
            return "OC_STACK_NO_RESOURCE";
        case OC_STACK_RESOURCE_ERROR:
            return "OC_STACK_RESOURCE_ERROR";
        case OC_STACK_SLOW_RESOURCE:
            return "OC_STACK_SLOW_RESOURCE";
        case OC_STACK_NO_OBSERVERS:
            return "OC_STACK_NO_OBSERVERS";
#ifdef WITH_PRESENCE
        case OC_STACK_PRESENCE_STOPPED:
            return "OC_STACK_PRESENCE_STOPPED";
#endif
        case OC_STACK_ERROR:
            return "OC_STACK_ERROR";
        default:
            return "UNKNOWN";
    }
}

OCRepPayload* getPayload(const char* uri, int64_t power, bool state)
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if (!payload)
    {
        IOTIVITYTEST_LOG(ERROR, "Failed to allocate Payload");
        return NULL;
    }

    OCRepPayloadSetUri(payload, uri);
    OCRepPayloadSetPropBool(payload, "state", state);
    OCRepPayloadSetPropInt(payload, "power", power);

    return payload;
}

//This function takes the request as an input and returns the response
OCRepPayload* constructResponse(OCEntityHandlerRequest *ehRequest)
{
    if (ehRequest->payload && ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
    {
        IOTIVITYTEST_LOG(ERROR, "Incoming payload not a representation");
        return NULL;
    }

    OCRepPayload* input = (OCRepPayload*) (ehRequest->payload);

    LEDResource *currLEDResource = &LED;

    if (ehRequest->resource == gLedInstance[0].handle)
    {
        currLEDResource = &gLedInstance[0];
        strcpy(gResourceUri, g_ledResURI_01);
    }
    else if (ehRequest->resource == gLedInstance[1].handle)
    {
        currLEDResource = &gLedInstance[1];
        strcpy(gResourceUri, g_ledResURI_02);
    }

    if (OC_REST_PUT == ehRequest->method)
    {
        // Get pointer to query
        int64_t pow;
        if (OCRepPayloadGetPropInt(input, "power", &pow))
        {
            currLEDResource->power = pow;
        }

        bool state;
        if (OCRepPayloadGetPropBool(input, "state", &state))
        {
            currLEDResource->state = state;
        }
    }

    return getPayload(gResourceUri, currLEDResource->power, currLEDResource->state);
}

OCEntityHandlerResult ProcessGetRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload **payload)
{
    OCEntityHandlerResult ehResult;

    OCRepPayload *getResp = constructResponse(ehRequest);

    if (getResp)
    {
        *payload = getResp;
        ehResult = OC_EH_OK;
    }
    else
    {
        ehResult = OC_EH_ERROR;
    }

    return ehResult;
}

OCEntityHandlerResult ProcessPutRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload **payload)
{
    OCEntityHandlerResult ehResult;

    OCRepPayload *putResp = constructResponse(ehRequest);

    if (putResp)
    {
        *payload = putResp;
        ehResult = OC_EH_OK;
    }
    else
    {
        ehResult = OC_EH_ERROR;
    }

    return ehResult;
}

OCEntityHandlerResult ProcessPostRequest(OCEntityHandlerRequest *ehRequest,
        OCEntityHandlerResponse *response, OCRepPayload **payload)
{
    OCRepPayload *respPLPost_led = NULL;
    OCEntityHandlerResult ehResult = OC_EH_OK;

    /*
     * The entity handler determines how to process a POST request.
     * Per the REST paradigm, POST can also be used to update representation of existing
     * resource or create a new resource.
     * In the sample below, if the POST is for /a/led then a new instance of the LED
     * resource is created with default representation (if representation is included in
     * POST payload it can be used as initial values) as long as the instance is
     * lesser than max new instance count. Once max instance count is reached, POST on
     * /a/led updated the representation of /a/led (just like PUT)
     */

    if (ehRequest->resource == LED.handle)
    {
        if (gCurrLedInstance < SAMPLE_MAX_NUM_POST_INSTANCE)
        {
            // Create new LED instance
            char newLedUri[15] = LED_RESOURCE_URI;
            int newLedUriLength = strlen(newLedUri);
            snprintf(newLedUri + newLedUriLength, sizeof(newLedUri) - newLedUriLength, "%d",
                    gCurrLedInstance);

            respPLPost_led = OCRepPayloadCreate();
            OCRepPayloadSetUri(respPLPost_led, gResourceUri);
            OCRepPayloadSetPropString(respPLPost_led, KEY_URI, newLedUri);

            if (0 == createLEDResource(newLedUri, &gLedInstance[gCurrLedInstance], false, 0))
            {
                IOTIVITYTEST_LOG(INFO, "Created new LED instance");
                gLedInstance[gCurrLedInstance].state = 0;
                gLedInstance[gCurrLedInstance].power = 0;
                gCurrLedInstance++;
                strncpy((char *) response->resourceUri, newLedUri, MAX_URI_LENGTH);
                ehResult = OC_EH_RESOURCE_CREATED;
            }
        }
        else
        {
            respPLPost_led = constructResponse(ehRequest);
        }
    }
    else
    {
        for (int i = 0; i < SAMPLE_MAX_NUM_POST_INSTANCE; i++)
        {
            if (ehRequest->resource == gLedInstance[i].handle)
            {
                if (i == 0)
                {
                    respPLPost_led = constructResponse(ehRequest);
                    break;
                }
                else if (i == 1)
                {
                    respPLPost_led = constructResponse(ehRequest);
                }
            }
        }
    }

    if (respPLPost_led != NULL)
    {
        *payload = respPLPost_led;
        ehResult = OC_EH_OK;
    }
    else
    {
        IOTIVITYTEST_LOG(INFO, "Payload was NULL");
        ehResult = OC_EH_ERROR;
    }

    return ehResult;
}

OCEntityHandlerResult OCEntityHandlerCb(OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest, void* callbackParam)
{
    IOTIVITYTEST_LOG(INFO, "Inside entity handler - flags: 0x%x", flag);
    (void) callbackParam;
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    OCEntityHandlerResponse response;
    memset(&response, 0, sizeof(response));

    // Validate pointer
    if (!entityHandlerRequest)
    {
        IOTIVITYTEST_LOG(ERROR, "Invalid request pointer");
        return OC_EH_ERROR;
    }

    OCRepPayload* payload = NULL;

    if (flag & OC_REQUEST_FLAG)
    {
        IOTIVITYTEST_LOG(INFO, "Flag includes OC_REQUEST_FLAG");
        if (entityHandlerRequest)
        {
            if (OC_REST_GET == entityHandlerRequest->method)
            {
                IOTIVITYTEST_LOG(INFO, "Received OC_REST_GET from client");
                ehResult = ProcessGetRequest(entityHandlerRequest, &payload);
            }
            else if (OC_REST_PUT == entityHandlerRequest->method)
            {
                IOTIVITYTEST_LOG(INFO, "Received OC_REST_PUT from client");
                ehResult = ProcessPutRequest(entityHandlerRequest, &payload);
            }
            else if (OC_REST_POST == entityHandlerRequest->method)
            {
                IOTIVITYTEST_LOG(INFO, "Received OC_REST_POST from client");
                ehResult = ProcessPostRequest(entityHandlerRequest, &response, &payload);
            }
            else
            {
                IOTIVITYTEST_LOG(INFO, "Received unsupported method %d from client",
                        entityHandlerRequest->method);
                ehResult = OC_EH_ERROR;
            }

            if (ehResult == OC_EH_OK && ehResult != OC_EH_FORBIDDEN)
            {
                // Format the response.  Note this requires some info about the request
                response.requestHandle = entityHandlerRequest->requestHandle;
                response.resourceHandle = entityHandlerRequest->resource;
                response.ehResult = ehResult;
                response.payload = (OCPayload*) (payload);
                response.numSendVendorSpecificHeaderOptions = 0;
                memset(response.sendVendorSpecificHeaderOptions, 0,
                        sizeof(response.sendVendorSpecificHeaderOptions));
                memset(response.resourceUri, 0, sizeof(response.resourceUri));
                // Indicate that response is NOT in a persistent buffer
                response.persistentBufferFlag = 0;

                // Send the response
                if (OCDoResponse(&response) != OC_STACK_OK)
                {
                    IOTIVITYTEST_LOG(ERROR, "Error sending response");
                    ehResult = OC_EH_ERROR;
                }
            }
        }
    }

    OCPayloadDestroy(response.payload);
    return ehResult;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

FILE* server_fopen(const char *path, const char *mode)
{
    //(void) path;
    if (0 == strcmp(path, OC_SECURITY_DB_DAT_FILE_NAME))
    {
        if (gCurrentIndex == 1)
        {
            return fopen(CRED_FILE1, mode);
        }
        else if (gCurrentIndex == 2)
        {
            return fopen(CRED_FILE2, mode);
        }
        else if (gCurrentIndex == 3)
        {
            return fopen(CRED_FILE3, mode);
        }
        else if (gCurrentIndex == 4)
        {
            return fopen(CRED_FILE4, mode);
        }
        else if (gCurrentIndex == 5)
        {
            return fopen(CRED_FILE5, mode);
        }
    }
    else
    {
        return fopen(path, mode);
    }
}

void GeneratePinCB(char* pin, size_t pinSize)
{
    if (NULL == pin || pinSize <= 0)
    {
        IOTIVITYTEST_LOG(INFO, "Invalid PIN");
        return;
    }

    IOTIVITYTEST_LOG(INFO, "============================");
    IOTIVITYTEST_LOG(INFO, "    PIN CODE : [%s]", pin);
    IOTIVITYTEST_LOG(INFO, "============================");
}

OCStackResult displayNumCB(void * ctx, uint8_t mutualVerifNum[MUTUAL_VERIF_NUM_LEN])
{
    IOTIVITYTEST_LOG(DEBUG, "[Test Server] displayNumCB IN");
    IOTIVITYTEST_LOG(DEBUG, "[Test Server] ############ mutualVerifNum ############");

    for(int i = 0; i< MUTUAL_VERIF_NUM_LEN ; i++)
    {
        IOTIVITYTEST_LOG(DEBUG, "[Test Server] %02X ", mutualVerifNum[i] );
    }

    IOTIVITYTEST_LOG(DEBUG, "[Test Server] ############ mutualVerifNum ############");
    IOTIVITYTEST_LOG(DEBUG, "[Test Server] displayNumCB OUT");
    return OC_STACK_OK;
}

OCStackResult confirmNumCB(void * ctx)
{
    IOTIVITYTEST_LOG(DEBUG, "[Test Server] confirmNumCB IN");

    for (;;)
    {
        int serverConfirm;
        printf("   > Press 1 for confirmation\n");
        printf("   > Press 0 otherwise\n");
        for (int ret=0; 1!=ret; )
        {
            ret = scanf("%d", &serverConfirm);
            for (; 0x20<=getchar(); );  // for removing overflow garbage
                                        // '0x20<=code' is character region
        }

        if (1 == serverConfirm)
        {
            break;
        }
        else if (0 == serverConfirm)
        {
            return OC_STACK_ERROR;
        }

        IOTIVITYTEST_LOG(ERROR, "[Test Server] Entered Wrong Number. Please Enter Again");
    }

    IOTIVITYTEST_LOG(DEBUG, "[Test Server] confirmNumCB OUT");
    return OC_STACK_OK;
}

int startServer(int serverType)
{
    struct timespec timeout;

    gCurrentIndex = serverType;

    if (gCurrentIndex == 1)
        {
            //setup
            removeAllResFile(JUSTWORK1);
            CommonUtil::waitInSecond(DELAY_LONG);
            copyAllResFile(JUSTWORK1);
            CommonUtil::waitInSecond(DELAY_LONG);
        }
        else if (gCurrentIndex == 2)
        {
            //setup
            removeAllResFile(JUSTWORK2);
            CommonUtil::waitInSecond(DELAY_LONG);
            copyAllResFile(JUSTWORK2);
            CommonUtil::waitInSecond(DELAY_LONG);
        }
        else if (gCurrentIndex == 3)
        {
            //setup
            removeAllResFile(RANDOMPIN);
            CommonUtil::waitInSecond(DELAY_LONG);
            copyAllResFile(RANDOMPIN);
            CommonUtil::waitInSecond(DELAY_LONG);
        }
        else if (gCurrentIndex == 4)
        {
            //setup
            removeAllResFile(PRECONFIGPIN1);
            CommonUtil::waitInSecond(DELAY_LONG);
            copyAllResFile(PRECONFIGPIN1);
            CommonUtil::waitInSecond(DELAY_LONG);
        }
        else if (gCurrentIndex == 5)
        {
            //setup
            removeAllResFile(MVJUSTWORK);
            CommonUtil::waitInSecond(DELAY_LONG);
            copyAllResFile(MVJUSTWORK);
            CommonUtil::waitInSecond(DELAY_LONG);

            // Set callbacks for verification
            SetDisplayNumCB(NULL, (DisplayNumCallback)displayNumCB);
            SetUserConfirmCB(NULL, (UserConfirmCallback)confirmNumCB);

            // Set Verification Option for ownership transfer
            // Currently, BOTH display AND confirm
            SetVerifyOption((VerifyOptionBitmask_t)(DISPLAY_NUM | USER_CONFIRM));
        }

    IOTIVITYTEST_LOG(DEBUG, "[Server %d] OCServer is starting...", gCurrentIndex);

    // Initialize Persistent Storage for SVR database
    OCPersistentStorage ps =
    { server_fopen, fread, fwrite, fclose, unlink };

    OCRegisterPersistentStorageHandler(&ps);

    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        IOTIVITYTEST_LOG(ERROR, "OCStack init error");
        return 0;
    }

    if (gCurrentIndex == 3)
    {
        SetGeneratePinCB((GeneratePinCallback)&GeneratePinCB);
    }

    /*
     * Declare and create the example resource: LED
     */
    createLEDResource(gResourceUri, &LED, false, 0);

    timeout.tv_sec = 0;
    timeout.tv_nsec = 100000000L;

    // Break from loop with Ctrl-C
    IOTIVITYTEST_LOG(INFO, "Entering ocserver main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            IOTIVITYTEST_LOG(ERROR, "OCStack process error");
            return 0;
        }
        nanosleep(&timeout, NULL);
    }

    IOTIVITYTEST_LOG(INFO, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        IOTIVITYTEST_LOG(ERROR, "OCStack process error");
    }

    return 0;
}

int createLEDResource(char *uri, LEDResource *ledResource, bool resourceState, int resourcePower)
{
    if (!uri)
    {
        IOTIVITYTEST_LOG(ERROR, "Resource URI cannot be NULL");
        return -1;
    }

    ledResource->state = resourceState;
    ledResource->power = resourcePower;
    OCStackResult res = OCCreateResource(&(ledResource->handle), RESOURCE_TYPE_LED,
    OC_RSRVD_INTERFACE_DEFAULT, uri, OCEntityHandlerCb, NULL,
            OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);
    IOTIVITYTEST_LOG(INFO, "Created LED resource with result: %s", getResult(res));

    return 0;
}
