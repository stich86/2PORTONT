/* MiniDLNA media server
 * Copyright (C) 2009  Justin Maggard
 *
 * This file is part of MiniDLNA.
 *
 * MiniDLNA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * MiniDLNA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MiniDLNA. If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"
#ifdef TIVO_SUPPORT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tivo_utils.h"
#include "upnpglobalvars.h"
#include "upnphttp.h"
#include "upnpsoap.h"
#include "utils.h"
#include "sql.h"
#include "log.h"

void
SendRootContainer(struct upnphttp * h)
{
	char * resp;
	int len;

	len = asprintf(&resp, "<?xml version='1.0' encoding='UTF-8' ?>\n"
			"<TiVoContainer>"
			 "<Details>"
			  "<ContentType>x-container/tivo-server</ContentType>"
			  "<SourceFormat>x-container/folder</SourceFormat>"
			  "<TotalDuration>0</TotalDuration>"
			  "<TotalItems>3</TotalItems>"
			  "<Title>%s</Title>"
			 "</Details>"
			 "<ItemStart>0</ItemStart>"
			 "<ItemCount>3</ItemCount>"
			 "<Item>"
			  "<Details>"
			   "<ContentType>x-container/tivo-photos</ContentType>"
			   "<SourceFormat>x-container/folder</SourceFormat>"
			   "<Title>Pictures on %s</Title>"
			  "</Details>"
			  "<Links>"
			   "<Content>"
			    "<Url>/TiVoConnect?Command=QueryContainer&amp;Container=3</Url>"
			   "</Content>"
			  "</Links>"
			 "</Item>"
			 "<Item>"
			  "<Details>"
			   "<ContentType>x-container/tivo-music</ContentType>"
			   "<SourceFormat>x-container/folder</SourceFormat>"
			   "<Title>Music on %s</Title>"
			  "</Details>"
			  "<Links>"
			   "<Content>"
			    "<Url>/TiVoConnect?Command=QueryContainer&amp;Container=1</Url>"
			   "</Content>"
			  "</Links>"
			 "</Item>"
			 "<Item>"
			  "<Details>"
			   "<ContentType>x-container/tivo-videos</ContentType>"
			   "<SourceFormat>x-container/folder</SourceFormat>"
			   "<Title>Videos on %s</Title>"
			  "</Details>"
			  "<Links>"
			   "<Content>"
			    "<Url>/TiVoConnect?Command=QueryContainer&amp;Container=2</Url>"
	                    "<ContentType>x-container/tivo-videos</ContentType>"
			   "</Content>"
			  "</Links>"
			 "</Item>"
			"</TiVoContainer>", friendly_name, friendly_name, friendly_name, friendly_name);
	BuildResp_upnphttp(h, resp, len);
	free(resp);
	SendResp_upnphttp(h);
}

char *
unescape_tag(char * tag)
{
	modifyString(tag, "&amp;amp;", "&amp;", 0);
	modifyString(tag, "&amp;amp;lt;", "&lt;", 0);
	modifyString(tag, "&amp;lt;", "&lt;", 0);
	modifyString(tag, "&amp;amp;gt;", "&gt;", 0);
	modifyString(tag, "&amp;gt;", "&gt;", 0);
	return tag;
}

#define FLAG_SEND_RESIZED  0x01
#define FLAG_NO_PARAMS     0x02
#define FLAG_VIDEO         0x04
int callback(void *args, int argc, char **argv, char **azColName)
{
	struct Response *passed_args = (struct Response *)args;
	char *id = argv[0], *class = argv[1], *detailID = argv[2], *size = argv[3], *title = argv[4], *duration = argv[5],
             *bitrate = argv[6], *sampleFrequency = argv[7], *artist = argv[8], *album = argv[9], *genre = argv[10],
             *comment = argv[11], *date = argv[12], *resolution = argv[13], *mime = argv[14], *path = argv[15];
	char str_buf[4096];
	int ret = 0, flags = 0, count;

	if( strncmp(class, "item", 4) == 0 )
	{
		unescape_tag(title);
		if( strncmp(mime, "audio", 5) == 0 )
		{
			flags |= FLAG_NO_PARAMS;
			ret = sprintf(str_buf, "<Item><Details>"
			                       "<ContentType>audio/*</ContentType>"
			                       "<SourceFormat>%s</SourceFormat>"
			                       "<SourceSize>%s</SourceSize>"
			                       "<SongTitle>%s</SongTitle>", mime, size, title);
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
			if( date )
			{
				ret = sprintf(str_buf, "<AlbumYear>%.*s</AlbumYear>", 4, date);
				memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
				passed_args->size += ret;
			}
		}
		else if( strcmp(mime, "image/jpeg") == 0 )
		{
			flags |= FLAG_SEND_RESIZED;
			ret = sprintf(str_buf, "<Item><Details>"
			                       "<ContentType>image/*</ContentType>"
			                       "<SourceFormat>image/jpeg</SourceFormat>"
			                       "<SourceSize>%s</SourceSize>", size);
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
			if( date )
			{
				struct tm tm;
				memset(&tm, 0, sizeof(tm));
				tm.tm_isdst = -1; // Have libc figure out if DST is in effect or not
				strptime(date, "%Y-%m-%dT%H:%M:%S", &tm);
				ret = sprintf(str_buf, "<CaptureDate>0x%X</CaptureDate>", (unsigned int)mktime(&tm));
				memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
				passed_args->size += ret;
			}
			if( comment )
			{
				ret = sprintf(str_buf, "<Caption>%s</Caption>", comment);
				memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
				passed_args->size += ret;
			}
		}
		else if( strncmp(mime, "video", 5) == 0 )
		{
			char *episode;
			flags |= FLAG_NO_PARAMS;
			flags |= FLAG_VIDEO;
			ret = sprintf(str_buf, "<Item><Details>"
			                       "<ContentType>video/x-tivo-mpeg</ContentType>"
			                       "<SourceFormat>%s</SourceFormat>"
			                       "<SourceSize>%s</SourceSize>", mime, size);
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
			episode = strstr(title, " - ");
			if( episode )
			{
				ret = sprintf(str_buf, "<Title>%.*s</Title>"
				                       "<EpisodeTitle>%s</EpisodeTitle>", 
				                       episode-title, title, episode+3);
			}
			else
			{
				ret = sprintf(str_buf, "<Title>%s</Title>", title);
			}
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
			if( date )
			{
				struct tm tm;
				memset(&tm, 0, sizeof(tm));
				tm.tm_isdst = -1; // Have libc figure out if DST is in effect or not
				strptime(date, "%Y-%m-%dT%H:%M:%S", &tm);
				ret = sprintf(str_buf, "<CaptureDate>0x%X</CaptureDate>", (unsigned int)mktime(&tm));
				memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
				passed_args->size += ret;
			}
			if( comment )
			{
				ret = sprintf(str_buf, "<Description>%s</Description>", comment);
				memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
				passed_args->size += ret;
			}
		}
		else
		{
			return 0;
		}
		ret = sprintf(str_buf, "<Title>%s</Title>", unescape_tag(title));
		memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
		passed_args->size += ret;
		if( artist ) {
			ret = sprintf(str_buf, "<ArtistName>%s</ArtistName>", unescape_tag(artist));
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
		}
		if( album ) {
			ret = sprintf(str_buf, "<AlbumTitle>%s</AlbumTitle>", unescape_tag(album));
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
		}
		if( genre ) {
			ret = sprintf(str_buf, "<MusicGenre>%s</MusicGenre>", unescape_tag(genre));
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
		}
		if( resolution ) {
			char *width = strsep(&resolution, "x");
			ret = sprintf(str_buf, "<SourceWidth>%s</SourceWidth>"
			                       "<SourceHeight>%s</SourceHeight>",
			              width, resolution);
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
		}
		if( duration ) {
			ret = sprintf(str_buf, "<Duration>%d</Duration>",
			      atoi(rindex(duration, '.')+1) + (1000*atoi(rindex(duration, ':')+1)) + (60000*atoi(rindex(duration, ':')-2)) + (3600000*atoi(duration)));
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
		}
		if( bitrate ) {
			ret = sprintf(str_buf, "<SourceBitRate>%s</SourceBitRate>", bitrate);
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
		}
		if( sampleFrequency ) {
			ret = sprintf(str_buf, "<SourceSampleRate>%s</SourceSampleRate>", sampleFrequency);
			memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
			passed_args->size += ret;
		}
		ret = sprintf(str_buf, "</Details><Links><Content>"
		                       "<ContentType>%s</ContentType>"
		                       "<Url>/%s/%s.dat</Url>%s</Content>",
		                       mime,
		                       (flags & FLAG_SEND_RESIZED)?"Resized":"MediaItems", detailID,
		                       (flags & FLAG_NO_PARAMS)?"<AcceptsParams>No</AcceptsParams>":"");
		memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
		passed_args->size += ret;
		if( flags & FLAG_VIDEO )
		{
			char *name = basename(path);
			char *esc_name = escape_tag(name);
			ret = sprintf(str_buf, "<CustomIcon>"
			                         "<ContentType>video/*</ContentType>"
			                         "<Url>urn:tivo:image:save-until-i-delete-recording</Url>"
			                       "</CustomIcon>"
			                       "<Push><Container>Videos</Container></Push>"
			                       "<File>%s</File> </Links>", esc_name?esc_name:name);
			if( esc_name )
				free(esc_name);
		}
		else
		{
			ret = sprintf(str_buf, "</Links>");
		}
	}
	else if( strncmp(class, "container", 9) == 0 )
	{
		/* Determine the number of children */
#ifdef __sparc__ /* Adding filters on large containers can take a long time on slow processors */
		count = sql_get_int_field(db, "SELECT count(*) from OBJECTS where PARENT_ID = '%s'", id);
#else
		count = sql_get_int_field(db, "SELECT count(*) from OBJECTS o left join DETAILS d on (d.ID = o.DETAIL_ID) where PARENT_ID = '%s' and "
		                              " (MIME in ('image/jpeg', 'audio/mpeg', 'video/mpeg', 'video/x-tivo-mpeg')"
		                              " or CLASS glob 'container*')", id);
#endif
		ret = sprintf(str_buf, "<Item>"
		                         "<Details>"
		                           "<ContentType>x-container/folder</ContentType>"
		                           "<SourceFormat>x-container/folder</SourceFormat>"
		                           "<Title>%s</Title>"
		                           "<TotalItems>%d</TotalItems>"
		                         "</Details>"
		                         "<Links>"
		                           "<Content>"
		                             "<Url>/TiVoConnect?Command=QueryContainer&amp;Container=%s</Url>"
		                             "<ContentType>x-tivo-container/folder</ContentType>"
		                           "</Content>"
		                         "</Links>",
		                         unescape_tag(title), count, id);
	}
	memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
	passed_args->size += ret;
	ret = sprintf(str_buf, "</Item>");
	memcpy(passed_args->resp+passed_args->size, &str_buf, ret+1);
	passed_args->size += ret;

	passed_args->returned++;

	return 0;
}

void
SendItemDetails(struct upnphttp * h, sqlite_int64 item)
{
	char * resp = malloc(32768);
	char *sql;
	char *zErrMsg = NULL;
	struct Response args;
	int ret;
	memset(&args, 0, sizeof(args));

	args.resp = resp;
	args.size = sprintf(resp, "<?xml version='1.0' encoding='UTF-8' ?>\n<TiVoItem>");
	args.requested = 1;
	asprintf(&sql, "SELECT o.OBJECT_ID, o.CLASS, o.DETAIL_ID, d.SIZE, d.TITLE,"
	               " d.DURATION, d.BITRATE, d.SAMPLERATE, d.ARTIST, d.ALBUM,"
	               " d.GENRE, d.COMMENT, d.DATE, d.RESOLUTION, d.MIME, d.PATH, d.TRACK "
	               "from OBJECTS o left join DETAILS d on (d.ID = o.DETAIL_ID)"
		       " where o.DETAIL_ID = %lld group by o.DETAIL_ID", item);
	DPRINTF(E_DEBUG, L_TIVO, "%s\n", sql);
	ret = sqlite3_exec(db, sql, callback, (void *) &args, &zErrMsg);
	free(sql);
	if( ret != SQLITE_OK )
	{
		DPRINTF(E_ERROR, L_HTTP, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	strcat(resp, "</TiVoItem>");

	BuildResp_upnphttp(h, resp, strlen(resp));
	free(resp);
	SendResp_upnphttp(h);
}

void
SendContainer(struct upnphttp * h, const char * objectID, int itemStart, int itemCount, char * anchorItem,
              int anchorOffset, int recurse, char * sortOrder, char * filter, unsigned long int randomSeed)
{
	char * resp = malloc(262144);
	char *sql, *item, *saveptr;
	char *zErrMsg = NULL;
	char **result;
	char *title = NULL;
	char what[10], order[64]={0}, order2[64]={0}, myfilter[256]={0};
	char str_buf[1024];
	char *which;
	char type[8];
	char groupBy[19] = {0};
	struct Response args;
	int totalMatches = 0;
	int i, ret;
	memset(&args, 0, sizeof(args));
	memset(resp, 0, sizeof(262144));

	args.resp = resp;
	args.size = 1024;
	if( itemCount >= 0 )
	{
		args.requested = itemCount;
	}
	else
	{
		if( itemCount == -100 )
			itemCount = 1;
		args.requested = itemCount * -1;
	}

	switch( *objectID )
	{
		case '1':
			strcpy(type, "music");
			break;
		case '2':
			strcpy(type, "videos");
			break;
		case '3':
			strcpy(type, "photos");
			break;
		default:
			strcpy(type, "server");
			break;
	}

	if( strlen(objectID) == 1 )
	{
		switch( *objectID )
		{
			case '1':
				asprintf(&title, "Music on %s", friendly_name);
				break;
			case '2':
				asprintf(&title, "Videos on %s", friendly_name);
				break;
			case '3':
				asprintf(&title, "Pictures on %s", friendly_name);
				break;
			default:
				asprintf(&title, "Unknown on %s", friendly_name);
				break;
		}
	}
	else
	{
		sql = sqlite3_mprintf("SELECT NAME from OBJECTS where OBJECT_ID = '%s'", objectID);
		if( (sql_get_table(db, sql, &result, &ret, NULL) == SQLITE_OK) && ret )
		{
			title = escape_tag(result[1]);
			if( !title )
				title = strdup(result[1]);
		}
		else
			title = strdup("UNKNOWN");
		sqlite3_free(sql);
		sqlite3_free_table(result);
	}

	if( recurse )
	{
		asprintf(&which, "OBJECT_ID glob '%s$*'", objectID);
		strcpy(groupBy, "group by DETAIL_ID");
	}
	else
	{
		asprintf(&which, "PARENT_ID = '%s'", objectID);
	}

	if( sortOrder )
	{
		if( strcasestr(sortOrder, "Random") )
		{
			sprintf(order, "tivorandom(%lu)", randomSeed);
			if( itemCount < 0 )
				sprintf(order2, "tivorandom(%lu) DESC", randomSeed);
			else
				sprintf(order2, "tivorandom(%lu)", randomSeed);
		}
		else
		{
			short track_done = 0;
			item = strtok_r(sortOrder, ",", &saveptr);
			for( i=0; item != NULL; i++ )
			{
				int reverse=0;
				if( *item == '!' )
				{
					reverse = 1;
					item++;
				}
				if( strcasecmp(item, "Type") == 0 )
				{
					strcat(order, "CLASS");
					strcat(order2, "CLASS");
				}
				else if( strcasecmp(item, "Title") == 0 )
				{
					/* Explicitly sort music by track then title. */
					if( !track_done && *objectID == '1' )
					{
						strcat(order, "TRACK");
						strcat(order2, "TRACK");
						track_done = 1;
					}
					else
					{
						strcat(order, "TITLE");
						strcat(order2, "TITLE");
						track_done = 0;
					}
				}
				else if( strcasecmp(item, "CreationDate") == 0 ||
				         strcasecmp(item, "CaptureDate") == 0 )
				{
					strcat(order, "DATE");
					strcat(order2, "DATE");
				}
				else
				{
					DPRINTF(E_INFO, L_TIVO, "Unhandled SortOrder [%s]\n", item);
					goto unhandled_order;
				}

				if( reverse )
				{
					strcat(order, " DESC");
					if( itemCount >= 0 )
						strcat(order2, " DESC");
					else
						strcat(order2, " ASC");
				}
				else
				{
					strcat(order, " ASC");
					if( itemCount >= 0 )
						strcat(order2, " ASC");
					else
						strcat(order2, " DESC");
				}
				strcat(order, ", ");
				strcat(order2, ", ");
				unhandled_order:
				if( !track_done )
					item = strtok_r(NULL, ",", &saveptr);
			}
			strcat(order, "TITLE ASC, DETAIL_ID ASC");
			if( itemCount >= 0 )
				strcat(order2, "TITLE ASC, DETAIL_ID ASC");
			else
				strcat(order2, "TITLE DESC, DETAIL_ID DESC");
		}
	}
	else
	{
		sprintf(order, "CLASS, NAME, DETAIL_ID");
		if( itemCount < 0 )
			sprintf(order2, "CLASS DESC, NAME DESC, DETAIL_ID DESC");
		else
			sprintf(order2, "CLASS, NAME, DETAIL_ID");
	}

	if( filter )
	{
		item = strtok_r(filter, ",", &saveptr);
		for( i=0; item != NULL; i++ )
		{
			if( i )
			{
				strcat(myfilter, " or ");
			}
			if( (strcasecmp(item, "x-container/folder") == 0) ||
			    (strncasecmp(item, "x-tivo-container/", 17) == 0) )
			{
				strcat(myfilter, "CLASS glob 'container*'");
			}
			else if( strncasecmp(item, "image", 5) == 0 )
			{
				strcat(myfilter, "MIME = 'image/jpeg'");
			}
			else if( strncasecmp(item, "audio", 5) == 0 )
			{
				strcat(myfilter, "MIME = 'audio/mpeg'");
			}
			else if( strncasecmp(item, "video", 5) == 0 )
			{
				strcat(myfilter, "MIME in ('video/mpeg', 'video/x-tivo-mpeg')");
			}
			else
			{
				DPRINTF(E_INFO, L_TIVO, "Unhandled Filter [%s]\n", item);
				if( i )
				{
					ret = strlen(myfilter);
					myfilter[ret-4] = '\0';
				}
				i--;
			}
			item = strtok_r(NULL, ",", &saveptr);
		}
	}
	else
	{
		strcpy(myfilter, "MIME in ('image/jpeg', 'audio/mpeg', 'video/mpeg', 'video/x-tivo-mpeg') or CLASS glob 'container*'");
	}

	if( anchorItem )
	{
		if( strstr(anchorItem, "QueryContainer") )
		{
			strcpy(what, "OBJECT_ID");
			anchorItem = rindex(anchorItem, '=')+1;
		}
		else
		{
			strcpy(what, "DETAIL_ID");
		}
		sqlite3Prng.isInit = 0;
		sql = sqlite3_mprintf("SELECT %s from OBJECTS o left join DETAILS d on (o.DETAIL_ID = d.ID)"
		                      " where %s and (%s)"
	                              " %s"
		                      " order by %s", what, which, myfilter, groupBy, order2);
		DPRINTF(E_DEBUG, L_TIVO, "%s\n", sql);
		if( (sql_get_table(db, sql, &result, &ret, NULL) == SQLITE_OK) && ret )
		{
			for( i=1; i<=ret; i++ )
			{
				if( strcmp(anchorItem, result[i]) == 0 )
				{
					if( itemCount < 0 )
						itemStart = ret - i + itemCount;
					else
						itemStart += i;
					break;
				}
			}
			sqlite3_free_table(result);
		}
		sqlite3_free(sql);
	}
	args.start = itemStart+anchorOffset;
	sqlite3Prng.isInit = 0;

	ret = sql_get_int_field(db, "SELECT count(distinct DETAIL_ID) "
	                            "from OBJECTS o left join DETAILS d on (o.DETAIL_ID = d.ID)"
	                            " where %s and (%s)",
	                            which, myfilter);
	totalMatches = (ret > 0) ? ret : 0;
	if( itemCount < 0 && !args.start )
	{
		args.start = totalMatches + itemCount;
	}

	sql = sqlite3_mprintf("SELECT o.OBJECT_ID, o.CLASS, o.DETAIL_ID, d.SIZE, d.TITLE,"
	                      " d.DURATION, d.BITRATE, d.SAMPLERATE, d.ARTIST, d.ALBUM,"
	                      " d.GENRE, d.COMMENT, d.DATE, d.RESOLUTION, d.MIME, d.PATH, d.TRACK "
	                      "from OBJECTS o left join DETAILS d on (d.ID = o.DETAIL_ID)"
		              " where %s and (%s)"
	                      " %s"
			      " order by %s limit %d, %d",
	                      which, myfilter, groupBy, order, args.start, args.requested);
	DPRINTF(E_DEBUG, L_TIVO, "%s\n", sql);
	ret = sqlite3_exec(db, sql, callback, (void *) &args, &zErrMsg);
	sqlite3_free(sql);
	if( ret != SQLITE_OK )
	{
		DPRINTF(E_ERROR, L_HTTP, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	ret = sprintf(str_buf, "<?xml version='1.0' encoding='UTF-8' ?>\n"
			       "<TiVoContainer>"
	                         "<Details>"
	                           "<ContentType>x-container/tivo-%s</ContentType>"
	                           "<SourceFormat>x-container/folder</SourceFormat>"
	                           "<TotalItems>%d</TotalItems>"
	                           "<Title>%s</Title>"
	                         "</Details>"
	                         "<ItemStart>%d</ItemStart>"
	                         "<ItemCount>%d</ItemCount>",
	                         type, totalMatches, title, args.start, args.returned);
	args.resp = resp+1024-ret;
	memcpy(args.resp, &str_buf, ret);
	ret = sprintf(str_buf, "</TiVoContainer>");
	memcpy(resp+args.size, &str_buf, ret+1);
	args.size += ret;
	args.size -= args.resp-resp;
	free(title);
	free(which);
	BuildResp_upnphttp(h, args.resp, args.size);
	free(resp);
	SendResp_upnphttp(h);
}

void
ProcessTiVoCommand(struct upnphttp * h, const char * orig_path)
{
	char *path;
	char *key, *val;
	char *saveptr = NULL, *item;
	char *command = NULL, *container = NULL, *anchorItem = NULL;
	char *sortOrder = NULL, *filter = NULL;
	sqlite_int64 detailItem=0;
	int itemStart=0, itemCount=-100, anchorOffset=0, recurse=0;
	unsigned long int randomSeed=0;

	path = strdup(orig_path);
	DPRINTF(E_DEBUG, L_GENERAL, "Processing TiVo command %s\n", path);

	item = strtok_r( path, "&", &saveptr );
	while( item != NULL )
	{
		if( strlen(item) == 0 )
		{
			item = strtok_r( NULL, "&", &saveptr );
			continue;
		}
		decodeString(item, 1);
		val = item;
		key = strsep(&val, "=");
		decodeString(val, 1);
		DPRINTF(E_DEBUG, L_GENERAL, "%s: %s\n", key, val);
		if( strcasecmp(key, "Command") == 0 )
		{
			command = val;
		}
		else if( strcasecmp(key, "Container") == 0 )
		{
			container = val;
		}
		else if( strcasecmp(key, "ItemStart") == 0 )
		{
			itemStart = atoi(val);
		}
		else if( strcasecmp(key, "ItemCount") == 0 )
		{
			itemCount = atoi(val);
		}
		else if( strcasecmp(key, "AnchorItem") == 0 )
		{
			anchorItem = basename(val);
		}
		else if( strcasecmp(key, "AnchorOffset") == 0 )
		{
			anchorOffset = atoi(val);
		}
		else if( strcasecmp(key, "Recurse") == 0 )
		{
			recurse = strcasecmp("yes", val) == 0 ? 1 : 0;
		}
		else if( strcasecmp(key, "SortOrder") == 0 )
		{
			sortOrder = val;
		}
		else if( strcasecmp(key, "Filter") == 0 )
		{
			filter = val;
		}
		else if( strcasecmp(key, "RandomSeed") == 0 )
		{
			randomSeed = strtoul(val, NULL, 10);
		}
		else if( strcasecmp(key, "Url") == 0 )
		{
			if( val )
				detailItem = strtoll(basename(val), NULL, 10);
		}
		else if( strcasecmp(key, "Format") == 0 || // Only send XML
		         strcasecmp(key, "SerialNum") == 0 || // Unused for now
		         strcasecmp(key, "DoGenres") == 0 ) // Not sure what this is, so ignore it
		{
			;;
		}
		else
		{
			DPRINTF(E_DEBUG, L_GENERAL, "Unhandled parameter [%s]\n", key);
		}
		item = strtok_r( NULL, "&", &saveptr );
	}
	if( anchorItem )
	{
		strip_ext(anchorItem);
	}

	if( command )
	{
		if( strcmp(command, "QueryContainer") == 0 )
		{
			if( !container || (strcmp(container, "/") == 0) )
			{
				SendRootContainer(h);
			}
			else
			{
				SendContainer(h, container, itemStart, itemCount, anchorItem, anchorOffset, recurse, sortOrder, filter, randomSeed);
			}
		}
		else if( strcmp(command, "QueryItem") == 0 )
		{
			SendItemDetails(h, detailItem);
		}
		else
		{
			DPRINTF(E_DEBUG, L_GENERAL, "Unhandled command [%s]\n", command);
			Send501(h);
			free(path);
			return;
		}
	}
	free(path);
	CloseSocket_upnphttp(h);
}
#endif // TIVO_SUPPORT
