/* 
   Unix SMB/CIFS implementation.
   struct samu local cache for 
   Copyright (C) Jim McDonough (jmcd@us.ibm.com) 2004.
      
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "includes.h"

#undef DBGC_CLASS
#define DBGC_CLASS DBGC_PASSDB

#define LOGIN_CACHE_FILE "login_cache.tdb"

#define SAM_CACHE_FORMAT "dwwd"

static TDB_CONTEXT *cache;

bool login_cache_init(void)
{
	char* cache_fname = NULL;
	
	/* skip file open if it's already opened */
	if (cache) return True;

	asprintf(&cache_fname, "%s/%s", lp_lockdir(), LOGIN_CACHE_FILE);
	if (cache_fname)
	{	
	DEBUG(5, ("Opening cache file at %s\n", cache_fname));
	}
	else {
	{
		DEBUG(0, ("Filename allocation failed.\n"));
	}
	return False;
	}

	cache = tdb_open_log(cache_fname, 0, TDB_DEFAULT,
	                     O_RDWR|O_CREAT, 0644);

	if (!cache)
		DEBUG(5, ("Attempt to open %s failed.\n", cache_fname));

	SAFE_FREE(cache_fname);

	return (cache ? True : False);
}

bool login_cache_shutdown(void)
{
	/* tdb_close routine returns -1 on error */
	if (!cache) return False;
	DEBUG(5, ("Closing cache file\n"));
	return tdb_close(cache) != -1;
}

/* if we can't read the cache, oh well, no need to return anything */
LOGIN_CACHE * login_cache_read(struct samu *sampass)
{
	char *keystr;
	TDB_DATA databuf;
	LOGIN_CACHE *entry;

	if (!login_cache_init())
		return NULL;

	if (pdb_get_nt_username(sampass) == NULL) {
		return NULL;
	}

	keystr = SMB_STRDUP(pdb_get_nt_username(sampass));
	if (!keystr || !keystr[0]) {
		SAFE_FREE(keystr);
		return NULL;
	}

	DEBUG(7, ("Looking up login cache for user %s\n",
		  keystr));
	databuf = tdb_fetch_bystring(cache, keystr);
	SAFE_FREE(keystr);

	if (!(entry = SMB_MALLOC_P(LOGIN_CACHE))) {
		DEBUG(1, ("Unable to allocate cache entry buffer!\n"));
		SAFE_FREE(databuf.dptr);
		return NULL;
	}

	if (tdb_unpack (databuf.dptr, databuf.dsize, SAM_CACHE_FORMAT,
			&entry->entry_timestamp, &entry->acct_ctrl, 
			&entry->bad_password_count, 
			&entry->bad_password_time) == -1) {
		DEBUG(7, ("No cache entry found\n"));
		SAFE_FREE(entry);
		SAFE_FREE(databuf.dptr);
		return NULL;
	}

	SAFE_FREE(databuf.dptr);

	DEBUG(5, ("Found login cache entry: timestamp %12u, flags 0x%x, count %d, time %12u\n",
		  (unsigned int)entry->entry_timestamp, entry->acct_ctrl, 
		  entry->bad_password_count, (unsigned int)entry->bad_password_time));
	return entry;
}

bool login_cache_write(const struct samu *sampass, LOGIN_CACHE entry)
{
	char *keystr;
	TDB_DATA databuf;
	bool ret;

	if (!login_cache_init())
		return False;

	if (pdb_get_nt_username(sampass) == NULL) {
		return False;
	}

	keystr = SMB_STRDUP(pdb_get_nt_username(sampass));
	if (!keystr || !keystr[0]) {
		SAFE_FREE(keystr);
		return False;
	}

	entry.entry_timestamp = time(NULL);

	databuf.dsize = 
		tdb_pack(NULL, 0, SAM_CACHE_FORMAT,
			 entry.entry_timestamp,
			 entry.acct_ctrl,
			 entry.bad_password_count,
			 entry.bad_password_time);
	databuf.dptr = SMB_MALLOC_ARRAY(uint8, databuf.dsize);
	if (!databuf.dptr) {
		SAFE_FREE(keystr);
		return False;
	}
			 
	if (tdb_pack(databuf.dptr, databuf.dsize, SAM_CACHE_FORMAT,
			 entry.entry_timestamp,
			 entry.acct_ctrl,
			 entry.bad_password_count,
			 entry.bad_password_time)
	    != databuf.dsize) {
		SAFE_FREE(keystr);
		SAFE_FREE(databuf.dptr);
		return False;
	}

	ret = tdb_store_bystring(cache, keystr, databuf, 0);
	SAFE_FREE(keystr);
	SAFE_FREE(databuf.dptr);
	return ret == 0;
}

bool login_cache_delentry(const struct samu *sampass)
{
	int ret;
	char *keystr;
	
	if (!login_cache_init()) 
		return False;	

	if (pdb_get_nt_username(sampass) == NULL) {
		return False;
	}

	keystr = SMB_STRDUP(pdb_get_nt_username(sampass));
	if (!keystr || !keystr[0]) {
		SAFE_FREE(keystr);
		return False;
	}

	DEBUG(9, ("About to delete entry for %s\n", keystr));
	ret = tdb_delete_bystring(cache, keystr);
	DEBUG(9, ("tdb_delete returned %d\n", ret));
	
	SAFE_FREE(keystr);
	return ret == 0;
}
