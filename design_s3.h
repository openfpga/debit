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

#ifndef _DESIGN_S3_H
#define _DESIGN_S3_H

#include <stdint.h>
#include <glib.h>
#include "bitarray.h"
#include "bitstream_parser.h"

#define CHIP "spartan3"

typedef enum _id_s3 {
  XC3S50, XC3S200,
  XC3S400, XC3S1000,
  XC3S1500, XC3S2000,
  XC3S4000, XC3S5000,
  XC3__NUM,
} s3_id_t;

/* family specific */
typedef enum {
  V2C_IOB = 0,
  V2C_IOI,
  V2C_CLB,
  V2C_BRAM,
  V2C_BRAM_INT,
  V2C_GCLK,
  V2C__NB_CFG,
} v2_design_col_t;

typedef struct _chip_struct {
  s3_id_t chip;
  guint32 idcode;
  guint32 framelen;
  const unsigned col_count[V2C__NB_CFG];
  const unsigned *frame_count;
} chip_struct_t;

/*
 * FAR Register implementation
 */

/*
 * FAR register -- hardware
 */

/* This is exactly identical to Virtex-2 */

/* TODO: get rid of this. Higly non-portable */
typedef struct {
  unsigned int bn :9; // NOT in bytes, but in u32 words (or won't fit)
  unsigned int mna :8;
  unsigned int mja :8;
  unsigned int ba :2;
  unsigned int __rsvd :5;
} v2_frame_addr_t;

#define FAR_BN_OFFSET 0
#define FAR_BN_LEN 9
#define FAR_BN_MASK ((1<<FAR_BN_LEN) - 1) << FAR_BN_OFFSET

#define FAR_MNA_OFFSET (FAR_BN_OFFSET + FAR_BN_LEN)
#define FAR_MNA_LEN 8
#define FAR_MNA_MASK ((1<<FAR_MNA_LEN) - 1) << FAR_MNA_OFFSET

#define FAR_MJA_OFFSET (FAR_MNA_OFFSET + FAR_MNA_LEN)
#define FAR_MJA_LEN 8
#define FAR_MJA_MASK ((1<<FAR_MJA_LEN) - 1) << FAR_MJA_OFFSET

#define FAR_BA_OFFSET (FAR_MJA_OFFSET + FAR_MJA_LEN)
#define FAR_BA_LEN 2
#define FAR_BA_MASK ((1<<FAR_BA_LEN) - 1) << FAR_BA_OFFSET

static inline unsigned
bn_of_far(const uint32_t far) {
	return (far & FAR_BN_MASK) >> FAR_BN_OFFSET;
}

static inline unsigned
mna_of_far(const uint32_t far) {
	return (far & FAR_MNA_MASK) >> FAR_MNA_OFFSET;
}

static inline unsigned
mja_of_far(const uint32_t far) {
	return (far & FAR_MJA_MASK) >> FAR_MJA_OFFSET;
}

static inline unsigned
ba_of_far(const uint32_t far) {
	return (far & FAR_BA_MASK) >> FAR_BA_OFFSET;
}

/*
 * FAR register -- our more simple software vision of it
 */

typedef struct sw_far {
	unsigned bn;
	unsigned mna;
	unsigned mja;
	unsigned ba;
} sw_far_t;

static inline void
fill_swfar(sw_far_t *sw_far, const uint32_t hwfar) {
	sw_far->bn = bn_of_far(hwfar);
	sw_far->mna = mna_of_far(hwfar);
	sw_far->mja = mja_of_far(hwfar);
	sw_far->ba = ba_of_far(hwfar);
}

static inline uint32_t
get_hwfar(const sw_far_t *sw_far) {
	return (sw_far->bn +
		(sw_far->mna << FAR_MNA_OFFSET) +
		(sw_far->mja << FAR_MJA_OFFSET) +
		(sw_far->ba  << FAR_BA_OFFSET));
}

/*
 * The frame index is a three-way lookup table. We choose for now to use
 * a two-way lookup table to index the frames internally.
 */

static inline
const gchar **get_frame_loc(const bitstream_parsed_t *parsed,
			    const guint type,
			    const guint idx,
			    const guint frame) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const unsigned *col_count = chip_struct->col_count;
  const unsigned *frame_count = chip_struct->frame_count;
  g_assert(type < V2C__NB_CFG);
  g_assert(idx < col_count[type]);
  g_assert(frame < frame_count[type]);
  (void) col_count;

  /* This is a double-lookup method */
  return &parsed->frames[type][idx * frame_count[type] + frame];
}

/* FDRI handling. Requires FAR handling.
   Registers a frame */

static inline
const gchar *get_frame(const bitstream_parsed_t *parsed,
		       const guint type,
		       const guint idx,
		       const guint frame) {
  const gchar *frameptr = *get_frame_loc(parsed, type, idx, frame);
  g_assert(frameptr != NULL);
  return frameptr;
}

#endif /* _DESIGN_S3_H */
