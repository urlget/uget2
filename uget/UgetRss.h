/*
 *
 *   Copyright (C) 2012-2017 by C.H. Huang
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

#ifndef UGET_RSS_H
#define UGET_RSS_H

//#include <time.h>
#include <UgList.h>
#include <UgHtml.h>
#include <UgEntry.h>
#include <UgThread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgetRss        UgetRss;
typedef struct UgetRssFeed    UgetRssFeed;
typedef struct UgetRssItem    UgetRssItem;

typedef enum {
	UGET_RSS_USER      = 0,

	UGET_RSS_STABLE    = 1,    // http://feeds.feedburner.com/uget/stable?format=xml
	UGET_RSS_DEVELMENT = 2,    // http://feeds.feedburner.com/uget/development?format=xml
	UGET_RSS_NEWS      = 3,    // http://feeds.feedburner.com/uget/news?format=xml
	UGET_RSS_TUTORIALS = 4,    // http://feeds.feedburner.com/uget/tutorials?format=xml
	UGET_RSS_ALL       = 5,    // http://feeds.feedburner.com/uget/all?format=xml
} UgetRssType;

// ----------------------------------------------------------------------------

struct UgetRssItem
{
	UG_LINK_MEMBERS (UgetRssItem, UgetRssItem, self);
//	UgetRssItem*  self;
//	UgetRssItem*  next;
//	UgetRssItem*  prev;
	char*   title;
	char*   link;
	time_t  updated;   // pubDate
};

UgetRssItem* uget_rss_item_new (void);
void         uget_rss_item_free (UgetRssItem* item);

// ----------------------------------------------------------------------------

extern const UgEntry  UgetRssFeedEntry[];

struct UgetRssFeed
{
	UG_LINK_MEMBERS (UgetRssFeed, UgetRssFeed, self);
//	UgetRssFeed*  self;
//	UgetRssFeed*  next;
//	UgetRssFeed*  prev;
	char*   title;
	char*   link;
	time_t  updated;   // pubDate

	UgList  items;

	// user data
	char*   url;
	int     type;
	time_t  checked;   // don't care item before checked
};

UgetRssFeed*  uget_rss_feed_new (void);
void          uget_rss_feed_free (UgetRssFeed* feed);

UgetRssItem*  uget_rss_feed_find (UgetRssFeed* feed, time_t after_time);
void          uget_rss_feed_clear (UgetRssFeed* feed, int reserve_user_data);

// ----------------------------------------------------------------------------

extern const UgEntry  UgetRssEntry[];

struct UgetRss
{
	UgThread  thread;
	UgHtml    uhtml;
	UgList    feeds;

	UgetRssFeed* checked;
	uint8_t      updating;
	int          n_updated;
	int          ref_count;
};

UgetRss*  uget_rss_new (void);
void      uget_rss_ref (UgetRss* urss);
void      uget_rss_unref (UgetRss* urss);

void  uget_rss_add (UgetRss* urss, UgetRssFeed* feed);
void  uget_rss_add_url (UgetRss* urss, const char* url);
void  uget_rss_add_builtin (UgetRss* urss, UgetRssType type);
void  uget_rss_update (UgetRss* urss, int joinable);

UgetRssFeed*  uget_rss_find_updated (UgetRss* urss, UgetRssFeed* after_feed);

int   uget_rss_save_feeds (UgetRss* urss, const char* path);
int   uget_rss_load_feeds (UgetRss* urss, const char* path);

#ifdef __cplusplus
}
#endif

#endif  // End of UGET_RSS_H

