/*
 * Code to dump RATE table from Nortel Millennium Payphone
 * Table 73 (0x49)
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 * The RATE Table is an array of 1191 bytes.  The first 39 bytes contain
 * unknown data.
 *
 * The remaining 1152 bytes are a set of 128 9-byte rate entries.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"

const char *str_rates[] = {
    "mm_intra_lata     ",
    "lms_rate_local    ",
    "fixed_charge_local",
    "not_available     ",
    "invalid_npa_nxx   ",
    "toll_intra_lata   ",
    "toll_inter_lata   ",
    "mm_inter_lata     ",
    "mm_local          ",
    "international     ",
    "      ?0a?        ",
    "      ?0b?        ",
    "      ?0c?        ",
    "      ?0d?        ",
    "      ?0e?        ",
    "      ?0f?        ",
};

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   rate_index;
    char  rate_str_initial[10];
    char  rate_str_additional[10];
    char  timestamp_str[20];
    dlog_mt_rate_table_t *prate_table;
    uint8_t *load_buffer;
    int ret = 0;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_rate mm_table_49.bin [outputfile.bin]\n");
        return -1;
    }

    printf("Nortel Millennium RATE Table 0x49 (73) Dump\n\n");

    prate_table = (dlog_mt_rate_table_t *)calloc(1, sizeof(dlog_mt_rate_table_t));

    if (prate_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_rate_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(prate_table);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)prate_table) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_rate_table_t) - 1, 1, instream) != 1) {
        printf("Error reading RATE table.\n");
        free(prate_table);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("Date: %s\n", timestamp_to_string(prate_table->timestamp, timestamp_str, sizeof(timestamp_str)));
    printf("Telco ID: 0x%02x (%d)\n", prate_table->telco_id, prate_table->telco_id);

    /* Dump spare 32 bytes at the beginning of the RATE table */
    printf("Spare bytes:\n");
    dump_hex(prate_table->spare, 32);

    printf("\n+------------+-------------------------+----------------+--------------+-------------------+-----------------+\n" \
           "| Index      | Type                    | Initial Period | Initial Rate | Additional Period | Additional Rate |\n"   \
           "+------------+-------------------------+----------------+--------------+-------------------+-----------------+");

    for (rate_index = 0; rate_index < RATE_TABLE_MAX_ENTRIES; rate_index++) {
        rate_table_entry_t *prate = &prate_table->r[rate_index];

        if (prate->type ==  0) continue;

        if ((prate->type == 0) && (prate->initial_charge == 0) && (prate->additional_charge == 0) && \
            (prate->initial_period == 0) && (prate->additional_period == 0)) {
            continue;
        }

        if (prate->initial_period & FLAG_PERIOD_UNLIMITED) {
            snprintf(rate_str_initial, sizeof(rate_str_initial), "Unlimited");
        } else {
            snprintf(rate_str_initial, sizeof(rate_str_initial), "   %5ds", prate->initial_period);
        }

        if (prate->additional_period & FLAG_PERIOD_UNLIMITED) {
            snprintf(rate_str_additional, sizeof(rate_str_additional), "Unlimited");
        } else {
            snprintf(rate_str_additional, sizeof(rate_str_additional), "   %5ds", prate->additional_period);
        }

        printf("\n| %3d (0x%02x) | 0x%02x %s |      %s |       %6.2f |         %s |          %6.2f |",
               rate_index, rate_index,
               prate->type,
               str_rates[(prate->type) & 0x0F],
               rate_str_initial,
               (float)prate->initial_charge / 100,
               rate_str_additional,
               (float)prate->additional_charge / 100);
    }

    printf("\n+------------------------------------------------------------------------------------------------------------+\n");

    /* Modify RATE table */

    if (argc == 3) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_rate_table_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (prate_table != NULL) {
        free(prate_table);
    }

    return ret;
}
