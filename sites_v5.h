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

#ifndef _SITES_V5_H
#define _SITES_V5_H

typedef enum _switch_type {
  SW_NONE = 0,
  SW_INT, SW_CLB,
  SW_IOB, SW_DSP48,
  SW_BRAM, NR_SWITCH_TYPE,
} __attribute__((packed)) switch_type_t;

typedef enum _site_type {
  SITE_TYPE_NEUTRAL = 0,
  IOB, CLB, DSP48,
  GCLKC, BRAM,
  NR_SITE_TYPE,
} __attribute__((packed)) site_type_t;

/* XXX duplicated wherever included */
static const switch_type_t sw_of_type_table[NR_SITE_TYPE] = {
  [SITE_TYPE_NEUTRAL] = SW_NONE,
  [CLB] = SW_INT,
  [IOB] = SW_INT,
  [DSP48] = SW_INT,
};

static inline
switch_type_t sw_of_type(const site_type_t site) {
  return sw_of_type_table[site];
}

typedef enum _wire_direction {
  WIRE_DIRECTION_NEUTRAL = 0,
  N, WN, NW,
  W, WS, SW,
  S, SE, ES,
  E, EN, NE,
  DN, UP, // global clock
  NR_WIRE_DIRECTION,
} __attribute__((packed)) wire_direction_t;

typedef enum _wire_situation {
  WIRE_SITUATION_NEUTRAL = 0,
  BEG, A, B, MID, C, D, END,
  NR_WIRE_SITUATION,
} __attribute__((packed)) wire_situation_t;

typedef enum _wire_type {
  WIRE_TYPE_NEUTRAL = 0,
  DOUBLE, HEX,
  OMUX,
  LH, LV,
  LOGIC,
  NR_WIRE_TYPE,
} __attribute__((packed)) wire_type_t;

#endif /* _SITES_V5_H */
