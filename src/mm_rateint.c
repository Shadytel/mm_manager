/*
 * Code to dump DLOG_MT_INTL_SBR_TABLE table from Nortel Millennium Payphone
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2023, Howard M. Harte
 *
 * The International Set-based rating able is an array of 603 bytes.
 * The first three bytes define the default flags and rate entry for
 * international codes not found in the remainder of the table.
 * The remainder of the table is an array of 200 entries of three bytes
 * each.  Two bytes for calling code, and one byte for flags/rate table
 * entry.

 * Thanks to �strid for figuring out the data structures for the International
 * Set-based Rating table, which is not documented in the Database Design
 * Report MSR 2.1
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "mm_manager.h"

#define TABLE_ID    DLOG_MT_INTL_SBR_TABLE

/* Sample table entries */
intl_rate_table_entry_t new_irates[] = {
    {  44, 6             },  // International Rate 0 - United Kingdom
    {   7, 7             },  // International Rate 1 - Russia
    { 850, IXL_BLOCKED   },  // International Rate 2 - North Korea
    {  98, IXL_BLOCKED   },  // International Rate 3 - Iran
    { 218, IXL_BLOCKED   },  // International Rate 4 - Libya
    { 249, IXL_BLOCKED   },  // International Rate 4 - Sudan
    { 963, IXL_BLOCKED   },  // International Rate 4 - Syria
    {  43, IXL_NCC_RATED },  // International Rate 8 - Austria
};

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   rate_index;
    dlog_mt_intl_sbr_table_t *ptable;
    uint8_t* load_buffer;
    int ret = 0;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_rateint mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_intl_sbr_table_t *)calloc(1, sizeof(dlog_mt_intl_sbr_table_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_intl_sbr_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    if (mm_validate_table_fsize(TABLE_ID, instream, sizeof(dlog_mt_intl_sbr_table_t) - 1) != 0) {
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_intl_sbr_table_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    /* Display International SBR table common information. */
    printf("International Flags: 0x%02x (%d)\n", ptable->flags,              ptable->flags);
    printf(" Default Rate index: 0x%02x (%d) ",  ptable->default_rate_index, ptable->default_rate_index);

    if (ptable->default_rate_index == 0) {         // Rated by NCC
        printf("NCC-rated\n");
    } else if (ptable->default_rate_index == 1) {  // Blocked call
        printf("Blocked\n");
    } else {
        printf("Rate table Index: %02x (%d)\n",
               IXL_TO_RATE(ptable->default_rate_index), IXL_TO_RATE(ptable->default_rate_index));
    }
    printf("              Spare: 0x%02x (%d)\n", ptable->spare, ptable->spare);

    printf("\n+------------+--------------+------------+\n" \
           "| Index      | CCode        | RATE Entry |\n"   \
           "+------------+--------------+------------+");

    for (rate_index = 0; rate_index < INTL_RATE_TABLE_MAX_ENTRIES; rate_index++) {
        intl_rate_table_entry_t *prate;

        prate = &ptable->irate[rate_index];

        if (LE16(prate->ccode) ==  0) continue;

        printf("\n| %3d (0x%02x) | 0x%04x %5d | ",
               rate_index, rate_index,
               LE16(prate->ccode),
               LE16(prate->ccode));

        if (prate->flags == 0) {         // Rated by NCC
            printf("NCC-rated  |");
        } else if (prate->flags == 1) {  // Blocked
            printf("BLOCKED    |");
        } else {
            printf("0x%02x (%d)  |", IXL_TO_RATE(prate->flags), IXL_TO_RATE(prate->flags));
        }
    }

    printf("\n+----------------------------------------+\n");

    if (argc == 3) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* Update International RATE table */
    memset(ptable->irate, 0, sizeof(intl_rate_table_entry_t) * INTL_RATE_TABLE_MAX_ENTRIES);
    memcpy(ptable->irate, new_irates, sizeof(new_irates));

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_intl_sbr_table_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    free(ptable);

    return ret;
}
