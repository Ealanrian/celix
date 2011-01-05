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
 * command.c
 *
 *  Created on: Aug 13, 2010
 *      Author: alexanderb
 */

#include <stdio.h>

#include "command_private.h"

char * command_getName(COMMAND command) {
	return command->name;
}

char * command_getUsage(COMMAND command) {
	return command->usage;
}

char * command_getShortDescription(COMMAND command) {
	return command->shortDescription;
}

void command_execute(COMMAND command, char * line, void (*out)(char *), void (*err)(char *)) {
	command->executeCommand(command, line, out, err);
}
