/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _BITSTREAM_H
#define _BITSTREAM_H

#include "bitstream_parser.h"
#include "wiring.h"
#include "sites.h"

/** \file
 *
 * Interface file for accessing the bitstream data in a
 * pseudo-structured way.
 */

void
query_bitstream_luts(const bitstream_parsed_t *, const csite_descr_t *, guint16[]);

guint32
query_bitstream_site_bits(const bitstream_parsed_t *, const csite_descr_t *,
			  const guint *, const gsize);
guint16 *
query_bitstream_bram_data(const bitstream_parsed_t *bitstream, const csite_descr_t *site);

gsize
query_bitstream_type_size(const bitstream_parsed_t *parsed,
			  const site_type_t type);

int
query_bitstream_site_data(gchar *data, const gsize nbytes,
	                  const bitstream_parsed_t *bitstream,
			  const csite_descr_t *site);

#endif /* _BITSTREAM_H */
