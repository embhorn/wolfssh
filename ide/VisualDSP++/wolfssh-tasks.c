/* wolfssh-tasks.c
 *
 * Copyright (C) 2006-2020 wolfSSL Inc.
 *
 * This file is part of wolfSSH.
 *
 * wolfSSH is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifdef WOLFSSL_USER_SETTINGS
    #include <wolfssl/wolfcrypt/settings.h>
#else
    #include <wolfssl/options.h>
#endif

#include <wolfssh/ssh.h>
#include <wolfssh/test.h>
#include "examples/server/server.h"
#include "examples/echoserver/echoserver.h"

#ifdef FUSION_RTOS
#include <fcl_os.h>

#define RESULT_BUF_SIZE  1024

typedef struct {
   int   isRunning;
   u8    buf[RESULT_BUF_SIZE];
   int   len;
} wolfssh_result_t;

static wolfssh_result_t _result = {0};

static fclThreadHandle _task = NULL;
#define WOLFSSH_TASK_STACK_SIZE (1024 * 100)
fclThreadPriority WOLFSSH_TASK_PRIORITY = (fclThreadPriority) (FCL_THREAD_PRIORITY_TIME_CRITICAL+1);

/*
static void fs_test(void)
{
    FCL_FILE* fp;
    
    
    printf("Enter fs_test\n");

    fp = FCL_FOPEN("C:\\KEYS\\SERVER-KEY-RSA.DER", "r");
    if (fp != NULL)
    {
        printf("\tSuccess opening SERVER-KEY-RSA.DER\n");
        FCL_FCLOSE(fp);
    }
    else
    {
        printf("\tFailed opening SERVER-KEY-RSA.DER\n");
        return;
    }
    
    fp = FCL_FOPEN("C:\\TEST\\FS-TEST.TXT", "wb");
    if (fp != NULL)
    {
        if (FCL_FWRITE("HELLO WORLD", 1, 13, fp) == 13)
            printf("\tSuccess writing C:\\TEST\\FS-TEST.TXT\n");
        else
            printf("\tFailed writing to C:\\TEST\\FS-TEST.TXT\n");
    
        FCL_FCLOSE(fp);
    }
    else
    {
        printf("\tFailed opening SERVER-KEY-RSA.DER\n");
        return;
    }
    
}
*/
static int scp_server_taskEnter(void *args)
{
    int ret;

    printf("Enter scp_server_taskEnter\n");

    #ifdef DEBUG_WOLFSSH
        wolfSSH_Debugging_ON();
    #endif

    printf("Call server_test\n");
    ret = server_test(args);

    printf("Result of server_test was %d\n", ret);

    _result.isRunning = 0;
    fosTaskDelete(_task);

    return 0;
}

static int ssh_echoserver_taskEnter(void *args)
{
    int ret;

    printf("Enter ssh_echoserver_taskEnter\n");

    #ifdef DEBUG_WOLFSSH
        wolfSSH_Debugging_ON();
    #endif

    printf("Call echoserver_test\n");
    ret = echoserver_test(args);

    printf("Result of echoserver_test was %d\n", ret);
    wolfSSH_Cleanup();

    _result.isRunning = 0;
    fosTaskDelete(_task);

    return 0;
}

int wolfssh_task_start(void* voidinfo, char* argline)
{
    char optionA[] = "ssh_echoserver";
    char optionB[] = "scp_server";
    fssShellInfo *info = (fssShellInfo*)voidinfo;
    struct wolfArgs args;

//    fs_test();

    if(_result.isRunning) {
        fssShellPuts(info, "previous task still running\r\n");
        return 1;
    }

   _result.isRunning = 1;

   if (FCL_STRNCMP(argline, optionA, FCL_STRLEN(optionA)) == 0) {
         _task = fclThreadCreate(ssh_echoserver_taskEnter,
                                 argline,
                                 WOLFSSH_TASK_STACK_SIZE,
                                 WOLFSSH_TASK_PRIORITY);
   }
   else if (FCL_STRNCMP(argline, optionB, FCL_STRLEN(optionB)) == 0) {
        _task = fclThreadCreate(scp_server_taskEnter,
                                &args,
                                WOLFSSH_TASK_STACK_SIZE,
                                WOLFSSH_TASK_PRIORITY);
   }
   else {
   	    printf("Invalid input: %s\n", argline);
   	    printf("Please try with ssh_echoserver or scp_server\n");
   	    return -1;
   }
   
   FCL_ASSERT(_task != FCL_THREAD_HANDLE_INVALID);

   return 0;
}

#endif /* FUSION_RTOS */
