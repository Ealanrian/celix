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
 * bundle_cache.c
 *
 *  Created on: Aug 6, 2010
 *      Author: alexanderb
 */
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "bundle_cache.h"
#include "bundle_archive.h"
#include "headers.h"
#include "constants.h"

struct bundleCache {
	PROPERTIES configurationMap;
	char * cacheDir;
};

void bundleCache_deleteTree(char * directory);

BUNDLE_CACHE bundleCache_create(PROPERTIES configurationMap) {
	BUNDLE_CACHE cache = (BUNDLE_CACHE) malloc(sizeof(*cache));

	cache->configurationMap = configurationMap;
	char * cacheDir = getProperty(configurationMap, (char *) FRAMEWORK_STORAGE);
	if (cacheDir == NULL) {
		cacheDir = ".cache";
	}
	cache->cacheDir = cacheDir;

	return cache;
}

void bundleCache_delete(BUNDLE_CACHE cache) {
	bundleCache_deleteTree(cache->cacheDir);
}

void bundleCache_deleteTree(char * directory) {
	DIR * dir = opendir(directory);
	struct dirent * dp;
	while ((dp = readdir(dir))) {
		if ((strcmp((dp->d_name), ".") != 0) && (strcmp((dp->d_name), "..") != 0)) {
			char subdir[strlen(directory) + strlen(dp->d_name) + 2];
			strcpy(subdir, directory);
			strcat(subdir, "/");
			strcat(subdir, dp->d_name);

			struct stat s;
			stat(dp->d_name, &s);
			if (S_ISDIR(s.st_mode)) {
//			if (dp->d_type == DT_DIR) {
				bundleCache_deleteTree(subdir);
			} else {
				remove(subdir);
			}
		}
	}
	remove(directory);
}

ARRAY_LIST bundleCache_getArchives(BUNDLE_CACHE cache) {
	DIR * dir = opendir(cache->cacheDir);
	if (dir == NULL) {
		mkdir(cache->cacheDir, 0755);
		dir = opendir(cache->cacheDir);
		if (dir == NULL) {
			printf("Problem opening/creating cache.\n");
			return NULL;
		}
	}

	ARRAY_LIST list = arrayList_create();
	struct dirent * dp;
	while ((dp = readdir(dir))) {
		char archiveRoot[strlen(cache->cacheDir) + strlen(dp->d_name) + 2];
		strcpy(archiveRoot, cache->cacheDir);
		strcat(archiveRoot, "/");
		strcat(archiveRoot, dp->d_name);

		struct stat s;
		stat(archiveRoot, &s);
		if (S_ISDIR(s.st_mode)
//		if (dp->d_type == DT_DIR
				&& (strcmp((dp->d_name), ".") != 0)
				&& (strcmp((dp->d_name), "..") != 0)
				&& (strncmp(dp->d_name, "bundle", 6) == 0)
				&& (strcmp(dp->d_name, "bundle0") != 0)) {

			BUNDLE_ARCHIVE archive = bundleArchive_recreate(strdup(archiveRoot));
			arrayList_add(list, archive);
		}
	}

	return list;
}

BUNDLE_ARCHIVE bundleCache_createArchive(BUNDLE_CACHE cache, long id, char * location) {
	char archiveRoot[256];
	sprintf(archiveRoot, "%s/bundle%ld",  cache->cacheDir, id);

	return bundleArchive_create(strdup(archiveRoot), id, location);
}
