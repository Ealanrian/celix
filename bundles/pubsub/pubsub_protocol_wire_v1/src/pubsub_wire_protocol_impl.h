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

#ifndef PUBSUB_PROTOCOL_WIRE_H_
#define PUBSUB_PROTOCOL_WIRE_H_

#include "pubsub_protocol.h"

#define PUBSUB_WIRE_PROTOCOL_TYPE "wire"

typedef struct pubsub_protocol_wire_v1 pubsub_protocol_wire_v1_t;

celix_status_t pubsubProtocol_create(celix_bundle_context_t *context, pubsub_protocol_wire_v1_t **protocol);
celix_status_t pubsubProtocol_destroy(pubsub_protocol_wire_v1_t* protocol);

celix_status_t pubsubProtocol_getSyncHeader(void* handle, void *syncHeader);

celix_status_t pubsubProtocol_encodeHeader(void *handle, pubsub_protocol_message_t *message, void **outBuffer, size_t *outLength);
celix_status_t pubsubProtocol_encodePayload(void *handle, pubsub_protocol_message_t *message, void **outBuffer, size_t *outLength);
celix_status_t pubsubProtocol_encodeMetadata(void *handle, pubsub_protocol_message_t *message, void **outBuffer, size_t *outLength);

celix_status_t pubsubProtocol_decodeHeader(void* handle, void *data, size_t length, pubsub_protocol_message_t *message);
celix_status_t pubsubProtocol_decodePayload(void* handle, void *data, size_t length, pubsub_protocol_message_t *message);
celix_status_t pubsubProtocol_decodeMetadata(void* handle, void *data, size_t length, pubsub_protocol_message_t *message);


#endif /* PUBSUB_PROTOCOL_WIRE_H_ */