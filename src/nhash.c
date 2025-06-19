/*
 *-------------------------------------------------------------------------------
 *
 * This file is part of the WSPR application, Weak Signal Propagation Reporter
 *
 * File Name:   nhash.c
 * Description: Functions to produce 32-bit hashes for hash table lookup
 *
 * Copyright (C) 2008-2014 Joseph Taylor, K1JT
 * License: GPL-3
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Files: lookup3.c
 * Copyright: Copyright (C) 2006 Bob Jenkins <bob_jenkins@burtleburtle.net>
 * License: public-domain
 *  You may use this code any way you wish, private, educational, or commercial.
 *  It's free.
 *
 *-------------------------------------------------------------------------------
 */

#include <stdint.h>
#include <stddef.h>

/* Mix and final macros from Bob Jenkins */
#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))
#define mix(a, b, c)        \
  {                         \
    a -= c; a ^= rot(c, 4); c += b; \
    b -= a; b ^= rot(a, 6); a += c; \
    c -= b; c ^= rot(b, 8); b += a; \
    a -= c; a ^= rot(c,16); c += b; \
    b -= a; b ^= rot(a,19); a += c; \
    c -= b; c ^= rot(b, 4); b += a; \
  }
#define final(a, b, c)       \
  {                         \
    c ^= b; c -= rot(b,14);       \
    a ^= c; a -= rot(c,11);       \
    b ^= a; b -= rot(a,25);       \
    c ^= b; c -= rot(b,16);       \
    a ^= c; a -= rot(c, 4);       \
    b ^= a; b -= rot(a,14);       \
    c ^= b; c -= rot(b,24);       \
  }

/* Jenkins' hashlittle implementation renamed here */
static uint32_t nhash_impl(const void *key, int *length0, uint32_t *initval0) {
    uint32_t a, b, c;
    int length = *length0;
    uint32_t initval = *initval0;
    union { const void *ptr; size_t i; } u;
    u.ptr = key;
    a = b = c = 0xdeadbeef + (uint32_t)length + initval;

    if ((u.i & 3) == 0) {
        const uint32_t *k32 = (const uint32_t *)key;
        const uint8_t *k8;
        while (length > 12) {
            a += k32[0]; b += k32[1]; c += k32[2];
            mix(a, b, c);
            length -= 12;
            k32 += 3;
        }
        k8 = (const uint8_t *)k32;
        switch (length) {
        case 12: c += k32[2]; b += k32[1]; a += k32[0]; break;
        case 11: c += k8[10] << 16; b += k32[1]; a += k32[0]; break;
        case 10: c += k8[9] << 8;   b += k32[1]; a += k32[0]; break;
        case 9:  c += k8[8];        b += k32[1]; a += k32[0]; break;
        case 8:  b += k32[1];       a += k32[0];      break;
        case 7:  b += k8[6] << 16;  a += k32[0];      break;
        case 6:  b += k8[5] << 8;   a += k32[0];      break;
        case 5:  b += k8[4];        a += k32[0];      break;
        case 4:  a += k32[0];                           break;
        case 3:  a += k8[2] << 16;                    break;
        case 2:  a += k8[1] << 8;                     break;
        case 1:  a += k8[0];                         break;
        case 0:  return c;
        }
    } else if ((u.i & 1) == 0) {
        const uint16_t *k16 = (const uint16_t *)key;
        const uint8_t *k8;
        while (length > 12) {
            a += k16[0] + ((uint32_t)k16[1] << 16);
            b += k16[2] + ((uint32_t)k16[3] << 16);
            c += k16[4] + ((uint32_t)k16[5] << 16);
            mix(a, b, c);
            length -= 12;
            k16 += 6;
        }
        k8 = (const uint8_t *)k16;
        switch (length) {
        case 12: c += k16[4] + ((uint32_t)k16[5] << 16);
                 b += k16[2] + ((uint32_t)k16[3] << 16);
                 a += k16[0] + ((uint32_t)k16[1] << 16);
                 break;
        case 11: c += k8[10] << 16; /* fallthrough */
        case 10: c += k16[4]; b += k16[2] + ((uint32_t)k16[3] << 16);
                 a += k16[0] + ((uint32_t)k16[1] << 16);
                 break;
        case 9:  c += k8[8]; /* fallthrough */
        case 8:  b += k16[2] + ((uint32_t)k16[3] << 16);
                 a += k16[0] + ((uint32_t)k16[1] << 16);
                 break;
        case 7:  b += k8[6] << 16; /* fallthrough */
        case 6:  b += k16[2]; a += k16[0] + ((uint32_t)k16[1] << 16); break;
        case 5:  b += k8[4]; /* fallthrough */
        case 4:  a += k16[0] + ((uint32_t)k16[1] << 16); break;
        case 3:  a += k8[2] << 16; /* fallthrough */
        case 2:  a += k16[0]; break;
        case 1:  a += k8[0]; break;
        case 0:  return c;
        }
    } else {
        const uint8_t *k8 = (const uint8_t *)key;
        while (length > 12) {
            a += k8[0]; a += (uint32_t)k8[1] << 8;
            a += (uint32_t)k8[2] << 16; a += (uint32_t)k8[3] << 24;
            b += k8[4]; b += (uint32_t)k8[5] << 8;
            b += (uint32_t)k8[6] << 16; b += (uint32_t)k8[7] << 24;
            c += k8[8]; c += (uint32_t)k8[9] << 8;
            c += (uint32_t)k8[10] << 16; c += (uint32_t)k8[11] << 24;
            mix(a, b, c);
            length -= 12;
            k8 += 12;
        }
        switch (length) {
        case 12: c += (uint32_t)k8[11] << 24; /* fallthrough */
        case 11: c += (uint32_t)k8[10] << 16; /* fallthrough */
        case 10: c += (uint32_t)k8[9] << 8;   /* fallthrough */
        case 9:  c += k8[8]; /* fallthrough */
        case 8:  b += (uint32_t)k8[7] << 24;  /* fallthrough */
        case 7:  b += (uint32_t)k8[6] << 16;  /* fallthrough */
        case 6:  b += (uint32_t)k8[5] << 8;   /* fallthrough */
        case 5:  b += k8[4]; break;
        case 4:  a += (uint32_t)k8[3] << 24; break;
        case 3:  a += (uint32_t)k8[2] << 16; break;
        case 2:  a += (uint32_t)k8[1] << 8;  break;
        case 1:  a += k8[0]; break;
        case 0:  return c;
        }
    }

    final(a, b, c);
    return c;
}

/*
 * This is the C‐linkage wrapper that JTEncode.cpp calls:
 */
#ifdef __cplusplus
extern "C" {
#endif

int nhash_(unsigned char *ic, int len, int val) {
    uint32_t length = (uint32_t)len;
    uint32_t initval = (uint32_t)val;
    return (int)nhash_impl(ic, (int *)&length, &initval);
}

#ifdef __cplusplus
}
#endif

