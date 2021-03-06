/*
 * Copyright (C) 2006, 2007 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 *
 * This file is part of debit.
 *
 * Debit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Debit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with debit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdio.h>

#include <glib.h>

#include "debitlog.h"
#include "bitisolation_db.h"

state_t *
get_pip_state(const pip_db_t *pipdb, const unsigned i) {
  return &pipdb->state_array[i];
}

const char *
get_pip_name(const pip_db_t *pipdb, const unsigned i) {
  return pipdb->pip_array[i].name;
}

const char *
get_pip_start(const pip_db_t *pipdb, const unsigned i) {
  return pipdb->pip_array[i].start;
}

const char *
get_pip_end(const pip_db_t *pipdb, const unsigned i) {
  return pipdb->pip_array[i].end;
}

pip_ref_t *
get_pip(const pip_db_t *pipdb, const unsigned i) {
  return &pipdb->pip_array[i];
}

unsigned
get_pip_index(const pip_db_t *pipdb, const gchar *pip) {
  const gchar *chunk = g_string_chunk_insert_const (pipdb->chunk, pip);
  return GPOINTER_TO_UINT(g_hash_table_lookup (pipdb->hash, chunk));
}

typedef void (*ifile_iterator_t)(const gchar*, void *data);

static void
iterate_over_input(const gchar **knw,
		   ifile_iterator_t iter, void *data) {
  unsigned idx = 0;
  const gchar *inp;

  while ((inp = knw[idx++])) {
    GDir *dir = g_dir_open(inp, 0, NULL);
    const gchar *file;

    if (!dir)
      continue;

    /* Iterate over elements in the directory */
    while ((file = g_dir_read_name (dir)))
      if (g_str_has_suffix(file, ".dat")) {
	gchar *filename = g_build_filename(inp,file,NULL);
	iter(filename, data);
	g_free(filename);
      }

    g_dir_close(dir);
  }
}

static void
add_pip_line(const gchar *line, void *data) {
  pip_db_t *db = data;
  const gchar *chunk;
  GHashTable *hash = db->hash;
  guint value_int;
  gpointer orig_key, value_ptr;

  chunk = g_string_chunk_insert_const (db->chunk, line);

  /* query & insert it in the LUT */
  if (g_hash_table_lookup_extended (hash, chunk, &orig_key, &value_ptr))
    return;
  value_int = g_hash_table_size(hash);
  value_ptr = GUINT_TO_POINTER(value_int);
  debit_log(L_CORRELATE, "Adding pip %s with value %u to the hashtable", chunk, value_int);
  g_hash_table_insert (hash, (gpointer)chunk, value_ptr);
}

void
iterate_over_lines(const gchar *filename,
		   line_iterator_t iter, void *data) {
  pip_db_t *pipdb = data;
  gchar *contents;
  gchar **lines, *line;
  unsigned i = 0;

  /* XXX handle failure, ungracefully */
  g_file_get_contents(filename, &contents, NULL, NULL);
  if (!contents) {
    g_warning("file %s does not exist!", filename);
    return;
  }

  lines = g_strsplit(contents, "\n", 0);
  g_free(contents);

  while((line = lines[i++]) != NULL)
    if (strlen(line) != 0)
      iter(line, pipdb);

  g_strfreev(lines);
}

static void
add_pip_file(const gchar *file, void *data) {
  pip_db_t *pipdb = data;
  debit_log(L_CORRELATE, "Loading file %s", file);
  iterate_over_lines(file, add_pip_line, pipdb);
}

static void
store_iline(gpointer key,
	    gpointer value,
	    gpointer user_data) {
  guint idx = GPOINTER_TO_UINT(value);
  pip_db_t *db = user_data;
  pip_ref_t *ref = &db->pip_array[idx];
  gchar **endpoints = g_strsplit (key, " ", 2);

  ref->name = key;

  if (endpoints) {
    ref->start = g_string_chunk_insert_const (db->chunk, endpoints[0]);
    ref->end = g_string_chunk_insert_const (db->chunk, endpoints[1]);
  } else {
    ref->start = NULL;
    ref->end = NULL;
  }

  g_strfreev(endpoints);
}

/* build the pip db from a series of txt files */
pip_db_t *
build_pip_db(const gchar **files) {
  pip_db_t *db = g_new0(pip_db_t, 1);
  /* direct hash & compare thanks to the K-K chunky below */
  GHashTable *hash = g_hash_table_new (NULL, NULL);
  unsigned pipnum;

  db->chunk = g_string_chunk_new (16);
  db->hash = hash;

  iterate_over_input(files, add_pip_file, db);

  /* the summary of the table, nicely put */
  pipnum = g_hash_table_size(db->hash);
  db->pip_num = pipnum;
  db->pip_array = g_new0(pip_ref_t, pipnum);
  db->state_array = g_new(state_t, pipnum);
  g_hash_table_foreach (hash, store_iline, db);
  return db;
}

void free_pip_db(pip_db_t *db) {
  /* free states */
  g_hash_table_destroy(db->hash);
  g_string_chunk_free(db->chunk);
  g_free(db->state_array);
  g_free(db->pip_array);
  g_free(db);
}

void
iterate_over_pips(const pip_db_t *pipdb, pip_iterator_t iter, void *dat) {
  pip_ref_t *piparray = pipdb->pip_array;
  state_t *pipstate = pipdb->state_array;
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  for(pip = 0; pip < npips; pip++) {
    pip_ref_t *pipref = &piparray[pip];
    state_t *state = &pipstate[pip];
    iter(pipref, state, dat);
  }
}

static void do_state(pip_ref_t *ref, state_t *state, void *dat) {
  alloc_state(state, dat);
  init_state(state);
}

/* FIXME: move this to one big allocation array */
void
alloc_pips_state(pip_db_t *pip_db, const alldata_t *dat) {
  void *arg = (void *) dat;
  iterate_over_pips(pip_db, do_state, arg);
}

static void free_state(pip_ref_t *ref, state_t *state, void *dat) {
  (void) dat;
  (void) ref;
  release_state(state);
}

void free_pips_state(pip_db_t *pipdb) {
  iterate_over_pips(pipdb, free_state, NULL);
}
