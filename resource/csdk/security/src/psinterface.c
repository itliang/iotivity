//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "ocstack.h"
#include "logger.h"
#include "ocmalloc.h"
#include "cJSON.h"
#include "cainterface.h"
#include "secureresourcemanager.h"
#include "resourcemanager.h"
#include "srmresourcestrings.h"
#include <stdlib.h>
#include <string.h>

#define TAG  PCF("SRM-PSI")

//SVR database buffer block size
const size_t DB_FILE_SIZE_BLOCK = 1023;

// TODO Consolidate all macros in one file
#define VERIFY_SUCCESS(op, logLevel) { if (!(op)) \
            {OC_LOG((logLevel), TAG, PCF(#op " failed!!")); goto exit;} }

#define VERIFY_NON_NULL(arg, logLevel) { if (!(arg)) { OC_LOG((logLevel), \
             TAG, PCF(#arg " is NULL")); goto exit; } }
/**
 * Gets the Secure Virtual Database size.
 *
 * @param ps  pointer of OCPersistentStorage for the SVR name ("acl", "cred", "pstat" etc).
 *
 * @retval  total size of the SVR database.
 */
size_t GetSVRDatabaseSize(OCPersistentStorage* ps)
{
    size_t size = 0;
    if (!ps)
    {
        return size;
    }
    size_t bytesRead  = 0;
    char buffer[DB_FILE_SIZE_BLOCK];
    FILE* fp = ps->open(SVR_DB_FILE_NAME, "r");
    if (fp)
    {
        do
        {
            bytesRead = ps->read(buffer, 1, DB_FILE_SIZE_BLOCK, fp);
            size += bytesRead;
        } while (bytesRead > 0);
        ps->close(fp);
    }
    return size;
}

/**
 * Reads the Secure Virtual Database from PS into dynamically allocated
 * memory buffer.
 *
 * @note Caller of this method MUST use OCFree() method to release memory
 *       referenced by return value.
 *
 * @retval  reference to memory buffer containing SVR database.
 */
char * GetSVRDatabase()
{
    char * jsonStr = NULL;
    FILE * fp = NULL;
    OCPersistentStorage* ps = SRMGetPersistentStorageHandler();
    int size = GetSVRDatabaseSize(ps);
    if (0 == size)
    {
        OC_LOG (ERROR, TAG, PCF("FindSVRDatabaseSize failed"));
        return NULL;
    }

    if (ps && ps->open)
    {
        // Open default SRM database file. An app could change the path for its server.
        fp = ps->open(SVR_DB_FILE_NAME, "r");
        if (fp)
        {
            jsonStr = (char*) OCMalloc(size + 1);
            VERIFY_NON_NULL(jsonStr, FATAL);
            size_t bytesRead = ps->read(jsonStr, 1, size, fp);
            jsonStr[bytesRead] = '\0';

            OC_LOG_V(INFO, TAG, PCF("Read %d bytes from SVR database file"), bytesRead);
            ps->close(fp);
            fp = NULL;
        }
        else
        {
            OC_LOG (ERROR, TAG, PCF("Unable to open SVR database file!!"));
        }
    }

exit:
    if (ps && fp)
    {
        ps->close(fp);
    }
    return jsonStr;
}


/**
 * This method is used by a entity handlers of SVR's to update
 * SVR database.
 *
 * @param rsrcName string denoting the SVR name ("acl", "cred", "pstat" etc).
 * @param jsonObj JSON object containing the SVR contents.
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult UpdateSVRDatabase(const char* rsrcName, cJSON* jsonObj)
{
    OCStackResult ret = OC_STACK_ERROR;
    cJSON *jsonSVRDb = NULL;

    // Read SVR database from PS
    char* jsonSVRDbStr = GetSVRDatabase();
    VERIFY_NON_NULL(jsonSVRDbStr, ERROR);

    // Use cJSON_Parse to parse the existing SVR database
    jsonSVRDb = cJSON_Parse(jsonSVRDbStr);
    VERIFY_NON_NULL(jsonSVRDb, ERROR);

    OCFree(jsonSVRDbStr);
    jsonSVRDbStr = NULL;

    if (jsonObj->child )
    {
        // Create a duplicate of the JSON object which was passed.
        cJSON* jsonDuplicateObj = cJSON_Duplicate(jsonObj, 1);
        VERIFY_NON_NULL(jsonDuplicateObj, ERROR);

        cJSON* jsonObj = cJSON_GetObjectItem(jsonSVRDb, rsrcName);

        /*
         ACL, PStat & Doxm resources at least have default entries in the database but
         Cred resource may have no entries. The first cred resource entry (for provisioning tool)
         is created when the device is owned by provisioning tool and it's ownerpsk is generated.*/
        if((strcmp(rsrcName, OIC_JSON_CRED_NAME) == 0) && (!jsonObj))
        {
            // Add the fist cred object in existing SVR database json
            cJSON_AddItemToObject(jsonSVRDb, rsrcName, jsonDuplicateObj->child);
        }
        else
        {
            VERIFY_NON_NULL(jsonObj, ERROR);

            // Replace the modified json object in existing SVR database json
            cJSON_ReplaceItemInObject(jsonSVRDb, rsrcName, jsonDuplicateObj->child);
        }

        // Generate string representation of updated SVR database json object
        jsonSVRDbStr = cJSON_PrintUnformatted(jsonSVRDb);
        VERIFY_NON_NULL(jsonSVRDbStr, ERROR);

        // Update the persistent storage with new SVR database
        OCPersistentStorage* ps = SRMGetPersistentStorageHandler();
        if (ps && ps->open)
        {
            FILE* fp = ps->open(SVR_DB_FILE_NAME, "w");
            if (fp)
            {
                size_t bytesWritten = ps->write(jsonSVRDbStr, 1, strlen(jsonSVRDbStr), fp);
                if (bytesWritten == strlen(jsonSVRDbStr))
                {
                    ret = OC_STACK_OK;
                }
                OC_LOG_V(INFO, TAG, PCF("Written %d bytes into SVR database file"), bytesWritten);
                ps->close(fp);
                fp = NULL;
            }
            else
            {
                OC_LOG (ERROR, TAG, PCF("Unable to open SVR database file!! "));
            }
        }
    }

exit:
    OCFree(jsonSVRDbStr);
    cJSON_Delete(jsonSVRDb);

    return ret;
}
