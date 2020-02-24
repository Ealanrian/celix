/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "celix_properties.h"

#include "pubsub_wire_protocol_impl.h"
#include "pubsub_wire_protocol_common.h"

#define NETSTRING_ERROR_TOO_LONG     -1
#define NETSTRING_ERROR_NO_COLON     -2
#define NETSTRING_ERROR_TOO_SHORT    -3
#define NETSTRING_ERROR_NO_COMMA     -4
#define NETSTRING_ERROR_LEADING_ZERO -5
#define NETSTRING_ERROR_NO_LENGTH    -6

struct pubsub_protocol_wire_v1 {
    celix_bundle_context_t *bundle_context;
};

static celix_status_t pubsubProtocol_createNetstring(const char* string, char** netstringOut);
static int pubsubProtocol_parseNetstring(char *buffer, size_t buffer_length,
                                  char **netstring_start, size_t *netstring_length);

celix_status_t pubsubProtocol_create(celix_bundle_context_t *context, pubsub_protocol_wire_v1_t **protocol) {
    celix_status_t status = CELIX_SUCCESS;

    *protocol = calloc(1, sizeof(**protocol));

    if (!*protocol) {
        status = CELIX_ENOMEM;
    }
    else {
        (*protocol)->bundle_context = context;
    }

    return status;
}

celix_status_t pubsubProtocol_destroy(pubsub_protocol_wire_v1_t* protocol) {
    celix_status_t status = CELIX_SUCCESS;

    free(protocol);

    return status;
}

celix_status_t pubsubProtocol_getSyncHeader(void* handle, void *syncHeader) {
    for (int i = 0; i < 5; ++i) {
        ((char *) syncHeader)[i] = '\0';
    }
    writeInt(syncHeader, 0, PROTOCOL_WIRE_SYNC);

    return CELIX_SUCCESS;
}

celix_status_t pubsubProtocol_encodeHeader(void *handle, pubsub_protocol_message_t *message, void **outBuffer, size_t *outLength) {
    celix_status_t status = CELIX_SUCCESS;

    *outBuffer = calloc(1, 24);
    if (*outBuffer == NULL) {
        status = CELIX_ENOMEM;
    } else {
        int idx = 0;
        idx = writeInt(*outBuffer, idx, PROTOCOL_WIRE_SYNC);
        idx = writeInt(*outBuffer, idx, PROTOCOL_WIRE_ENVELOPE_VERSION);
        idx = writeInt(*outBuffer, idx, message->header.msgId);
        idx = writeShort(*outBuffer, idx, message->header.msgMajorVersion);
        idx = writeShort(*outBuffer, idx, message->header.msgMinorVersion);
        idx = writeInt(*outBuffer, idx, message->header.payloadSize);
        idx = writeInt(*outBuffer, idx, message->header.metadataSize);

        *outLength = idx;
    }

    return status;
}

celix_status_t pubsubProtocol_encodePayload(void *handle, pubsub_protocol_message_t *message, void **outBuffer, size_t *outLength) {
    *outBuffer = message->payload.payload;
    *outLength = message->payload.length;

    return CELIX_SUCCESS;
}

celix_status_t pubsubProtocol_encodeMetadata(void *handle, pubsub_protocol_message_t *message, void **outBuffer, size_t *outLength) {
    celix_status_t status = CELIX_SUCCESS;

    char *line = calloc(1, 1);
    size_t len = 0;

    const char *key;
    if (message->metadata.metadata != NULL || celix_properties_size(message->metadata.metadata) > 0) {
        CELIX_PROPERTIES_FOR_EACH(message->metadata.metadata, key) {
            const char *val = celix_properties_get(message->metadata.metadata, key, "!Error!");
            char *keyNetString = NULL;
            char *valueNetString = NULL;

            pubsubProtocol_createNetstring(key, &keyNetString);
            pubsubProtocol_createNetstring(val, &valueNetString);

            len += strlen(keyNetString);
            len += strlen(valueNetString);
            char *tmp = realloc(line, len + 1);
            if (!tmp) {
                free(line);
                status = CELIX_ENOMEM;
            }
            line = tmp;

            strncat(line, keyNetString, strlen(keyNetString));
            strncat(line, valueNetString, strlen(valueNetString));

            free(keyNetString);
            free(valueNetString);
        }
    }

    *outBuffer = line;
    *outLength = len;

    return status;
}

celix_status_t pubsubProtocol_decodeHeader(void* handle, void *data, size_t length, pubsub_protocol_message_t *message) {
    celix_status_t status = CELIX_SUCCESS;

    int idx = 0;
    unsigned int sync;
    idx = readInt(data, idx, &sync);
    if (sync != PROTOCOL_WIRE_SYNC) {
        status = CELIX_ILLEGAL_ARGUMENT;
    } else {
        unsigned int envelopeVersion;
        idx = readInt(data, idx, &envelopeVersion);
        if (envelopeVersion != PROTOCOL_WIRE_ENVELOPE_VERSION) {
            status = CELIX_ILLEGAL_ARGUMENT;
        } else {
            idx = readInt(data, idx, &message->header.msgId);
            idx = readShort(data, idx, &message->header.msgMajorVersion);
            idx = readShort(data, idx, &message->header.msgMinorVersion);
            idx = readInt(data, idx, &message->header.payloadSize);
            readInt(data, idx, &message->header.metadataSize);
        }
    }

    return status;
}

celix_status_t pubsubProtocol_decodePayload(void* handle, void *data, size_t length, pubsub_protocol_message_t *message){
    message->payload.payload = data;
    message->payload.length = length;

    return CELIX_SUCCESS;
}

celix_status_t pubsubProtocol_decodeMetadata(void* handle, void *data, size_t length, pubsub_protocol_message_t *message) {
    celix_status_t status = CELIX_SUCCESS;

    char *netstring = data;

    message->metadata.metadata = celix_properties_create();
    while (strlen(netstring) > 0) {
        size_t outlen;
        pubsubProtocol_parseNetstring(netstring, length, &netstring, &outlen);
        char *key = strndup(netstring, outlen);
        netstring += outlen + 1;

        pubsubProtocol_parseNetstring(netstring, length, &netstring, &outlen);
        char *value = strndup(netstring, outlen);
        netstring += outlen + 1;

        celix_properties_setWithoutCopy(message->metadata.metadata, key, value);
    }

    return status;
}

static celix_status_t pubsubProtocol_createNetstring(const char* string, char** netstringOut) {
    celix_status_t status = CELIX_SUCCESS;

    size_t str_len = strlen(string);
    if (str_len == 0) {
        // 0:,
        *netstringOut = calloc(1, 4);
        if (*netstringOut == NULL) {
            status = CELIX_ENOMEM;
        } else {
            *netstringOut[0] = '0';
            *netstringOut[1] = ':';
            *netstringOut[2] = ',';
            *netstringOut[3] = '\0';
        }
    } else {
        size_t numlen = ceil(log10(str_len + 1));
        *netstringOut = calloc(1, numlen + str_len + 3);
        if (*netstringOut == NULL) {
            status = CELIX_ENOMEM;
        } else {
            sprintf(*netstringOut, "%zu:%s,", str_len, string);
        }
    }

    return status;
}

/* Reads a netstring from a `buffer` of length `buffer_length`. Writes
   to `netstring_start` a pointer to the beginning of the string in
   the buffer, and to `netstring_length` the length of the
   string. Does not allocate any memory. If it reads successfully,
   then it returns 0. If there is an error, then the return value will
   be negative. The error values are:
   NETSTRING_ERROR_TOO_LONG      More than 999999999 bytes in a field
   NETSTRING_ERROR_NO_COLON      No colon was found after the number
   NETSTRING_ERROR_TOO_SHORT     Number of bytes greater than buffer length
   NETSTRING_ERROR_NO_COMMA      No comma was found at the end
   NETSTRING_ERROR_LEADING_ZERO  Leading zeros are not allowed
   NETSTRING_ERROR_NO_LENGTH     Length not given at start of netstring
   If you're sending messages with more than 999999999 bytes -- about
   2 GB -- then you probably should not be doing so in the form of a
   single netstring. This restriction is in place partially to protect
   from malicious or erroneous input, and partly to be compatible with
   D. J. Bernstein's reference implementation.
   Example:
      if (netstring_read("3:foo,", 6, &str, &len) < 0) explode_and_die();
 */
static int pubsubProtocol_parseNetstring(char *buffer, size_t buffer_length,
                   char **netstring_start, size_t *netstring_length) {
    int i;
    size_t len = 0;

    /* Write default values for outputs */
    *netstring_start = NULL; *netstring_length = 0;

    /* Make sure buffer is big enough. Minimum size is 3. */
    if (buffer_length < 3) return NETSTRING_ERROR_TOO_SHORT;

    /* No leading zeros allowed! */
    if (buffer[0] == '0' && isdigit(buffer[1]))
        return NETSTRING_ERROR_LEADING_ZERO;

    /* The netstring must start with a number */
    if (!isdigit(buffer[0])) return NETSTRING_ERROR_NO_LENGTH;

    /* Read the number of bytes */
    for (i = 0; i < buffer_length && isdigit(buffer[i]); i++) {
        /* Error if more than 9 digits */
        if (i >= 9) return NETSTRING_ERROR_TOO_LONG;
        /* Accumulate each digit, assuming ASCII. */
        len = len*10 + (buffer[i] - '0');
    }

    /* Check buffer length once and for all. Specifically, we make sure
       that the buffer is longer than the number we've read, the length
       of the string itself, and the colon and comma. */
    if (i + len + 1 >= buffer_length) return NETSTRING_ERROR_TOO_SHORT;

    /* Read the colon */
    if (buffer[i++] != ':') return NETSTRING_ERROR_NO_COLON;

    /* Test for the trailing comma, and set the return values */
    if (buffer[i + len] != ',') return NETSTRING_ERROR_NO_COMMA;
    *netstring_start = &buffer[i]; *netstring_length = len;

    return 0;
}