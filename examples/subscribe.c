/*****************************************************************************
*
*  Copyright (C) 2015 Exosite LLC
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <time.h>
#include "exosite.h"

const char VENDOR[] = "patrick";
const char MODEL[] =  "generic_test";
const char SERIAL[] = "001";

void command_handler (char *value)
{
    printf("Got Command: %s\n", value);
}

int main(void)
{
    long long unsigned int loopcount = 0, errorcount = 0;
    char read_str[32];
    char loop_str[16];
    char error_str[16];
    const uint8_t op_count = 4;
    exo_op ops[op_count];

    exo_init(VENDOR, MODEL, SERIAL);

    for (int i = 0; i < op_count; i++){
        exo_op_init(&ops[i]);
    }

    // only need to setup subscribe once
    exo_subscribe(&ops[1], "command", read_str, 32);
    
    while(1)
    {
        if (loopcount % 100 == 0){
            // prepare data to write
            snprintf(loop_str, 15, "%llu", loopcount);

            // queue write operation
            exo_write(&ops[2], "uptime", loop_str);
        }

        // perform queued operations until all are done or failed
        while(exo_operate(ops, op_count) != EXO_IDLE);

        // check if ops succeeded or failed
        for (int i = 1; i < op_count; i++){
            if (exo_is_op_finished(&ops[i])) {
                if (exo_is_op_success(&ops[i])) {
                    if (exo_is_op_read(&ops[i]) || exo_is_op_subscribe(&ops[i])) {
                        printf("[SUCCESS] got '%s' = `%s`\n", ops[i].alias, ops[i].value);
                    } else if (exo_is_op_write(&ops[i])) {
                        printf("[SUCCESS] set '%s' = `%s`\n", ops[i].alias, ops[i].value);
                    } else {
                        printf("[WARNING] something succeeded, but I don't know what\n");
                    }
                } else {
                    printf("[ERROR] on '%s'\n", ops[i].alias);
                    printf("        error count is now %llu\n", errorcount);

                    // queue a write to error count next time
                    snprintf(error_str, 15, "%llu", errorcount);
                    exo_write(&ops[3], "errorcount", error_str);
                }

                exo_op_done(&ops[i]);
            }
        }

        nanosleep((struct timespec[]){{0, 500000000}}, NULL);
        loopcount++;
    }
    return 0;
}
