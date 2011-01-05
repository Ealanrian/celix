/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */
/*
 * echo_client_private.h
 *
 *  Created on: Sep 21, 2010
 *      Author: alexanderb
 */

#ifndef ECHO_CLIENT_PRIVATE_H_
#define ECHO_CLIENT_PRIVATE_H_

#include <stdbool.h>
#include <pthread.h>

#include "headers.h"

struct echoClient {
	SERVICE_TRACKER tracker;
	bool running;

	pthread_t sender;
};

typedef struct echoClient * ECHO_CLIENT;

ECHO_CLIENT echoClient_create(SERVICE_TRACKER context);

void echoClient_start(ECHO_CLIENT client);
void echoClient_stop(ECHO_CLIENT client);

void echoClient_destroy(ECHO_CLIENT client);


#endif /* ECHO_CLIENT_PRIVATE_H_ */
