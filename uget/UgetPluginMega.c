/*
 *
 *   Copyright (C) 2016-2018 by C.H. Huang
 *   plushuang.tw@gmail.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  ---
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU Lesser General Public License in all respects
 *  for all of the code used other than OpenSSL.  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so.  If you
 *  do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 *
 */


#ifdef	HAVE_CONFIG_H
#include <config.h>
#endif

// use OpenSSL by default in Windows and Android
#if defined _WIN32 || defined _WIN64 || defined __ANDROID__
#  if !(defined USE_OPENSSL || defined USE_GNUTLS)
#    define USE_OPENSSL
#  endif
#endif

// OpenSSL
#ifdef USE_OPENSSL
//#include <openssl/crypto.h>
#include <openssl/opensslv.h>  // OPENSSL_VERSION_NUMBER
#include <openssl/modes.h>     // CRYPTO_ctr128_encrypt
#include <openssl/aes.h>       // AES_BLOCK_SIZE
// GnuTLS
#elif defined USE_GNUTLS
#include <gcrypt.h>
// mega plug-in must decrypt data
#else
#error mega plug-in need OpenSSL or GnuTLS to compile.
#endif

#include <curl/curl.h>
#include <UgetCurl.h>

#include <UgUtil.h>
#include <UgFileUtil.h>
#include <UgStdio.h>
#include <UgString.h>
#include <UgetPluginMega.h>

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#define  ug_sleep                 Sleep
#else
#include <unistd.h>               // usleep()
#define  ug_sleep(millisecond)    usleep(millisecond * 1000)
#endif // _WIN32 || _WIN64

#ifdef HAVE_GLIB
#include <glib/gi18n.h>
#undef  printf
#else
#define N_(x)   x
#define  _(x)   x
#endif

enum
{
	MEGA_UNKNOWN,
	MEGA_FOLDER,
	MEGA_FILE,
};

// ----------------------------------------------------------------------------
// MEGA site
static int  mega_parse_url(UgetPluginMega* plugin, const char* url);
static int  mega_request_info(UgetPluginMega* plugin, const char* id);
static int  mega_decrypt_file(UgetPluginMega* plugin, int preset_progress);

// ----------------------------------------------------------------------------
// UgetPluginInfo (derived from UgDataInfo)

static void plugin_init (UgetPluginMega* plugin);
static void plugin_final(UgetPluginMega* plugin);
static int  plugin_start(UgetPluginMega* plugin, UgetNode* node);
static int  plugin_sync (UgetPluginMega* plugin, UgetNode* node);

static const char* schemes[] = {"https", NULL};
static const char* hosts[]   = {"mega.co.nz", "mega.nz",
                                NULL};

static const UgetPluginInfo UgetPluginMegaInfoStatic =
{
	"mega",
	sizeof(UgetPluginMega),
	(UgInitFunc)   plugin_init,
	(UgFinalFunc)  plugin_final,
	(UgAssignFunc) plugin_start,
	(UgetPluginCtrlFunc) uget_plugin_agent_ctrl,
	(UgetPluginSyncFunc) plugin_sync,
	hosts,
	schemes,
	NULL,
	(UgetPluginSetFunc) uget_plugin_agent_global_set,
	(UgetPluginGetFunc) uget_plugin_agent_global_get
};
// extern
const UgetPluginInfo* UgetPluginMegaInfo = &UgetPluginMegaInfoStatic;

// ----------------------------------------------------------------------------
// control functions

static void plugin_init(UgetPluginMega* plugin)
{
	// initialize UgetPluginAgent
	uget_plugin_agent_init((UgetPluginAgent*)plugin);

	// initialize UgetPluginMega
	ug_json_init(&plugin->json);
	ug_value_init_object(&plugin->value, 5);
}

static void plugin_final(UgetPluginMega* plugin)
{
	// finalize UgetPluginMega
	ug_free(plugin->id);
	ug_free(plugin->key);
	ug_free(plugin->iv);
	ug_free(plugin->url);
	ug_free(plugin->file);
	ug_json_final(&plugin->json);
	ug_value_clear(&plugin->value);

	// finalize UgetPluginAgent
	uget_plugin_agent_final((UgetPluginAgent*)plugin);
}

// ----------------------------------------------------------------------------

static UG_THREAD_RETURN_TYPE  plugin_thread(UgetPluginMega* plugin);

static int  plugin_start(UgetPluginMega* plugin, UgetNode* node)
{
	UgetCommon*  common;

	common = ug_info_get(node->info, UgetCommonInfo);
	if (common == NULL || common->uri == NULL)
		return FALSE;

	// parse MEGA URL
	if (mega_parse_url(plugin, common->uri) != MEGA_FILE) {
		uget_plugin_post((UgetPlugin*) plugin,
				uget_event_new_error(UGET_EVENT_ERROR_CUSTOM,
				                     _("Can't handle this MEGA URL.")));
		return FALSE;
	}

	plugin->target_node = uget_node_new(NULL);
	ug_info_assign(plugin->target_node->info, node->info, NULL);
	plugin->target_proxy  = ug_info_get(plugin->target_node->info, UgetProxyInfo);
	plugin->target_common = ug_info_get(plugin->target_node->info, UgetCommonInfo);
	plugin->target_progress = ug_info_realloc(plugin->target_node->info, UgetProgressInfo);

	return uget_plugin_agent_start_thread((UgetPluginAgent*)plugin, node,
	                                      (UgThreadFunc)plugin_thread);
}

static int  is_downloaded(UgetPluginMega* plugin, UgetCommon* target_common);

static UG_THREAD_RETURN_TYPE  plugin_thread(UgetPluginMega* plugin)
{
	UgetPluginInfo*  plugin_info;
	UgetCommon* target_common;
	UgetEvent*  msg_next;
	UgetEvent*  msg;

	// get MEGA download URL & attributes
	if (mega_request_info(plugin, plugin->id) == FALSE) {
		uget_plugin_post((UgetPlugin*) plugin,
				uget_event_new_error(UGET_EVENT_ERROR_CUSTOM,
				                     _("Can't get download URL.")));
		goto exit;
	}

	target_common = plugin->target_common;
	// set MEGA download URL
	ug_free(target_common->uri);
	target_common->uri = plugin->url;
	plugin->url = NULL;
	// set MEGA output file name
	if (target_common->file == NULL)
		plugin->named = TRUE;
	else {
		ug_free(plugin->file);
		plugin->file = ug_strdup(target_common->file);
	}
	ug_free(target_common->file);
	target_common->file = ug_strdup_printf("%s.enc", plugin->file);

	// check existed file
	if (is_downloaded(plugin, target_common) == TRUE) {
		mega_decrypt_file(plugin, TRUE);
		goto exit;
	}

	uget_plugin_agent_global_get(UGET_PLUGIN_AGENT_DEFAULT_PLUGIN, &plugin_info);

	// create target_plugin to download
	plugin->target_plugin = uget_plugin_new(plugin_info);
	uget_plugin_ctrl_speed(plugin->target_plugin, plugin->limit);
	if (uget_plugin_start(plugin->target_plugin, plugin->target_node) == FALSE) {
		msg = uget_event_new_error(UGET_EVENT_ERROR_THREAD_CREATE_FAILED,
		                           NULL);
		uget_plugin_post((UgetPlugin*) plugin, msg);
		goto exit;
	}

	do {
		// sleep 0.5 second
		ug_sleep(500);
		// stop target_plugin when user paused this plug-in.
		if (plugin->paused) {
			uget_plugin_stop(plugin->target_plugin);
			break;
		}
		if (plugin->limit_changed) {
			plugin->limit_changed = FALSE;
			uget_plugin_ctrl_speed(plugin->target_plugin, plugin->limit);
		}

		// move event from target_plugin to plug-in
		msg = uget_plugin_pop((UgetPlugin*) plugin->target_plugin);
		for (;  msg;  msg = msg_next) {
			msg_next = msg->next;
			msg->prev = NULL;
			msg->next = NULL;

			// handle or discard some message
			switch (msg->type) {
			case UGET_EVENT_ERROR:
				// stop downloading if error occurred
				plugin->paused = TRUE;
				break;

			case UGET_EVENT_NORMAL:
				// ignore "not resumable" event
				if (msg->value.code == UGET_EVENT_NORMAL_NOT_RESUMABLE)
					uget_event_free(msg);
				continue;

			case UGET_EVENT_STOP:
			case UGET_EVENT_COMPLETED:
				// discard message
				uget_event_free(msg);
				continue;
			}
			// post event to plug-in
			uget_plugin_post((UgetPlugin*) plugin, msg);
		}
		// sync data in plugin_sync()
		plugin->synced = FALSE;
	} while (uget_plugin_agent_sync_plugin((UgetPluginAgent*) plugin, NULL));

	// free target_plugin
	uget_plugin_unref((UgetPlugin*) plugin->target_plugin);
	plugin->target_plugin = NULL;

	// if downloading completed, decrypt file
	if (plugin->paused == FALSE)
		mega_decrypt_file(plugin, FALSE);
exit:
	plugin->synced = FALSE;
	plugin->stopped = TRUE;
	uget_plugin_post((UgetPlugin*)plugin,
			uget_event_new(UGET_EVENT_STOP));
	uget_plugin_unref((UgetPlugin*) plugin);
	return UG_THREAD_RETURN_VALUE;
}

static int  plugin_sync(UgetPluginMega* plugin, UgetNode* node)
{
	UgetCommon*    common;
	UgetProgress*  progress;
	char*  str;

	if (plugin->stopped) {
		if (plugin->synced)
			return FALSE;
		plugin->synced = TRUE;
	}
	// avoid crash if plug-in plug-in failed to start.
	if (plugin->node == NULL)
		return TRUE;
	if (node == NULL)
		node = plugin->node;

	// --------------------------------
	// sync data between plugin->node and plugin->target_node

	// sync common data (include speed limit) between node and target_node
	common = ug_info_realloc(node->info, UgetCommonInfo);
	uget_plugin_agent_sync_common((UgetPluginAgent*) plugin,
	                              common, plugin->target_common);

	// sync progress data from target_node to node
	progress = ug_info_realloc(node->info, UgetProgressInfo);
	uget_plugin_agent_sync_progress((UgetPluginAgent*) plugin,
	                                progress, plugin->target_progress);
	if (plugin->decrypting == FALSE)
		progress->percent = progress->percent * 96 / 100;

	// sync child nodes from target_node to node
	uget_plugin_agent_sync_children((UgetPluginAgent*) plugin,
	                                (plugin->stopped) ? FALSE : TRUE);

	// change child node's name if decrypting completed.
	if (plugin->stopped && plugin->decrypting) {
		if (plugin->node->children && plugin->node->children->name) {
			str = strstr(plugin->node->children->name, ".enc");
			if (str != NULL)
				str[0] = 0;
		}
	}

	// plug-in has got file name from server.
	if (plugin->named) {
		plugin->named = FALSE;
		ug_free(node->name);
		node->name = ug_strdup(plugin->file);
		ug_free(common->file);
		common->file = ug_strdup(plugin->file);
	}

	// if plug-in was stopped, return FALSE.
	return TRUE;
}

static int  is_downloaded(UgetPluginMega* plugin, UgetCommon* target_common)
{
	char* path;
	char* temp;

	// check existed file
	if (target_common->folder == NULL)
		path = ug_strdup(target_common->file);
	else
		path = ug_build_filename(target_common->folder, target_common->file, NULL);

	if (ug_file_is_exist(path)) {
		temp = path;
		path = ug_strdup_printf("%s.aria2", path);
		ug_free(temp);
		if (ug_file_is_exist(path) == FALSE) {
			ug_free(path);
			return TRUE;
		}
		ug_free(path);
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// MEGA site

static void xor_(uint8_t* dest, uint8_t* src1, uint8_t* src2, int length)
{
	for (; length > 0;  length--)
		*dest++ = *src1++ ^ *src2++;
}

static int  mega_parse_url(UgetPluginMega* plugin, const char* url)
{
	uint8_t* binary_key;
	int      length;

	plugin->id = strchr(url, '!');
	if (plugin->id != NULL) {
		// folder
		if (plugin->id != url && *(plugin->id-1) == 'F')
			return MEGA_FOLDER;
		// file
		plugin->id++;
		if (plugin->id[0] == 0)
			return MEGA_UNKNOWN;
	}

	plugin->key = strchr(plugin->id, '!');
	if (plugin->key != NULL) {
		plugin->key++;
		if (plugin->key[0] == 0)
			return MEGA_UNKNOWN;
	}

	// copy string from URL
	plugin->id  = ug_strndup(plugin->id, plugin->key - plugin->id - 1);
	plugin->key = ug_strdup(plugin->key);
	ug_str_replace_chars(plugin->key, "-", '+');
	ug_str_replace_chars(plugin->key, "_", '/');
	ug_str_remove_chars(plugin->key, plugin->key, ",");
	ug_str_remove_chars(plugin->key, plugin->key, "\n");

	binary_key = ug_base64_decode(plugin->key, strlen(plugin->key), &length);
	if (length < 32) {
		ug_free(binary_key);
		return MEGA_UNKNOWN;
	}

	plugin->key = ug_realloc(plugin->key, 16);
	xor_((uint8_t*)plugin->key+0, binary_key+0, binary_key+16, 8);
	xor_((uint8_t*)plugin->key+8, binary_key+8, binary_key+24, 8);

	plugin->iv = ug_malloc(16);
	memcpy(plugin->iv, binary_key+16, 8);
	memset(plugin->iv+8, 0, 8);
	return MEGA_FILE;
}

// ------------------------------------
// MEGA attributes

//	MEGA{
//		"c":"Yy6d4TsrLpaGN0NwGKf_gwRqgYlZ",
//		"n":"filename.ext"
//	}

// n  is filename

static int  mega_parse_attributes(UgetPluginMega* plugin, char* attributes)
{
	UgValue* member;
	char* iv;
	char* attr;
	char* buffer;
	int   length;

	ug_str_replace_chars(attributes, "-", '+');
	ug_str_replace_chars(attributes, "_", '/');
	ug_str_remove_chars(attributes, attributes, ",");
	ug_str_remove_chars(attributes, attributes, "\n");
	buffer = (char*)ug_base64_decode(attributes, strlen(attributes), &length);
	iv = ug_malloc0(16);
	attr = NULL;

#ifdef USE_OPENSSL
	{
		AES_KEY  key;

		attr = ug_malloc(length);
		AES_set_decrypt_key((uint8_t*)plugin->key, 128, &key);
//		AES_cbc_decrypt(temp, attr, length, &key, iv, AES_DECRYPT);
		CRYPTO_cbc128_decrypt((uint8_t*)buffer, (uint8_t*)attr, length,
		                      &key, (uint8_t*)iv, (block128_f)AES_decrypt);
	}
#endif  // USE_OPENSSL

#ifdef USE_GNUTLS
	{
		gcry_cipher_hd_t  gchd;

		gcry_cipher_open(&gchd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, 0);
		gcry_cipher_setkey(gchd, plugin->key, 16);
		gcry_cipher_setiv(gchd, iv, 16);
		gcry_cipher_decrypt(gchd, buffer, length, NULL, 0);
		gcry_cipher_close(gchd);
		attr = buffer;
		buffer = NULL;
	}
#endif // USE_GNUTLS

	ug_free(iv);
	ug_free(buffer);

#ifndef NDEBUG
	printf("%.*s\n", length, attr);
#endif

	// search JSON object
	buffer = strchr(attr, '{');
	if (buffer == NULL) {
		ug_free(attr);
		return FALSE;
	}
	length -= buffer - attr;

	// parse JSON object
	ug_value_clear(&plugin->value);
	ug_json_begin_parse(&plugin->json);
	ug_json_push(&plugin->json, ug_json_parse_value, &plugin->value, NULL);
	ug_json_parse(&plugin->json, buffer, length);
	ug_json_end_parse(&plugin->json);
	ug_free(attr);

	if (plugin->value.type != UG_VALUE_OBJECT)
		return FALSE;
	ug_value_sort(&plugin->value, ug_value_compare_name);

	// get file name
	member = ug_value_find_name(&plugin->value, "n");
	if (member == NULL || member->type != UG_VALUE_STRING)
		return FALSE;
	plugin->file = ug_strdup(member->c.string);

	return TRUE;
}

// ------------------------------------
// MEGA result

// [-9] = doesn't exist?
//
// [
//   {
//     "s":61297757,
//     "at":"m0n8BXaUMWAU0E62cXP6W7dzE3VZQL-luEZJmnRnbJQPR0RoI9ln720tB3xU4fQPpUzdtm2L6mFUFDVJSljHpum8LsAMZnKTo3ANWEcNIOI9mTAzXTp6_Hg7kyqSIkkV",
//     "msd":1,
//     "tl":0,
//     "g":"http://gfs270n155.userstorage.mega.co.nz/dl/44BKOuGpTz7mogJ8dP7I5tFynclZmBl6aJCoF6E3bqPvmy5SjM0u4qxxzxlvEf0s-Y7Yj3IEKzA8zsrHweCDGlyPvdX6DJc4vX6U9-M4xweMdUM-aiVtqsp8pr3mGw"
//   }
// ]

// at is attributes
// g  is download URL
// s  is size

static size_t curl_output_mega_result(char* text, size_t size,
                                      size_t nmemb, UgetPluginMega* plugin)
{
	size *= nmemb;
	ug_json_parse(&plugin->json, text, size);

#ifndef NDEBUG
	printf("%.*s\n", size, text);
#endif

	return size;
}

static int  mega_request_info(UgetPluginMega* plugin, const char* id)
{
	CURL*        curl;
	CURLcode     code;
	UgValue*     member;
	char*        string;

	ug_json_begin_parse(&plugin->json);
	ug_json_push(&plugin->json, ug_json_parse_value, &plugin->value, NULL);
	ug_json_push(&plugin->json, ug_json_parse_array, NULL, NULL);

	// setup option
	string = ug_strdup_printf("[{\"a\":\"g\",\"g\":1,\"p\":\"%s\"}]", id);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://eu.api.mega.co.nz/cs");
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, string);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(string));
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_output_mega_result);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, plugin);
	ug_curl_set_proxy(curl, plugin->target_proxy);
	code = curl_easy_perform(curl);
	ug_free(string);

	if (code != CURLE_OK)
		return FALSE;

	if (ug_json_end_parse(&plugin->json) != UG_JSON_ERROR_NONE)
		return FALSE;
	if (plugin->value.type != UG_VALUE_OBJECT)
		return FALSE;
	ug_value_sort(&plugin->value, ug_value_compare_name);

	// get download URL
	member = ug_value_find_name(&plugin->value, "g");
	if (member == NULL || member->type != UG_VALUE_STRING)
		return FALSE;
	plugin->url = ug_strdup(member->c.string);

	// get attributes
	member = ug_value_find_name(&plugin->value, "at");
	if (member == NULL || member->type != UG_VALUE_STRING)
		return FALSE;
	// mega_parse_attributes() will call ug_value_clear(&plugin->value);
	mega_parse_attributes(plugin, member->c.string);

	return TRUE;
}

int  mega_decrypt_file(UgetPluginMega* plugin, int preset_progress)
{
	UgetCommon* target_common;
	char* path;
	FILE *file_in, *file_out;

	target_common = plugin->target_common;
	// decrypt output file ---
	if (target_common->folder == NULL)
		path = ug_strdup(plugin->file);
	else
		path = ug_build_filename(target_common->folder, plugin->file, NULL);
	file_out = ug_fopen(path, "wb");
	ug_free(path);
	if (file_out == NULL)
		return FALSE;
	// decrypt input file ---
	if (target_common->folder == NULL)
		path = ug_strdup(target_common->file);
	else
		path = ug_build_filename(target_common->folder, target_common->file, NULL);
	file_in  = ug_fopen(path, "rb");
	if (file_in == NULL) {
		ug_free(path);
		fclose(file_out);
		return FALSE;
	}

	// preset progress before decrypting
	if (preset_progress == TRUE) {
		fseek(file_in, 0L, SEEK_END);
		plugin->target_progress->percent = 96;
		plugin->target_progress->complete = ftell(file_in);
		plugin->target_progress->total = plugin->target_progress->complete;
		fseek(file_in, 0L, SEEK_SET);    // rewind(file_in);
	}

	plugin->synced = FALSE;
	plugin->decrypting = TRUE;
	uget_plugin_post((UgetPlugin*) plugin,
			uget_event_new_normal(0, _("decrypting file...")));

#ifdef USE_OPENSSL
	{
		AES_KEY  aeskey;
		int      length;
		unsigned int   num;
		unsigned char* data_in;
		unsigned char* data_out;
		unsigned char* ecount_buf;

		data_in    = ug_malloc(AES_BLOCK_SIZE * 3);
		data_out   = data_in + AES_BLOCK_SIZE;
		ecount_buf = data_out + AES_BLOCK_SIZE;

		// set to zeros before the first call to ctr128_encrypt
		memset(ecount_buf, 0, AES_BLOCK_SIZE);
		num = 0;

		// CTR mode doesn't need separate encrypt and decrypt method.
		AES_set_encrypt_key((uint8_t*)plugin->key, 128, &aeskey);

		while (1) {
			length = fread(data_in, 1, AES_BLOCK_SIZE, file_in);

	#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			CRYPTO_ctr128_encrypt(data_in, data_out, length,
					&aeskey, (uint8_t*)plugin->iv, ecount_buf, &num,
							(block128_f)AES_encrypt);
	#else
			AES_ctr128_encrypt(data_in, data_out, length,
					&aeskey, (uint8_t*)plugin->iv, ecount_buf, &num);
	#endif

			fwrite(data_out, 1, length, file_out);

			// decrypting progress
			plugin->target_progress->complete = ftell(file_out);
			plugin->target_progress->percent = 96 +
					plugin->target_progress->complete * 4 / plugin->target_progress->total;
			plugin->synced = FALSE;
			// check EOF
			if (length < AES_BLOCK_SIZE)
				break;
		}

		ug_free(data_in);
	}
#endif  // USE_OPENSSL

#ifdef USE_GNUTLS
	{
		char*  buffer;
		int    length;
		gcry_cipher_hd_t  gchd;

		// CTR mode doesn't need separate encrypt and decrypt method.
		gcry_cipher_open(&gchd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CTR, 0);
		gcry_cipher_setkey(gchd, plugin->key, 16);
		gcry_cipher_setiv(gchd, plugin->iv, 16);
		gcry_cipher_setctr(gchd, plugin->iv, 16);  // counter vector

		buffer = ug_malloc(16);
		while (1) {
			length = fread(buffer, 1, 16, file_in);
			gcry_cipher_encrypt(gchd, buffer, length, NULL, 0);
			fwrite(buffer, 1, length, file_out);
			// decrypting progress
			plugin->target_progress->complete = ftell(file_out);
			plugin->target_progress->percent = 96 +
					plugin->target_progress->complete * 4 / plugin->target_progress->total;
			plugin->synced = FALSE;
			// check EOF
			if (length < 16)
				break;
		}
		ug_free(buffer);

		gcry_cipher_close(gchd);
	}
#endif  // USE_GNUTLS

	fclose(file_out);
	fclose(file_in);

	// decryption completed
	ug_remove(path);
	ug_free(path);
	plugin->target_progress->percent = 100;
	plugin->node->group |= UGET_GROUP_COMPLETED;
	// post message
	uget_plugin_post((UgetPlugin*) plugin,
			uget_event_new_normal(0, _("decryption completed")));
	uget_plugin_post((UgetPlugin*) plugin,
			uget_event_new(UGET_EVENT_COMPLETED));

	return TRUE;
}

