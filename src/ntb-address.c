/*
 * Notbit - A Bitmessage client
 * Copyright (C) 2013  Neil Roberts
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <assert.h>
#include <string.h>

#include "ntb-address.h"
#include "ntb-base58.h"

void
ntb_address_encode(uint8_t version,
                   uint8_t stream,
                   const uint8_t *ripe,
                   char *output)
{
        SHA512_CTX sha_ctx;
        uint8_t hash1[SHA512_DIGEST_LENGTH];
        uint8_t hash2[SHA512_DIGEST_LENGTH];
        uint8_t bin_address[1 + 1 + RIPEMD160_DIGEST_LENGTH + 4];
        uint8_t *p;
        int ripe_length;
        int max_trim;
        size_t address_length;
        int i;

        ripe_length = RIPEMD160_DIGEST_LENGTH;

        if (version >= 4)
                max_trim = RIPEMD160_DIGEST_LENGTH;
        else
                max_trim = 2;

        for (i = 0; i < max_trim && *ripe == 0; i++) {
                ripe++;
                ripe_length--;
        }

        SHA512_Init(&sha_ctx);
        SHA512_Update(&sha_ctx, &version, 1);
        SHA512_Update(&sha_ctx, &stream, 1);
        SHA512_Update(&sha_ctx, ripe, ripe_length);
        SHA512_Final(hash1, &sha_ctx);

        SHA512_Init(&sha_ctx);
        SHA512_Update(&sha_ctx, hash1, SHA512_DIGEST_LENGTH);
        SHA512_Final(hash2, &sha_ctx);

        p = bin_address;
        *(p++) = version;
        *(p++) = stream;
        memcpy(p, ripe, ripe_length);
        p += ripe_length;
        memcpy(p, hash2, 4);
        p += 4;

        assert(p - bin_address <= sizeof bin_address);

        memcpy(output, "BM-", 3);
        address_length = ntb_base58_encode(bin_address,
                                           p - bin_address,
                                           output + 3);
        output[address_length + 3] = '\0';
}
