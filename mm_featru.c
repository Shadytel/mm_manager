/*
 * Code to dump Feature Configuration table from Nortel Millennium Payphone
 * Table 26 (0x1a) - FEATRU pp. 2-151
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mm_manager.h"

void print_bits(uint8_t bits, char *str_array[]);

const char *auth_bits_str[] = {
    "FC_CARD_AUTH_ON_LOCAL_CALLS",
    "FC_DELAYED_CARD_AUTHORIZATION",
    "FC_CARD_AUTH_ON_MCE_LOCAL_CALLS",
    "FC_NO_NPA_ADDED_ZP_LOCAL_ACCS",
    "FC_CARD_AUTH_BIT_4",
    "FC_CARD_AUTH_BIT_5",
    "FC_CARD_AUTH_BIT_6",
    "FC_IMMED_MCE_CARD_AUTH"
};

const char *accs_bits_str[] = {
    "FC_ACCS_AVAILABLE",
    "FC_MCE_ROUTING",
    "FC_MANUAL_DIALED_CARD_NUM_ENABLED",
    "FC_MANUALLY_DIALED_NCC_VALID_REQ",
    "FC_AOS_ENABLED",
    "FC_ZERO_PLUS_LOCAL_CALLS_TO_NCC",
    "FC_ACCS_INFO_BIT_6",
    "FC_REMOVE_NPA_ZP_LOCAL_NCC_CALLS"
};

const char *call_mode_str[] = {
    "FC_CALL_MODE_NO_INCOMING",
    "FC_CALL_MODE_INCOMING_VOICE_ONLY",
    "FC_CALL_MODE_RING_DISABLED_ANSWER_DATA",
    "FC_CALL_MODE_RING_ENABLED_ANSWER_DATA"
};

const char *misc_flags_str[] = {
    "FC_IN_SERVICE_ON_CDR_LIST_FULL",
    "FC_TERM_RATE_DISPLAY_OPTION",
    "FC_INCOMING_CALL_FCA_PRECEDENCE",
    "FC_FCA_ON_CARD",
    "FC_REVERT_TO_PRIMARY_NCC_NUM",
    "FC_BLOCK_NO_RATE_CARRIER",
    "FC_RATED_CREDIT_CARD_CDR",
    "FC_11_DIGIT_LOCAL_CALLS"
};

const char *advertising_bits_str[] = {
    "FC_ADVERT_ENABLED",
    "FC_REP_DIALER_ADVERTISING",
    "FC_CALL_ESTABLISHED_ADVERTISING",
    "FC_ENABLE_DATE_TIME_DISPLAY",
    "FC_TIME_FORMAT",
    "FC_ADVERTISING_FLAGS_BIT_5",
    "FC_ADVERTISING_FLAGS_BIT_6",
    "FC_ADVERTISING_FLAGS_BIT_7"
};

const char *call_setup_flags_str[] = {
    "FC_DISPLAY_CALLED_NUMBER",
    "FC_ENABLE_SERVLEV_DISP_FLASHING",
    "FC_CALL_SETUP_PARAMS_BIT_2",
    "FC_CALL_SETUP_PARAMS_BIT_3",
    "FC_CALL_SETUP_PARAMS_BIT_4",
    "FC_CALL_SETUP_PARAMS_BIT_5",
    "FC_CALL_SETUP_PARAMS_BIT_6",
    "FC_SUPPRESS_CALLING_PROMPT"
};

const char *coin_calling_features_str[] = {
    "FC_COIN_CALL_OVERTIME",
    "FC_VOICE_FEEDBACK_ON_COIN_CALL",
    "FC_COIN_CALL_SECOND_WARNING",
    "FC_COIN_CALL_FEATURES_BIT_3",
    "FC_COIN_CALL_FEATURES_BIT_4",
    "FC_COIN_CALL_FEATURES_BIT_5",
    "FC_COIN_CALL_FEATURES_BIT_6",
    "FC_COIN_CALL_FEATURES_BIT_7"
};

const char *smartcard_flags_str[] = {
    "FC_SMART_CARD_FLAGS_BIT_0",
    "FC_SC_VALID_INTERNATIONAL_CALLS",
    "FC_SC_VALID_INTER_LATA_CALLS",
    "FC_SC_VALID_INTRA_LATA_CALLS",
    "FC_SC_VALID_LOCAL_CALLS",
    "FC_POST_PAYMENT_RATE_REQUEST",
    "FC_USE_TERMINAL_CARD_TABLE_DEF",
    "FC_RATE_INFO_NOT_DISPLAYED",
};

const char *carrier_reroute_flags_str[] = {
    "FC_BLOCK_REROUTE_COIN_CALL",
    "FC_BLOCK_REROUTE_CREDIT_CARD_CALL",
    "FC_BLOCK_REROUTE_SMART_CARD_CALL",
    "FC_BLOCK_REROUTE_CALL_CARD_CALL",
    "FC_CARRIER_BLOCK_REROUTE_BIT_4",
    "FC_CARRIER_BLOCK_REROUTE_BIT_5",
    "FC_CARRIER_BLOCK_REROUTE_BIT_6",
    "FC_CARRIER_BLOCK_REROUTE_BIT_7",
};

const char *datajack_flags_str[] = {
    "FC_DATAJACK_ENABLED",
    "FC_DATAJACK_MUTING",
    "FC_DATAJACK_ALLOW_FREE_LOCAL_CALL",
    "FC_DATAJACK_ALLOW_DA_CALLS",
    "FC_DJ_FLAGS_BIT_4",
    "FC_DJ_FLAGS_BIT_5",
    "FC_DJ_FLAGS_BIT_6",
    "FC_DJ_FLAGS_BIT_7"
};

int main(int argc, char *argv[])
{
    FILE *instream;
    dlog_mt_fconfig_opts_t *featru_table;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_featru mm_table_1a.bin\n");
        return (-1);
    }

    instream = fopen(argv[1], "rb");

    printf("Nortel Millennium FEATRU Table (Table 26) Dump\n");

    featru_table = calloc(1, sizeof(dlog_mt_fconfig_opts_t));
    if (fread(featru_table, sizeof(dlog_mt_fconfig_opts_t), 1, instream) <= 0) {
        printf("Error reading ADMESS table.\n");
        if (featru_table != NULL) {
            free(featru_table);
            return (-2);
        }
    }

    printf("                             term_type: 0x%02x\n", featru_table->term_type);
    printf("                       display_present: %d\n", featru_table->display_present);
    printf("                      num_call_follows: %d\n", featru_table->num_call_follows);
    printf("                         card_val_info: 0x%02x\t", featru_table->card_val_info);
    print_bits(featru_table->card_val_info, (char **)auth_bits_str);
    printf("                        accs_mode_info: 0x%02x\t", featru_table->accs_mode_info);
    print_bits(featru_table->accs_mode_info, (char **)accs_bits_str);
    printf("                    incoming_call_mode: 0x%02x\t%s\n", featru_table->incoming_call_mode, call_mode_str[featru_table->incoming_call_mode]);
    printf("          anti_fraud_for_incoming_call: 0x%02x\n", featru_table->anti_fraud_for_incoming_call);
    printf("                        OOS_POTS_flags: 0x%02x\t", featru_table->OOS_POTS_flags);
    print_bits(featru_table->OOS_POTS_flags, (char **)misc_flags_str);
    printf("                datajack_display_delay: %ds\n", featru_table->datajack_display_delay);
    printf("                     lang_scroll_order: 0x%02x\n", featru_table->lang_scroll_order);
    printf("                    lang_scroll_order2: 0x%02x\n", featru_table->lang_scroll_order2);
    printf("                      num_of_languages: %d\n", featru_table->num_of_languages);
    printf("                          rating_flags: 0x%02x\n", featru_table->rating_flags);
    printf("                      dialaround_timer: %d\n", featru_table->dialaround_timer);
    printf("       call_screen_list_ixl_oper_entry: 0x%02x\n", featru_table->call_screen_list_ixl_oper_entry);
    printf(" call_screen_list_inter_lata_aos_entry: 0x%02x\n", featru_table->call_screen_list_inter_lata_aos_entry);
    printf("        call_screen_list_ixl_aos_entry: 0x%02x\n", featru_table->call_screen_list_ixl_aos_entry);
    printf("                 datajack_grace_period: %d\n", featru_table->datajack_grace_period);
    printf("             operator_collection_timer: %d\n", featru_table->operator_collection_timer);
    printf("call_screen_list_intra_lata_oper_entry: 0x%02x\n", featru_table->call_screen_list_intra_lata_oper_entry);
    printf("call_screen_list_inter_lata_oper_entry: 0x%02x\n", featru_table->call_screen_list_inter_lata_oper_entry);
    printf("                     advertising_flags: 0x%02x\t", featru_table->advertising_flags);
    print_bits(featru_table->advertising_flags, (char **)advertising_bits_str);
    printf("                      default_language: %d\n", featru_table->default_language);
    printf("                call_setup_param_flags: 0x%02x\t", featru_table->call_setup_param_flags);
    print_bits(featru_table->call_setup_param_flags, (char **)call_setup_flags_str);
    printf("                         dtmf_duration: %d (%dms)\n", featru_table->dtmf_duration, featru_table->dtmf_duration * 10);
    printf("                      interdigit_pause: %d (%dms)\n", featru_table->interdigit_pause, featru_table->interdigit_pause * 10);
    printf("              ppu_preauth_credit_limit: %d\n", featru_table->ppu_preauth_credit_limit);
    printf("                 coin_calling_features: 0x%02x\t", featru_table->coin_calling_features);
    print_bits(featru_table->coin_calling_features, (char **)coin_calling_features_str);
    printf("             coin_call_overtime_period: %ds\n", featru_table->coin_call_overtime_period);
    printf("                   coin_call_pots_time: %ds\n", featru_table->coin_call_pots_time);
    printf("              international_min_digits: %d\n", featru_table->international_min_digits);
    printf("         default_rate_req_payment_type: %d\n", featru_table->default_rate_req_payment_type);
    printf("      next_call_revalidation_frequency: %d\n", featru_table->next_call_revalidation_frequency);
    printf("               cutoff_on_disc_duration: %d (%dms)\n", featru_table->cutoff_on_disc_duration, featru_table->cutoff_on_disc_duration * 10);
    printf("        cdr_upload_timer_international: %ds\n", featru_table->cdr_upload_timer_international);
    printf("             cdr_upload_timer_domestic: %ds\n", featru_table->cdr_upload_timer_domestic);
    printf("            num_perf_stat_dialog_fails: %d\n", featru_table->num_perf_stat_dialog_fails);
    printf("               num_co_line_check_fails: %d\n", featru_table->num_co_line_check_fails);
    printf("        num_alt_ncc_dialog_check_fails: %d\n", featru_table->num_alt_ncc_dialog_check_fails);
    printf("          num_failed_dialogs_until_oos: %d\n", featru_table->num_failed_dialogs_until_oos);
    printf("        num_failed_dialogs_until_alarm: %d\n", featru_table->num_failed_dialogs_until_alarm);
    printf("                       smartcard_flags: 0x%02x\t", featru_table->smartcard_flags);
    print_bits(featru_table->smartcard_flags, (char **)smartcard_flags_str);
    printf("      max_num_digits_manual_card_entry: %d\n", featru_table->max_num_digits_manual_card_entry);
    printf("         call_screen_list_zp_aos_entry: 0x%02x\n", featru_table->call_screen_list_zp_aos_entry);
    printf("                 carrier_reroute_flags: 0x%02x\t", featru_table->carrier_reroute_flags);
    print_bits(featru_table->carrier_reroute_flags, (char **)carrier_reroute_flags_str);
    printf("      min_num_digits_manual_card_entry: %d\n", featru_table->min_num_digits_manual_card_entry);
    printf("             max_num_smartcard_inserts: %d\n", featru_table->max_num_smartcard_inserts);
    printf("        max_num_diff_smartcard_inserts: %d\n", featru_table->max_num_diff_smartcard_inserts);
    printf("         call_screen_list_zm_aos_entry: 0x%02x\n", featru_table->call_screen_list_zm_aos_entry);
    printf("                        datajack_flags: 0x%02x\t", featru_table->datajack_flags);
    print_bits(featru_table->datajack_flags, (char **)datajack_flags_str);
    printf("              delay_on_hook_card_alarm: %d\n", featru_table->delay_on_hook_card_alarm);
    printf("   delay_on_hook_card_alarm_after_call: %d\n", featru_table->delay_on_hook_card_alarm_after_call);
    printf("                duration_of_card_alarm: %d\n", featru_table->duration_of_card_alarm);
    printf("                 card_alarm_on_cadence: %d\n", featru_table->card_alarm_on_cadence);
    printf("                card_alarm_off_cadence: %d\n", featru_table->card_alarm_off_cadence);
    printf(" delay_until_card_reader_blocked_alarm: %d\n", featru_table->delay_until_card_reader_blocked_alarm);
    printf("                       settlement_time: %d\n", featru_table->settlement_time);
    printf("                 grace_period_domestic: %d\n", featru_table->grace_period_domestic);
    printf("                           ias_timeout: %d\n", featru_table->ias_timeout);
    printf("            grace_period_international: %d\n", featru_table->grace_period_international);
    printf("        settlement_time_datajack_calls: %d\n", featru_table->settlement_time_datajack_calls);

    if (featru_table != NULL) {
        free(featru_table);
    }

    return (0);
}

void print_bits(uint8_t bits, char *str_array[])
{
    int i = 0;
    while (bits) {
        if (bits & 1) {
            printf("%s | ", str_array[i]);
        }
        bits >>= 1;
        i++;
    }
    printf("\n");
}