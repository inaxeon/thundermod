/*
 *   File:   guidsub.c
 *   Author: Matthew Millman (inaxeon@hotmail.com)
 *
 *   Tool for substituting little endian GUIDs in binary files
 *
 *   This is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *   This software is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *   along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include <stdbool.h>
#include <uuid/uuid.h>

#define SWAP32(x) ((x >> 24) & 0xFF) | ((x << 8) & 0xFF0000) | ((x >> 8) & 0xFF00) | ((x << 24) & 0xFF000000)
#define SWAP16(x) ((x << 8) | ((x >> 8) & 0xFF))

void uuid_fix_endianness(uuid_t uuid);

int main(int argc, char *argv[])
{
    FILE *f = NULL;
    size_t fileSize = 0;
    int pos = 0;
    uint8_t *buffer = NULL;
    const char *fileNameStr = NULL;
    const char *originalGuidStr = NULL;
    const char *newGuidStr = NULL;
    int opt = 0;
    int ret = EXIT_FAILURE;
    bool substituted = false;

    uuid_t originalGuid;
    uuid_t newGuid;

    while ((opt = getopt(argc, argv, "f:o:n:?")) != -1)
    {
        switch (opt)
        {
        case 'f':
            fileNameStr = optarg;
            break;
        case 'o':
            originalGuidStr = optarg;
            break;
        case 'n':
            newGuidStr = optarg;
            break;
        default:
            fprintf(stderr, "\r\nUsage: %s -f <filename> -o <original GUID> -n <new GUID>\n\n", argv[0]);
            goto fail;
        }
    }

    if (fileNameStr == NULL)
    {
        fprintf(stderr, "Error: No filename specified\n");
        goto fail;
    }

    if (originalGuidStr == NULL)
    {
        fprintf(stderr, "Error: Original GUID not specified\n");
        goto fail;
    }

    if (newGuidStr == NULL)
    {
        fprintf(stderr, "Error: New GUID not specified\n");
        goto fail;
    }

    if (uuid_parse(originalGuidStr, originalGuid))
    {
        fprintf(stderr, "Error: Failed to parse original GUID\n");
        goto fail;
    }

    uuid_fix_endianness(originalGuid);

    if (uuid_parse(newGuidStr, newGuid))
    {
        fprintf(stderr, "Error: Failed to parse new GUID\n");
        goto fail;
    }

    uuid_fix_endianness(newGuid);

    f = fopen(fileNameStr, "rb");

    if (!f)
    {
        fprintf(stderr, "Error: Failed to open input file\n");
        goto fail;
    }

    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(fileSize);

    if (fread(buffer, fileSize, 1, f) != 1)
    {
        fprintf(stderr, "Error: Failed to read input file\n");
        goto fail;
    }

    fclose(f);
    f = NULL;

    for (pos = 0; pos < (fileSize - sizeof(uuid_t)); pos++)
    {
        if (memcmp((buffer + pos), originalGuid, sizeof(uuid_t)) == 0)
        {
            memcpy((buffer + pos), newGuid, sizeof(uuid_t));
            substituted = true;
        }
    }

    if (!substituted)
    {
        fprintf(stdout, "GUID not found in source file. Exiting.\n");
        goto fail;
    }

    f = fopen(fileNameStr, "wb");

    if (!f)
    {
        fprintf(stderr, "Error: Failed to open file for writing\n");
        goto fail;
    }

    if (fwrite(buffer, fileSize, 1, f) != 1)
    {
        fprintf(stderr, "Error: Failed to write updated file\n");
        goto fail;
    }

    printf("Successfully updated protocol GUID\n");

    ret = EXIT_SUCCESS;

fail:

    if (f)
        fclose(f);
    if (buffer)
        free(buffer);

    return ret;
}

void uuid_fix_endianness(uuid_t uuid)
{
    // ffs. really?
    *(uint32_t *)(&uuid[0]) = SWAP32(*(uint32_t *)(&uuid[0]));
    *(uint16_t *)(&uuid[4]) = SWAP16(*(uint16_t *)(&uuid[4]));
    *(uint16_t *)(&uuid[6]) = SWAP16(*(uint16_t *)(&uuid[6]));
}