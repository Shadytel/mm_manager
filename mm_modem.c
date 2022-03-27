/*
 * This is a "Manager" for the Nortel Millennium payhone.
 *
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 */

#include <stdio.h>  /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h> /* String function definitions */
#include <time.h>
#include <fcntl.h>  /* File control definitions */
#include <errno.h>  /* Error number definitions */
#ifdef _WIN32
# include <windows.h>
#else  /* ifdef _WIN32 */
# include <unistd.h>  /* UNIX standard function definitions */
# include <termios.h> /* POSIX terminal control definitions */
#endif /* _WIN32 */
#include "./mm_serial.h"

/* Static function declarations */
static int send_at_command(int fd, char *command);

/* Initialize modem with a series of AT commands */
int        init_modem(int fd) {
    int status;

    printf("Reset modem.\n");
    status = send_at_command(fd, "ATZ");

    if (status != 0) {
        return -1;
    }

    printf("Disable modem command echo.\n");
    status = send_at_command(fd, "ATE=1");

    if (status != 0) {
        return -1;
    }

    printf("Set modulation to Bell 212A.\n");

    // status = send_at_command(fd, "AT&N2");        // 3-Com Business Modem 56K USB (use 1200 baud)
    // status = send_at_command(fd, "ATB1");         // USR 5686 Modem
    status = send_at_command(fd, "AT+MS=B212");      // Lenovo 56K USB Modem

    if (status != 0) {
        return -1;
    }

    printf("Set carrier wait timeout to 3 seconds.\n");
    status = send_at_command(fd, "ATS7=3"); /* Wait 3 seconds for carrier. */

    if (status != 0) {
        return -1;
    }

    printf("Set modem autoanswer.\n");
    status = send_at_command(fd, "ATS0=1");

    return status;
}

/* Wait for modem to connect */
int wait_for_modem_response(int fd, const char *match_str, int max_tries) {
    char buffer[255] = { 0 }; /* Input buffer */
    uint8_t bufindex = 0;
    int     tries    = 0;     /* Number of tries so far */

    drain_serial(fd);

    do {
        ssize_t nbytes; /* Number of bytes read */
        bufindex  = 0;
        buffer[0] = '\0';

        /* read characters into our string buffer until we get a CR or NL */
        while ((nbytes = read_serial(fd, &buffer[bufindex], 1)) > 0) {
            bufindex        += (uint8_t)nbytes;
            buffer[bufindex] = '\0';

            if ((buffer[bufindex - 1] == '\n') || (buffer[bufindex - 1] == '\r') || (bufindex >= (sizeof(buffer) - 1))) break;
        }

        /* See if we got the expected response */
        if (strstr(buffer, match_str) != 0) {
            return 0;
        }
        tries++;
    } while (tries < max_tries);

    return -1;
}

int hangup_modem(int fd) {
    int tries; /* Number of tries so far */

    for (tries = 0; tries < 3; tries++) {
        flush_serial(fd);

        for (int i = 0; i < 3; i++) {
            write_serial(fd, "+", 1);
#ifdef _WIN32
            Sleep(100);
#else  /* ifdef _WIN32 */
            nanosleep((const struct timespec[]) { { 0, 100 * 1000000L } }, NULL);
#endif /* ifdef _WIN32 */
        }

#ifdef _WIN32
        Sleep(1000);
#else  /* ifdef _WIN32 */
        sleep(1); /* Some modems need time to process the AT command. */
#endif /* ifdef _WIN32 */

        if (wait_for_modem_response(fd, "OK", 1) == 0) {
            return send_at_command(fd, "ATH0");
        }
    }
    return -1;
}

/* Send AT Command to Modem */
static int send_at_command(int fd, char *command) {
    char buffer[80]; /* Input buffer */
    int  tries;      /* Number of tries so far */

    for (tries = 0; tries < 3; tries++) {
        flush_serial(fd);
        snprintf(buffer, sizeof(buffer), "%s\r", command);

        /* send an AT command followed by a CR */
        if (write_serial(fd, buffer, strnlen(buffer, sizeof(buffer))) == 0) {
            continue;
        }

        /* Some modems need time to process the AT command. */
#ifdef _WIN32
        Sleep(100);
#else  /* ifdef _WIN32 */
        nanosleep((const struct timespec[]) { { 0, 100 * 1000000L } }, NULL);
#endif /* _WIN32 */

        if (wait_for_modem_response(fd, "OK", 5) == 0) {
            return 0;
        }
    }
    return -1;
}
