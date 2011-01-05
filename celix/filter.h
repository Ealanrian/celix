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
 * filter.h
 *
 *  Created on: Apr 28, 2010
 *      Author: dk489
 */

#ifndef FILTER_H_
#define FILTER_H_

typedef struct filter * FILTER;

FILTER filter_create(char * filterString);
void filter_destroy(FILTER filter);

int filter_match(FILTER filter, HASHTABLE properties);

#endif /* FILTER_H_ */
