/*
 *
 *   Copyright (C) 2012-2019 by C.H. Huang
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

#ifndef UGET_APP_H
#define UGET_APP_H

#include <UgArray.h>
#include <UgRegistry.h>
#include <UgetNode.h>
#include <UgetTask.h>
#include <UgetPlugin.h>
#include <UgetHash.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgetApp          UgetApp;

// ----------------------------------------------------------------------------
// UgetApp

#define	UGET_APP_MEMBERS  \
	UgetNode        real;           \
	UgetNode        split;          \
	UgetNode        sorted;         \
	UgetNode        sorted_split;   \
	UgetNode        mix;            \
	UgetNode        mix_split;      \
	UgRegistry      infos;          \
	UgRegistry      plugins;        \
	UgetPluginInfo* plugin_default; \
	UgetTask        task;           \
	UgArrayPtr      nodes;          \
	void*           uri_hash;       \
	char*           config_dir;     \
	int             n_error;        \
	int             n_moved;        \
	int             n_deleted;      \
	int             n_completed

void  uget_app_init (UgetApp* app);
void  uget_app_final (UgetApp* app);

// uget_app_grow() activate queue download
//                 return number of active download
int   uget_app_grow (UgetApp* app, int no_queuing);
// uget_app_trim() remove finished/recycled download that over capacity
//                 return number of trimmed download
int   uget_app_trim (UgetApp* app);

void  uget_app_set_config_dir (UgetApp* app, const char* dir);
void  uget_app_set_sorting (UgetApp* app, UgCompareFunc func, int reversed);
void  uget_app_set_notification (UgetApp* app, void* data,
                                 UgetNodeFunc inserted,
                                 UgetNodeFunc removed,
                                 UgNotifyFunc updated);

// category functions
// uget_app_move_category() return TRUE or FALSE
void  uget_app_add_category (UgetApp* app, UgetNode* cnode, int save_file);
int   uget_app_move_category (UgetApp* app, UgetNode* cnode, UgetNode* position);
void  uget_app_delete_category (UgetApp* app, UgetNode* cnode);
void  uget_app_stop_category (UgetApp* app, UgetNode* cnode);
void  uget_app_pause_category (UgetApp* app, UgetNode* cnode);
void  uget_app_resume_category (UgetApp* app, UgetNode* cnode);
UgetNode* uget_app_match_category (UgetApp* app, UgUri* uuri, const char* file);

// download functions: return TRUE or FALSE
int   uget_app_add_download_uri (UgetApp* app, const char* uri, UgetNode* cnode, int apply);
int   uget_app_add_download (UgetApp* app, UgetNode* dnode, UgetNode* cnode, int apply);
int   uget_app_move_download (UgetApp* app, UgetNode* dnode, UgetNode* dnode_position);
int   uget_app_move_download_to (UgetApp* app, UgetNode* dnode, UgetNode* cnode);
int   uget_app_delete_download (UgetApp* app, UgetNode* dnode, int delete_file);
int   uget_app_recycle_download (UgetApp* app, UgetNode* dnode);
int   uget_app_activate_download (UgetApp* app, UgetNode* dnode);
int   uget_app_pause_download (UgetApp* app, UgetNode* dnode);
int   uget_app_queue_download (UgetApp* app, UgetNode* dnode);
void  uget_app_reset_download_name (UgetApp* app, UgetNode* dnode);

#ifdef NO_URI_HASH
#define uget_app_use_uri_hash(app)
#define uget_app_clear_attachment(app)
#else
void  uget_app_use_uri_hash (UgetApp* app);
void  uget_app_clear_attachment (UgetApp* app);
#endif

// plug-in functions
// uget_app_find_plugin() return TRUE or FALSE
void  uget_app_clear_plugins (UgetApp* app);
void  uget_app_add_plugin (UgetApp* app, const UgetPluginInfo* pinfo);
void  uget_app_remove_plugin (UgetApp* app, const UgetPluginInfo* pinfo);
int   uget_app_find_plugin (UgetApp* app, const char* name, const UgetPluginInfo** pinfo);
void  uget_app_set_default_plugin (UgetApp* app, const UgetPluginInfo* pinfo);
UgetPluginInfo*  uget_app_match_plugin (UgetApp* app,
                                        const char* uri,
                                        const UgetPluginInfo* exclude);

// ----------------------------------------------------------------------------
// save/load categories

// uget_app_save_category() return TRUE or FALSE
int       uget_app_save_category (UgetApp* app, UgetNode* cnode, const char* filename, void* jsonfile);
UgetNode* uget_app_load_category (UgetApp* app, const char* filename, void* jsonfile);
int       uget_app_save_category_fd (UgetApp* app, UgetNode* cnode, int fd, void* jsonfile);
UgetNode* uget_app_load_category_fd (UgetApp* app, int fd, void* jsonfile);
// return number of category save/load
int   uget_app_save_categories (UgetApp* app, const char* folder);
int   uget_app_load_categories (UgetApp* app, const char* folder);

// ----------------------------------------------------------------------------
// keeping status

void  uget_node_set_keeping (UgetNode* node, int enable);

#ifdef __cplusplus
}
#endif

struct UgetApp
{
	UGET_APP_MEMBERS;
/*	// ------ UgetApp members ------
	UgetNode        real;           // real root node for real nodes
	UgetNode        split;          // virtual root
	UgetNode        sorted;         // virtual root
	UgetNode        sorted_split;   // virtual root
	UgetNode        mix;            // virtual root
	UgetNode        mix_split;      // virtual root
	UgRegistry      infos;
	UgRegistry      plugins;
	UgetPluginInfo* plugin_default;
	UgetTask        task;
	UgArrayPtr      nodes;
	void*           uri_hash;
	char*           config_dir;
	int             n_error;        // uget_app_grow() will count these value:
	int             n_moved;        // n_error, n_moved, n_deleted, and
	int             n_deleted;      // n_completed
	int             n_completed;    //
 */

#ifdef __cplusplus
	inline void  init(void)
		{ uget_app_init(this); }
	inline void  final(void)
		{ uget_app_final(this); }

	inline void  grow(int noQueuing)
		{ uget_app_grow(this, noQueuing); }
	inline void  trim(void)
		{ uget_app_trim(this); }

	inline void  setConfigDir(const char* dir)
		{ uget_app_set_config_dir(this, dir); }
	inline void  setSorting(UgCompareFunc func, int reversed)
		{ uget_app_set_sorting(this, func, reversed); }
	inline void  setNotification(void* data, UgetNodeFunc inserted, UgetNodeFunc removed, UgNotifyFunc updated)
		{ uget_app_set_notification(this, data, inserted, removed, updated); }

	inline void  addCategory(UgetNode* cnode, int saveFile)
		{ uget_app_add_category(this, cnode, saveFile); }
	inline void  moveCategory(UgetNode* cnode, UgetNode* position)
		{ uget_app_move_category(this, cnode, position); }
	inline void  deleteCategory(UgetNode* cnode)
		{ uget_app_delete_category(this, cnode); }
	inline void  stopCategory(UgetNode* cnode)
		{ uget_app_stop_category(this, cnode); }
	inline void  pauseCategory(UgetNode* cnode)
		{ uget_app_pause_category(this, cnode); }
	inline void  resumeCategory(UgetNode* cnode)
		{ uget_app_resume_category(this, cnode); }
	inline UgetNode*  matchCategory(UgUri* uuri, const char* file)
		{ return uget_app_match_category(this, uuri, file); }

	inline int   addDownload(const char* uri, UgetNode* cnode, int apply)
		{ return uget_app_add_download_uri(this, uri, cnode, apply); }
	inline int   addDownload(UgetNode* dnode, UgetNode* cnode, int apply)
		{ return uget_app_add_download(this, dnode, cnode, apply); }
	inline int   moveDownload(UgetNode* dnode, UgetNode* dnode_position)
		{ return uget_app_move_download(this, dnode, dnode_position); }
	inline int   moveDownloadTo(UgetNode* dnode, UgetNode* cnode)
		{ return uget_app_move_download_to(this, dnode, cnode); }
	inline int   deleteDownload(UgetNode* dnode, int deleteFile)
		{ return uget_app_delete_download(this, dnode, deleteFile); }
	inline int   recycleDownload(UgetNode* dnode)
		{ return uget_app_recycle_download(this, dnode); }
	inline int   activateDownload(UgetNode* dnode)
		{ return uget_app_activate_download(this, dnode); }
	inline int   pauseDownload(UgetNode* dnode)
		{ return uget_app_pause_download(this, dnode); }
	inline int   queueDownload(UgetNode* dnode)
		{ return uget_app_queue_download(this, dnode); }
	inline void  resetDownloadName(UgetNode* dnode)
		{ uget_app_reset_download_name(this, dnode); }

	inline void  useUriHash(void)
		{ uget_app_use_uri_hash(this); }
	inline void  clearAttachment(void)
		{ uget_app_clear_attachment(this); }

	inline void  clearPlugins(void)
		{ uget_app_clear_plugins(this); }
	inline void  addPlugin(const UgetPluginInfo* pinfo)
		{ uget_app_add_plugin(this, pinfo); }
	inline void  removePlugin(const UgetPluginInfo* pinfo)
		{ uget_app_remove_plugin(this, pinfo); }
	inline int   findPlugin(const char* name, const UgetPluginInfo** pinfo)
		{ return uget_app_find_plugin(this, name, pinfo); }
	inline void  setDefaultPlugin(const UgetPluginInfo* pinfo)
		{ return uget_app_set_default_plugin(this, pinfo); }
	inline UgetPluginInfo*  matchPlugin(const char* uri, const UgetPluginInfo* exclude)
		{ return uget_app_match_plugin(this, uri, exclude); }

	inline int   saveCategory(UgetNode* cnode, const char* filename, void* jsonfile)
		{ return uget_app_save_category(this, cnode, filename, jsonfile); }
	inline UgetNode*  loadCategory(const char* filename, void* jsonfile)
		{ return uget_app_load_category(this, filename, jsonfile); }
	// return number of category save/load
	inline int   saveCategories(const char* folder)
		{ return uget_app_save_categories(this, folder); }
	inline int   loadCategories(const char* folder)
		{ return uget_app_load_categories(this, folder); }
#endif
};

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{
typedef struct UgetApp    App;
};  // namespace Uget

#endif  // __cplusplus


#endif  // UGET_APP_H

