/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2011, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Chris Rienzo <chris@rienzo.net>
 *
 * mod_http_cache.c -- HTTP GET with caching
 *                  -- designed for downloading audio files from a webserver for playback
 *
 */
#include <switch.h>
#include <switch_curl.h>

/* Defines module interface to FreeSWITCH */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_http_cache_shutdown);
SWITCH_MODULE_LOAD_FUNCTION(mod_http_cache_load);
SWITCH_MODULE_DEFINITION(mod_http_cache, mod_http_cache_load, mod_http_cache_shutdown, NULL);
SWITCH_STANDARD_API(http_cache_get);
SWITCH_STANDARD_API(http_cache_put);
SWITCH_STANDARD_API(http_cache_tryget);
SWITCH_STANDARD_API(http_cache_clear);

#define DOWNLOAD_NEEDED "download"

typedef struct url_cache url_cache_t;

/**
 * status if the cache entry
 */
enum cached_url_status {
	/** downloading */
	CACHED_URL_RX_IN_PROGRESS,
	/** marked for removal */
	CACHED_URL_REMOVE,
	/** available */
	CACHED_URL_AVAILABLE
};
typedef enum cached_url_status cached_url_status_t;

/**
 * Cached URL information
 */
struct cached_url {
	/** The URL that was cached */
	char *url;
	/** The path and name of the cached URL */
	char *filename;
	/** The size of the cached URL, in bytes */
	size_t size;
	/** URL use flag */
	int used;
	/** Status of this entry */
	cached_url_status_t status;
	/** Number of sessions waiting for this URL */
	int waiters;
	/** time when downloaded */
	switch_time_t download_time;
	/** nanoseconds until stale */
	switch_time_t max_age;
};
typedef struct cached_url cached_url_t;

static cached_url_t *cached_url_create(url_cache_t *cache, const char *url);
static void cached_url_destroy(cached_url_t *url, switch_memory_pool_t *pool);

/**
 * Data for write_file_callback()
 */
struct http_get_data {
	/** File descriptor for the cached URL */
	int fd;
	/** The cached URL data */
	cached_url_t *url;
};
typedef struct http_get_data http_get_data_t;

static switch_status_t http_get(cached_url_t *url, switch_core_session_t *session);
static size_t get_file_callback(void *ptr, size_t size, size_t nmemb, void *get);
static size_t get_header_callback(void *ptr, size_t size, size_t nmemb, void *url);
static void process_cache_control_header(cached_url_t *url, char *data);

static switch_status_t http_put(switch_core_session_t *session, const char *url, const char *filename);

/**
 * Queue used for clock cache replacement algorithm.  This
 * queue has been simplified since replacement only happens
 * once it is filled.
 */
struct simple_queue {
	/** The queue data */
	void **data;
	/** queue bounds */
	size_t max_size;
	/** queue size */
	size_t size;
	/** Current index */
	int pos;
};
typedef struct simple_queue simple_queue_t;


/**
 * The cache
 */
struct url_cache {
	/** The maximum number of URLs to cache */
	int max_url;
	/** The maximum size of this cache, in bytes */
	size_t max_size;
	/** The default time to allow a cached URL to live, if none is specified */
	switch_time_t default_max_age;
	/** The current size of this cache, in bytes */
	size_t size;
	/** The location of the cache in the filesystem */
	char *location;
	/** Cache mapped by URL */
	switch_hash_t *map;
	/** Cached URLs queued for replacement */
	simple_queue_t queue;
	/** Synchronizes access to cache */
	switch_mutex_t *mutex;
	/** Memory pool */
	switch_memory_pool_t *pool;
	/** Number of cache hits */
	int hits;
	/** Number of cache misses */
	int misses;
	/** Number of cache errors */
	int errors;
};
static url_cache_t gcache;

static char *url_cache_get(url_cache_t *cache, switch_core_session_t *session, const char *url, int download, switch_memory_pool_t *pool);
static switch_status_t url_cache_add(url_cache_t *cache, switch_core_session_t *session, cached_url_t *url);
static void url_cache_remove(url_cache_t *cache, switch_core_session_t *session, cached_url_t *url);
static void url_cache_remove_soft(url_cache_t *cache, switch_core_session_t *session, cached_url_t *url);
static switch_status_t url_cache_replace(url_cache_t *cache, switch_core_session_t *session);
static void url_cache_lock(url_cache_t *cache, switch_core_session_t *session);
static void url_cache_unlock(url_cache_t *cache, switch_core_session_t *session);
static void url_cache_clear(url_cache_t *cache, switch_core_session_t *session);

/**
 * Put a file to the URL
 * @param session the (optional) session uploading the file
 * @param url The URL
 * @param filename The file to upload
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t http_put(switch_core_session_t *session, const char *url, const char *filename)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	CURL *curl_handle = NULL;
	long httpRes = 0;
	struct stat file_info = {0};
	FILE *file_to_put = NULL;
	int fd;
	
	/* open file and get the file size */
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "opening %s for upload to %s\n", filename, url);
	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "open() error: %s\n", strerror(errno));
		status = SWITCH_STATUS_FALSE;
		goto done;
	}
	if (fstat(fd, &file_info) == -1) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "fstat() error: %s\n", strerror(errno));
	}
	close(fd);

	/* libcurl requires FILE* */
 	file_to_put = fopen(filename, "rb");
	if (!file_to_put) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "fopen() error: %s\n", strerror(errno));
		status = SWITCH_STATUS_FALSE;
		goto done;
	}
	
	curl_handle = switch_curl_easy_init();
	if (!curl_handle) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "switch_curl_easy_init() failure\n");
		status = SWITCH_STATUS_FALSE;
		goto done;
	}
	switch_curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1);
	switch_curl_easy_setopt(curl_handle, CURLOPT_PUT, 1);
	switch_curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
	switch_curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	switch_curl_easy_setopt(curl_handle, CURLOPT_READDATA, file_to_put);
	switch_curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
	switch_curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	switch_curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10);
	switch_curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "freeswitch-http-cache/1.0");
	switch_curl_easy_perform(curl_handle);
	switch_curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpRes);
	switch_curl_easy_cleanup(curl_handle);

	if (httpRes == 200 || httpRes == 201 || httpRes == 204) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "%s saved to %s\n", filename, url);
	} else {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "Received HTTP error %ld trying to save %s to %s\n", httpRes, filename, url);
		status = SWITCH_STATUS_GENERR;
	}
	
done:
	if (file_to_put) {
		fclose(file_to_put);
	}

	return status;
}

/**
 * Called by libcurl to write result of HTTP GET to a file
 * @param ptr The data to write
 * @param size The size of the data element to write
 * @param nmemb The number of elements to write
 * @param get Info about this current GET request
 * @return bytes processed
 */
static size_t get_file_callback(void *ptr, size_t size, size_t nmemb, void *get)
{
	size_t realsize = (size * nmemb);
	http_get_data_t *get_data = get;
	ssize_t bytes_written = write(get_data->fd, ptr, realsize);
	size_t result = 0;
	if (bytes_written == -1) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "write(): %s\n", strerror(errno));
	} else {
		if (bytes_written != realsize) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "write(): short write!\n");
		}
		get_data->url->size += bytes_written;
		result = bytes_written;
	}
	
	return result;
}
/**
 * trim whitespace characters from string
 * @param str the string to trim
 * @return the trimmed string
 */
static char *trim(char *str)
{
	size_t len;

	if (zstr(str)) {
		return str;
	}
	len = strlen(str);

	/* strip whitespace from front */
	for (int i = 0; i < len; i++) {
		if (!isspace(str[i])) {
			str = &str[i];
			len -= i;
			break;
		}
	}
	if (zstr(str)) {
		return str;
	}
	
	/* strip whitespace from end */
	for (int i = len - 1; i >= 0; i--) {
		if (!isspace(str[i])) {
			break;
		}
		str[i] = '\0';
	}
	return str;
}

#define MAX_AGE "max-age="
/**
 * cache-control: max-age=123456
 * Only support max-age.  All other params are ignored.
 */
static void process_cache_control_header(cached_url_t *url, char *data)
{
	char *max_age_str;
	switch_time_t max_age;

	/* trim whitespace and check if empty */
	data = trim(data);
	if (zstr(data)) {
		return;
	}

	/* find max-age param */
	max_age_str = strcasestr(data, MAX_AGE);
	if (zstr(max_age_str)) {
		return;
	}

	/* find max-age value */
	max_age_str = max_age_str + (sizeof(MAX_AGE) - 1);
	if (zstr(max_age_str)) {
		return;
	}
	for (int i = 0; i < strlen(max_age_str); i++) {
		if (!isdigit(max_age_str[i])) {
			max_age_str[i] = '\0';
			break;
		}
	}
	if (zstr(max_age_str)) {
		return;
	}
	max_age = atoi(max_age_str);

	if (max_age < 0) {
		return;
	}

	url->max_age = switch_time_now() + (max_age * 1000 * 1000);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "setting max age to %u seconds from now\n", (int)max_age);
}

#define CACHE_CONTROL_HEADER "cache-control:"
#define CACHE_CONTROL_HEADER_LEN (sizeof(CACHE_CONTROL_HEADER) - 1)
/**
 * Called by libcurl to process headers from HTTP GET response
 * @param ptr the header data
 * @param size The size of the data element to write
 * @param nmemb The number of elements to write
 * @param get get data
 * @return bytes processed
 */
static size_t get_header_callback(void *ptr, size_t size, size_t nmemb, void *get)
{
	size_t realsize = (size * nmemb);
	cached_url_t *url = get;
	char *header = NULL;

	/* validate length... Apache and IIS won't send a header larger than 16 KB */
	if (realsize == 0 || realsize > 1024 * 16) {
		return realsize;
	}

	/* get the header, adding NULL terminator if there isn't one */
	switch_zmalloc(header, realsize + 1);
	strncpy(header, (char *)ptr, realsize);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s", header);
	
	/* check which header this is and process it */
	if (!strncasecmp(CACHE_CONTROL_HEADER, header, CACHE_CONTROL_HEADER_LEN)) {
		process_cache_control_header(url, header + CACHE_CONTROL_HEADER_LEN);
	}

	switch_safe_free(header);
	return realsize;
}

/**
 * Get exclusive access to the cache
 * @param cache The cache
 * @param session The session acquiring the cache
 */
static void url_cache_lock(url_cache_t *cache, switch_core_session_t *session)
{
	switch_mutex_lock(cache->mutex);
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Locked cache\n");
}

/**
 * Relinquish exclusive access to the cache
 * @param cache The cache
 * @param session The session relinquishing the cache
 */
static void url_cache_unlock(url_cache_t *cache, switch_core_session_t *session)
{
	switch_mutex_unlock(cache->mutex);
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Unlocked cache\n");
}

/**
 * Empties the cache
 */
static void url_cache_clear(url_cache_t *cache, switch_core_session_t *session)
{
	url_cache_lock(cache, session);

	// remove each cached URL from the hash and the queue
	for (int i = 0; i < cache->queue.max_size; i++) {
		cached_url_t *url = cache->queue.data[i];
		if (url) {
			switch_core_hash_delete(cache->map, url->url);
			cached_url_destroy(url, cache->pool);
			cache->queue.data[i] = NULL;
		}
	}
	cache->queue.pos = 0;
	cache->queue.size = 0;

	// reset cache stats
	cache->size = 0;
	cache->hits = 0;
	cache->misses = 0;
	cache->errors = 0;

	url_cache_unlock(cache, session);

	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Emptied cache\n");
}

/**
 * Get a URL from the cache, add it if it does not exist
 * @param cache The cache
 * @param session the (optional) session requesting the URL
 * @param url The URL
 * @param download If true, the file will be downloaded if it does not exist in the cache.
 * @param pool The pool to use for allocating the filename
 * @return The filename or NULL if there is an error
 */
static char *url_cache_get(url_cache_t *cache, switch_core_session_t *session, const char *url, int download, switch_memory_pool_t *pool)
{
	char *filename = NULL;
	cached_url_t *u = NULL;
	if (zstr(url)) {
		return NULL;
	}

	url_cache_lock(cache, session);
	u = switch_core_hash_find(cache->map, url);

	if (u && u->status == CACHED_URL_AVAILABLE) {
		if (switch_time_now() >= (u->download_time + u->max_age)) {
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Cached URL has expired.\n");
			url_cache_remove_soft(cache, session, u); /* will get permanently deleted upon replacement */
			u = NULL;
		} else if (switch_file_exists(u->filename, pool) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Cached URL file is missing.\n");
			url_cache_remove_soft(cache, session, u); /* will get permanently deleted upon replacement */
			u = NULL;
		}
	}

	if (!u && download) {
		/* URL is not cached, let's add it.*/
		/* Set up URL entry and add to map to prevent simultaneous downloads */
		cache->misses++;
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Cache MISS: size = %zu (%zu MB), hit ratio = %d/%d\n", cache->queue.size, cache->size / 1000000, cache->hits, cache->hits + cache->misses);
		u = cached_url_create(cache, url);
		if (url_cache_add(cache, session, u) != SWITCH_STATUS_SUCCESS) {
			/* This error should never happen */
			url_cache_unlock(cache, session);
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_CRIT, "Failed to add URL to cache!\n");
			cached_url_destroy(u, cache->pool);
			return NULL;
		}

		/* download the file */
		url_cache_unlock(cache, session);
		if (http_get(u, session) == SWITCH_STATUS_SUCCESS) {
			/* Got the file, let the waiters know it is available */
			url_cache_lock(cache, session);
			u->status = CACHED_URL_AVAILABLE;
			filename = switch_core_strdup(pool, u->filename);
			cache->size += u->size;
		} else {
			/* Did not get the file, flag for replacement */
			url_cache_lock(cache, session);
			url_cache_remove_soft(cache, session, u);
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Failed to download URL %s\n", url);
			cache->errors++;
		}
	} else if (!u) {
		filename = DOWNLOAD_NEEDED;
	} else {
		/* Wait until file is downloaded */
		if (u->status == CACHED_URL_RX_IN_PROGRESS) {
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Waiting for URL %s to be available\n", url);
			u->waiters++;
			url_cache_unlock(cache, session);
			while(u->status == CACHED_URL_RX_IN_PROGRESS) {
				switch_sleep(10 * 1000); /* 10 ms */
			}
			url_cache_lock(cache, session);
			u->waiters--;
		}

		/* grab filename if everything is OK */
		if (u->status == CACHED_URL_AVAILABLE) {
			filename = switch_core_strdup(pool, u->filename);
			cache->hits++;
			u->used = 1;
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Cache HIT: size = %zu (%zu MB), hit ratio = %d/%d\n", cache->queue.size, cache->size / 1000000, cache->hits, cache->hits + cache->misses);
		}
	}
	url_cache_unlock(cache, session);
	return filename;
}

/**
 * Add a URL to the cache.  The caller must lock the cache.
 * @param cache the cache
 * @param session the (optional) session
 * @param url the URL to add
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t url_cache_add(url_cache_t *cache, switch_core_session_t *session, cached_url_t *url)
{
	simple_queue_t *queue = &cache->queue;
	if (queue->size >= queue->max_size && url_cache_replace(cache, session) != SWITCH_STATUS_SUCCESS) {
		return SWITCH_STATUS_FALSE;
	}
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Adding %s(%s) to cache index %d\n", url->url, url->filename, queue->pos);

	queue->data[queue->pos] = url;
	queue->pos = (queue->pos + 1) % queue->max_size;
	queue->size++;
	switch_core_hash_insert(cache->map, url->url, url);
	return SWITCH_STATUS_SUCCESS;
}

/**
 * Select a URL for replacement and remove it from the cache.
 * Currently implemented with the clock replacement algorithm.  It's not
 * great, but is better than least recently used and is simple to implement.
 *
 * @param cache the cache
 * @param session the (optional) session 
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t url_cache_replace(url_cache_t *cache, switch_core_session_t *session)
{
	switch_status_t status = SWITCH_STATUS_FALSE;
	int i = 0;
	simple_queue_t *queue = &cache->queue;
	
	if (queue->size < queue->max_size || queue->size == 0) {
		return SWITCH_STATUS_FALSE;
	}

	for (i = 0; i < queue->max_size * 2; i++) {
		cached_url_t *to_replace = (cached_url_t *)queue->data[queue->pos];

		/* check for queue corruption */
		if (to_replace == NULL) {
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_CRIT, "Unexpected empty URL at cache index %d\n", queue->pos);
			status = SWITCH_STATUS_SUCCESS;
			break;
		}

		/* check if available for replacement */
		if (!to_replace->used && !to_replace->waiters) {
			/* remove from cache and destroy it */
			url_cache_remove(cache, session, to_replace);
			cached_url_destroy(to_replace, cache->pool);
			status = SWITCH_STATUS_SUCCESS;
			break;
		}

		/* not available for replacement.  Mark as not used and move to back of queue */
		if (to_replace->status == CACHED_URL_AVAILABLE) {
			to_replace->used = 0;
		}
		queue->pos = (queue->pos + 1) % queue->max_size;
	}

	return status;
}

/**
 * Remove a URL from the hash map and mark it as eligible for replacement from the queue.
 */
static void url_cache_remove_soft(url_cache_t *cache, switch_core_session_t *session, cached_url_t *url)
{
	switch_core_hash_delete(cache->map, url->url);
	url->used = 0;
	url->status = CACHED_URL_REMOVE;
}

/**
 * Remove a URL from the front or back of the cache
 * @param cache the cache
 * @param session the (optional) session
 * @param url the URL to remove
 */
static void url_cache_remove(url_cache_t *cache, switch_core_session_t *session, cached_url_t *url)
{
	simple_queue_t *queue;
	cached_url_t *to_remove;

	url_cache_remove_soft(cache, session, url);

	/* make sure cached URL matches the item in the queue and remove it */
	queue = &cache->queue;
	to_remove = (cached_url_t *)queue->data[queue->pos];
	if (url == to_remove) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Removing %s(%s) from cache index %d\n", url->url, url->filename, queue->pos);
		queue->data[queue->pos] = NULL;
		queue->size--; 
	} else {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_CRIT, "URL entry, %s, not in cache queue!!!\n", url->url);
	}

	/* adjust cache statistics */
	cache->size -= url->size;
}

/**
 * Create a cached URL entry
 * @param cache the cache
 * @param url the URL to cache
 * @return the cached URL
 */
static cached_url_t *cached_url_create(url_cache_t *cache, const char *url)
{
	switch_uuid_t uuid;
	char uuid_str[SWITCH_UUID_FORMATTED_LENGTH + 1] = { 0 };
	char *filename = NULL;
	char uuid_dir[3] = { 0 };
	char *dirname = NULL;
	cached_url_t *u = NULL;
	const char *file_extension = "";

	if (zstr(url)) {
		return NULL;
	}
	
	switch_zmalloc(u, sizeof(cached_url_t));

	/* filename is constructed from UUID and is stored in cache dir (first 2 characters of UUID) */
	switch_uuid_get(&uuid);
	switch_uuid_format(uuid_str, &uuid);
	strncpy(uuid_dir, uuid_str, 2);
	dirname = switch_mprintf("%s%s%s", cache->location, SWITCH_PATH_SEPARATOR, uuid_dir);
	filename = &uuid_str[2];

	/* create sub-directory if it doesn't exist */
	switch_dir_make_recursive(dirname, SWITCH_DEFAULT_DIR_PERMS, cache->pool);
	
	/* find extension on the end of URL */
	for(const char *ext = &url[strlen(url) - 1]; ext != url; ext--) {
		if (*ext == '/' || *ext == '\\') {
			break;
		}
		if (*ext == '.') {
			/* found it */
			file_extension = ext;
			break;
		}
	}

	/* intialize cached URL */
	u->filename = switch_mprintf("%s%s%s%s", dirname, SWITCH_PATH_SEPARATOR, filename, file_extension);
	u->url = switch_safe_strdup(url);
	u->size = 0;
	u->used = 1;
	u->status = CACHED_URL_RX_IN_PROGRESS;
	u->waiters = 0;
	u->download_time = switch_time_now();
	u->max_age = cache->default_max_age;

	switch_safe_free(dirname);

	return u;
}

/**
 * Destroy a cached URL (delete file and struct)
 */
static void cached_url_destroy(cached_url_t *url, switch_memory_pool_t *pool)
{
	if (!zstr(url->filename)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Deleting %s from cache\n", url->filename);
		switch_file_remove(url->filename, pool);
	}
	switch_safe_free(url->filename);
	switch_safe_free(url->url);
	switch_safe_free(url);
}

/**
 * Fetch a file via HTTP
 * @param url The cached URL entry
 * @param session the (optional) session
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t http_get(cached_url_t *url, switch_core_session_t *session)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	switch_CURL *curl_handle = NULL;
	http_get_data_t get_data = {0};
	long httpRes = 0;
	int start_time_ms = switch_time_now() / 1000;

	/* set up HTTP GET */
	get_data.fd = 0;
	get_data.url = url;
	
	curl_handle = switch_curl_easy_init();
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "opening %s for URL cache\n", get_data.url->filename);
	if ((get_data.fd = open(get_data.url->filename, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR)) > -1) {
		switch_curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
		switch_curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10);
		switch_curl_easy_setopt(curl_handle, CURLOPT_URL, get_data.url->url);
		switch_curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, get_file_callback);
		switch_curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *) &get_data);
		switch_curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, get_header_callback);
		switch_curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *) url);
		switch_curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "freeswitch-http-cache/1.0");
		switch_curl_easy_perform(curl_handle);
		switch_curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpRes);
		switch_curl_easy_cleanup(curl_handle);
		close(get_data.fd);
	} else {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "open() error: %s\n", strerror(errno));
		return SWITCH_STATUS_GENERR;
	}

	if (httpRes == 200) {
		int duration_ms = (switch_time_now() / 1000) - start_time_ms;
		if (duration_ms > 500) {
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_WARNING, "URL %s downloaded in %d ms\n", url->url, duration_ms);
		} else {
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "URL %s downloaded in %d ms\n", url->url, duration_ms);
		}
	} else {
		url->size = 0; // nothing downloaded or download interrupted
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "Received HTTP error %ld trying to fetch %s\n", httpRes, url->url);
		return SWITCH_STATUS_GENERR;
	}
	
	return status;
}

/**
 * Empties the entire cache
 * @param cache the cache to empty
 */
static void setup_dir(url_cache_t *cache)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "setting up %s\n", cache->location);
	switch_dir_make_recursive(cache->location, SWITCH_DEFAULT_DIR_PERMS, cache->pool);

	for (int i = 0x00; i <= 0xff; i++) {
		switch_dir_t *dir = NULL;
		char *dirname = switch_mprintf("%s%s%02x", cache->location, SWITCH_PATH_SEPARATOR, i);
		if (switch_dir_open(&dir, dirname, cache->pool) == SWITCH_STATUS_SUCCESS) {
			char filenamebuf[256] = { 0 };
			const char *filename = NULL;
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "deleting cache files in %s...\n", dirname);
			for(filename = switch_dir_next_file(dir, filenamebuf, sizeof(filenamebuf)); filename;
					filename = switch_dir_next_file(dir, filenamebuf, sizeof(filenamebuf))) {
				char *path = switch_mprintf("%s%s%s", dirname, SWITCH_PATH_SEPARATOR, filename);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "deleting: %s\n", path);
				switch_file_remove(path, cache->pool);
				switch_safe_free(path);
			}
			switch_dir_close(dir);
		}
		switch_safe_free(dirname);
	}
}

#define HTTP_GET_SYNTAX "<url>"
/**
 * Get a file from the cache, download if it isn't cached
 */
SWITCH_STANDARD_API(http_cache_get)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	switch_memory_pool_t *lpool = NULL;
	switch_memory_pool_t *pool = NULL;
	char *filename;

	if (zstr(cmd) || strncmp("http://", cmd, strlen("http://"))) {
		stream->write_function(stream, "USAGE: %s\n", HTTP_GET_SYNTAX);
		return SWITCH_STATUS_SUCCESS;
	}

	if (session) {
		pool = switch_core_session_get_pool(session);
	} else {
		switch_core_new_memory_pool(&lpool);
		pool = lpool;
	}

	filename = url_cache_get(&gcache, session, cmd, 1, pool);
	if (filename) {
		stream->write_function(stream, "%s", filename);
		
	} else {
		stream->write_function(stream, "-ERR\n");
		status = SWITCH_STATUS_FALSE;
	}

	if (lpool) {
		switch_core_destroy_memory_pool(&lpool);
	}

	return status;
}

#define HTTP_TRYGET_SYNTAX "<url>"
/**
 * Get a file from the cache, fail if download is needed
 */
SWITCH_STANDARD_API(http_cache_tryget)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	switch_memory_pool_t *lpool = NULL;
	switch_memory_pool_t *pool = NULL;
	char *filename;

	if (zstr(cmd) || strncmp("http://", cmd, strlen("http://"))) {
		stream->write_function(stream, "USAGE: %s\n", HTTP_GET_SYNTAX);
		return SWITCH_STATUS_SUCCESS;
	}

	if (session) {
		pool = switch_core_session_get_pool(session);
	} else {
		switch_core_new_memory_pool(&lpool);
		pool = lpool;
	}

	filename = url_cache_get(&gcache, session, cmd, 0, pool);
	if (filename) {
		if (!strcmp(DOWNLOAD_NEEDED, filename)) {
			stream->write_function(stream, "-ERR %s\n", DOWNLOAD_NEEDED);
		} else {
			stream->write_function(stream, "%s", filename);
		}
	} else {
		stream->write_function(stream, "-ERR\n");
		status = SWITCH_STATUS_FALSE;
	}

	if (lpool) {
		switch_core_destroy_memory_pool(&lpool);
	}

	return status;
}

#define HTTP_PUT_SYNTAX "<url> <file>"
/**
 * Put a file to the server
 */
SWITCH_STANDARD_API(http_cache_put)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	char *args = NULL;
	char *argv[10] = { 0 };
	int argc = 0;

	if (zstr(cmd) || strncmp("http://", cmd, strlen("http://"))) {
		stream->write_function(stream, "USAGE: %s\n", HTTP_PUT_SYNTAX);
		status = SWITCH_STATUS_SUCCESS;
		goto done;
	}

	args = strdup(cmd);
	argc = switch_separate_string(args, ' ', argv, (sizeof(argv) / sizeof(argv[0])));
	if (argc != 2) {
		stream->write_function(stream, "USAGE: %s\n", HTTP_PUT_SYNTAX);
		status = SWITCH_STATUS_SUCCESS;
		goto done;
	}

	status = http_put(session, argv[0], argv[1]);
	if (status == SWITCH_STATUS_SUCCESS) {
		stream->write_function(stream, "+OK\n");
	} else {
		stream->write_function(stream, "-ERR\n");
	}

done:
	switch_safe_free(args);
	return status;
}

#define HTTP_CACHE_CLEAR_SYNTAX ""
/**
 * Clears the cache
 */
SWITCH_STANDARD_API(http_cache_clear)
{
	if (!zstr(cmd)) {
		stream->write_function(stream, "USAGE: %s\n", HTTP_CACHE_CLEAR_SYNTAX);
	} else {
		url_cache_clear(&gcache, session);
		stream->write_function(stream, "+OK\n");
	}
	return SWITCH_STATUS_SUCCESS;
}

/**
 * Configure the module
 * @param cache to configure
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t do_config(url_cache_t *cache)
{
	char *cf = "http_cache.conf";
	switch_xml_t cfg, xml, param, settings;
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	int max_urls;
	switch_time_t default_max_age_sec;

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
		return SWITCH_STATUS_TERM;
	}

	/* set default config */
	max_urls = 4000;
	default_max_age_sec = 86400;
	cache->location = SWITCH_PREFIX_DIR "/http_cache";

	/* get params */
	settings = switch_xml_child(cfg, "settings");
	if (settings) {
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			char *var = (char *) switch_xml_attr_soft(param, "name");
			char *val = (char *) switch_xml_attr_soft(param, "value");
			if (!strcasecmp(var, "max-urls")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Setting max-urls to %s\n", val);
				max_urls = atoi(val);
			} else if (!strcasecmp(var, "location")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Setting location to %s\n", val);
				cache->location = switch_core_strdup(cache->pool, val);
			} else if (!strcasecmp(var, "default-max-age")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Setting default-max-age to %s\n", val);
				default_max_age_sec = atoi(val);
			} else {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Unsupported param: %s\n", var);
			}
		}
	}

	/* check config */
	if (max_urls <= 0) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "max-urls must be > 0\n");
		status = SWITCH_STATUS_TERM;
		goto done; 
	}
	if (zstr(cache->location)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "location must not be empty\n");
		status = SWITCH_STATUS_TERM;
		goto done;
	}
	if (default_max_age_sec <= 0) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "default-max-age must be > 0\n");
		status = SWITCH_STATUS_TERM;
		goto done;
	}

	cache->max_url = max_urls;
	cache->default_max_age = (default_max_age_sec * 1000 * 1000); /* convert from seconds to nanoseconds */
done:
	switch_xml_free(xml);

	return status;
}

/**
 * Called when FreeSWITCH loads the module
 */
SWITCH_MODULE_LOAD_FUNCTION(mod_http_cache_load)
{
	switch_api_interface_t *api;
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(api, "http_get", "HTTP GET", http_cache_get, HTTP_GET_SYNTAX);
	SWITCH_ADD_API(api, "http_tryget", "HTTP GET from cache only", http_cache_tryget, HTTP_GET_SYNTAX);
	SWITCH_ADD_API(api, "http_put", "HTTP PUT", http_cache_put, HTTP_PUT_SYNTAX);
	SWITCH_ADD_API(api, "http_clear_cache", "Clear the cache", http_cache_clear, HTTP_CACHE_CLEAR_SYNTAX);
	
	memset(&gcache, 0, sizeof(url_cache_t));
	gcache.pool = pool;

	if (do_config(&gcache) != SWITCH_STATUS_SUCCESS) {
		return SWITCH_STATUS_TERM;
	}

	switch_core_hash_init(&gcache.map, gcache.pool);
	switch_mutex_init(&gcache.mutex, SWITCH_MUTEX_UNNESTED, gcache.pool);

	/* create the queue */
	gcache.queue.max_size = gcache.max_url;
	gcache.queue.data = switch_core_alloc(gcache.pool, sizeof(void *) * gcache.queue.max_size);
	gcache.queue.pos = 0;
	gcache.queue.size = 0;

	setup_dir(&gcache);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/**
 * Called when FreeSWITCH stops the module
 */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_http_cache_shutdown)
{
	url_cache_clear(&gcache, NULL);
	switch_core_hash_destroy(&gcache.map);
	switch_mutex_destroy(gcache.mutex);
	return SWITCH_STATUS_SUCCESS;
}


/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */
