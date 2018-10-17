/* Copyright Statement:                                                        
 *                                                                             
 * This software/firmware and related documentation ("MediaTek Software") are  
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any     
 * reproduction, modification, use or disclosure of MediaTek Software, and     
 * information contained herein, in whole or in part, shall be strictly        
 * prohibited.                                                                 
 *                                                                             
 * MediaTek Inc. (C) 2014. All rights reserved.                                
 *                                                                             
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")     
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER  
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL          
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR          
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH 
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,            
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.   
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK       
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE  
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR     
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S 
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE       
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE  
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE  
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    
 *                                                                             
 * The following software/firmware and/or related documentation ("MediaTek     
 * Software") have been modified by MediaTek Inc. All revisions are subject to 
 * any receiver's applicable license agreements with MediaTek Inc.             
 */



/*-----------------------------------------------------------------------------

 * $RCSfile: cli_input.c,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/15 $
 * $SWAuthor: Clear Case Administrator $
 * $MD5HEX: 96b0a89975193784c9eddfef11dec3ef $
 *
 * Description:
 *         This program will handle input from UART to CLI console.
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>


#include "u_common.h"
#include "u_os.h"
#include "u_dbg.h"
#include "_cli.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    data declarations
 ----------------------------------------------------------------------------*/

static pthread_t        h_cli_thread;
static BOOL             b_cli_init = FALSE;
static CLI_OP_MODE      e_cli_enabled;

static UINT32           ui4_cli_cmd_buf_row_idx;
static UINT32           ui4_cli_cmd_buf_ref_row_idx;
static UINT32           ui4_cli_cmd_buf_idx;
static CHAR             s_cli_prompt_str[CLI_CMD_BUF_SIZE];
static CHAR             s_cli_cmd_buf[CLI_CMD_BUF_ROW_NUM][CLI_CMD_BUF_SIZE];


/*-----------------------------------------------------------------------------
                    function declarations
 ----------------------------------------------------------------------------*/
static int to_sched_priority(UINT8 ui1_priority)
{
    int sched_priority;
    sched_priority = 100 - (int)ui1_priority * 100 / 256;
    if (sched_priority < 1) sched_priority = 1;
    if (sched_priority > 99) sched_priority = 99;
    return sched_priority;
}

static void _test(int signo)
{
    dbg_print("signo is %d\n", signo);
}

static int _set_input_mode (VOID)
{
    #define ECHOFLAGS   (ECHO | ECHOE | ECHOK | ECHONL)
    INT32   fd;    //FD for getchar
    int err;
    struct termios term;

#if 1
	fd = open("/dev/ttyS0",O_RDWR);
	if(-1 == fd)
	{
		dbg_print("Can not open /dev/ttyS0\n");
	}
	else
	{
		dbg_print("open /dev/ttyS0 OK\n");
	}
#else
	fd = STDIN_FILENO;
#endif
    if(tcgetattr(fd, &term) == -1)
    {
        dbg_print("Can not get the attribution of the terminal\n");
        dbg_print("errno is %d, strerrno is %s\n",errno,strerror(errno));
        return 1;
    }

    dbg_print("Set input mode\n");
    term.c_lflag &= ~ECHOFLAGS;
    term.c_lflag &= (~ICANON);
    term.c_cc[VTIME] = 0;
    term.c_cc[VMIN] = 1;
    /* disable sigint */
    term.c_cc[VINTR] = 0;

//  sa.sa_sigaction = &_test;
//   sigemptyset(&sa.sa_mask);
//  sa.sa_flags = SA_SIGINFO;

    err = tcsetattr(fd, TCSANOW, &term);

    if (SIG_ERR == signal(SIGTTOU, _test))
    {
        dbg_print("Change Int error\n");
    }

//  if ((err = sigaction(SIGSEGV, &sa, NULL)) != 0) return err;
//    if ((err = sigaction(SIGINT, &sa, NULL)) != 0) return err;
//    if ((err = sigaction(SIGUSR1, &sa, NULL)) != 0) return err;

    if(-1 == err && EINTR == err)
    {
        dbg_print("Cannot set the attribution of the terminal");
        return 1;
    }

    return 0;
}

static inline CHAR _tolower(CHAR c)
{
    if (( c >= 'A' )&& ( c <= 'Z'))
        c -= 'A'-'a';
    return c;
}

static inline CHAR _toupper(CHAR c)
{
    if (( c >= 'a' )&& ( c <= 'z'))
        c -= 'a'-'A';
    return c;
}

static inline CHAR _go_back_along_alp(CHAR c, CHAR key)
{
    key = key % 46;
    if((c - key) < '-')
        key = 'Z' - (key - (c - '-' + 1));
    else
        key = c - key;
    return key;
}
/*-----------------------------------------------------------------------------
 * Name: _cli_is_encrypt_cmd
 *
 * Description: decrypt cli encrypted command.
 *
 * Inputs:  encrypted command string
 *
 * Outputs: -
 *
 * Returns: success return the length of the string, else 0
 ----------------------------------------------------------------------------*/
static BOOL _cli_is_encrypt_cmd(CHAR *cmd)
{
    CLI_DBG("func(%s) %s cmd[0]:%c\n",__FUNCTION__,cmd,cmd[0]);

    if( *cmd == '~')
        return TRUE;
    else
        return FALSE;
}
/*-----------------------------------------------------------------------------
 * Name: _cli_decrypt_cmd
 *
 * Description: decrypt cli encrypted command.
 *
 * Inputs:  encrypted command string
 *
 * Outputs: -
 *
 * Returns: success return the length of the string, else 0
 ----------------------------------------------------------------------------*/
static UINT32 _cli_decrypt_cmd(CHAR *cmd)
{
    CHAR c_temp_cmd[CLI_CMD_BUF_SIZE]={0};
    CHAR i4_key;
    UINT32 i4_i;
    UINT32 i4_in_len;

    CLI_DBG("func(%s) before cmd:%s\n",__FUNCTION__,cmd);
    cmd = cmd + 1;

    i4_in_len = strlen((CHAR *)cmd);

    for(i4_i = 0 ; i4_i < i4_in_len -1; i4_i++)
    {
        if(cmd[i4_i] >= 'a' && cmd[i4_i] <= 'z')
        {
            cmd[i4_i] -= 38;
        }
    }
    i4_key = _tolower(cmd[i4_in_len - 1]);

    for(i4_i = 0; i4_i < i4_in_len - 1 ; i4_i++)
    {
        c_temp_cmd[i4_i] =
            _tolower(_go_back_along_alp(_toupper(cmd[i4_i]),i4_key));
        i4_key  = cmd[i4_i];
    }

    for(i4_i = 0; i4_i < i4_in_len - 1; i4_i++)
    {
        if(c_temp_cmd[i4_i] == 45)
            c_temp_cmd[i4_i] = ' ';
        if(c_temp_cmd[i4_i] == ':')
            c_temp_cmd[i4_i] = '_';
    }

    i4_i = 0;
    cmd = cmd - 1;
    //copy the decrypted command to cmd
    for(i4_i = 0; i4_i < i4_in_len - 1; i4_i++)
    {
        cmd[i4_i] = c_temp_cmd[i4_i];
    }

    cmd[i4_i] = '\0';
    CLI_DBG("func(%s) after cmd:%s\n",__FUNCTION__,cmd);
    return i4_in_len;
}


static BOOL _cli_check_input(UINT8 ui1_data)
{

#if 0
    /* Enable or disable CLI */
    if (e_cli_enabled == CLI_DISABLED)
    {
        if (ASCII_KEY_CTRL_C == ui1_data)
        {
            e_cli_enabled = CLI_ENABLING;
        }
        return FALSE;
    }
    else if (e_cli_enabled == CLI_ENABLED)
    {
        if (ASCII_KEY_CTRL_C == ui1_data)
        {

            e_cli_enabled = CLI_DISABLED;
            CLI_DBG("\n\r===========\n\r");
            CLI_DBG("CLI is OFF\n\r");
            CLI_DBG("===========\n\r");
            return FALSE;
        }
    }
#endif

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * Name: _cli_get_char
 *
 * Description: This API waits for the first available character.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: ASCII_NULL      The UART queue is empty.
 *          Others          The first available character from UART queue.
 ----------------------------------------------------------------------------*/
static CHAR _cli_get_char(VOID)
{
    UINT8 bData = 0;

    bData = (UINT8)getchar();

    return bData;

}
#if 0
/*-----------------------------------------------------------------------------
 * Name: _cli_get_string
 *
 * Description: This API waits for the first available string.
 *
 * Inputs:  b_show_srt      Indicate if star signs are shown when character
 *                          input.
 *
 * Outputs: ps_str_buf      The first available string from UART queue.
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID _cli_get_string(CHAR* ps_str_buf,
                    BOOL  b_show_star)
{
    CHAR        c_char;
    UINT32      ui4_idx = 0;

    BOOL        fgFilterNL_InStart = TRUE;

    do
    {
        c_char = _cli_get_char();

        if (0xf0 == c_char || 0xef == c_char)
        {
            return;
        }

        if (fgFilterNL_InStart && ASCII_KEY_NL == c_char)
        {
            continue;
        }

        fgFilterNL_InStart = FALSE;

        if ((c_char != ASCII_KEY_NL) &&
            (c_char != ASCII_NULL))
        {
            if (c_char == ASCII_KEY_ESC)
            {
                c_char = _cli_get_char();
                if (c_char == ASCII_KEY_LEFT)
                {
                    if (ui4_idx > 0)
                    {
                        CLI_DBG("%c%c%c", ASCII_KEY_BS, ASCII_KEY_SPACE, ASCII_KEY_BS);
                        ui4_idx--;
                    }
                }
            }
            else if (c_char == ASCII_KEY_BS)
            {
                if (ui4_idx > 0)
                {
                    CLI_DBG("%c%c%c", ASCII_KEY_BS, ASCII_KEY_SPACE, ASCII_KEY_BS);
                    ui4_idx--;
                }
            }
            else
            {
                if ((c_char >= ASCII_KEY_PRINTABLE_MIN) &&
                    (c_char <= ASCII_KEY_PRINTABLE_MAX))
                {
                    if (ui4_idx == CLI_CMD_BUF_SIZE)
                    {
                        continue;
                    }

                    ps_str_buf[ui4_idx++] = c_char;
                    CLI_DBG("*");
                }
            }
        }
    } while ((c_char != ASCII_KEY_NL)&&(c_char != ASCII_KEY_ENTER));

    ps_str_buf[ui4_idx] = ASCII_NULL;
}
#endif//0
/*-----------------------------------------------------------------------------
 * Name: _cli_input_thread
 *
 * Description: This is CLI input thread for handling input from UART.
 *
 * Inputs:  pv_arg   Input argument
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void * _cli_input_thread(VOID* pv_arg)
{
    INT32       i4_return = CLIR_OK;
    CHAR        c_char = 0;
    UINT32      ui4_idx;

    CLI_DBG("start cli thread.\n");

    if(_set_input_mode())
        dbg_print("Set input mode error\n");

    /* Initialize input buffers */
    for (ui4_idx = 0; ui4_idx < CLI_CMD_BUF_ROW_NUM; ui4_idx++)
    {
        s_cli_cmd_buf[ui4_idx][0] = ASCII_NULL;
    }

    //e_cli_enabled = CLI_DISABLED;
    e_cli_enabled = CLI_ENABLED;
    while (1)
    {

#if 0
        if (e_cli_enabled == CLI_ENABLING)
        {
            for (ui4_idx = 0; ui4_idx < MAX_CLI_ENABLE_PASSWD_TRY; ui4_idx++)
            {
                if (x_cli_passwd_auto_adapt(FALSE)== CLIR_OK)
                {
                    e_cli_enabled = CLI_ENABLED;
                    x_dbg_stmt("\n===========\n\r");

                    x_dbg_stmt("CLI is ON\n\r");

                    x_dbg_stmt("===========\n\r");
                    x_dbg_stmt("%s", s_cli_prompt_str);
                    break;
                }
                x_dbg_stmt("\n\r");

                /* Reset command buffer indexes */
                ui4_cli_cmd_buf_row_idx = 0;
                ui4_cli_cmd_buf_idx = 0;
            }

            if (ui4_idx == MAX_CLI_ENABLE_PASSWD_TRY)
            {
                x_dbg_stmt("Oops! you are having a trouble, try again...\n\r\n\r");
                e_cli_enabled = CLI_DISABLED;
            }
        }
#endif//0

        c_char = _cli_get_char();

        if (FALSE == _cli_check_input((UINT8)c_char))
        {
            u_thread_delay(40);
            continue;
        }

        switch (c_char)
        {
            case ASCII_KEY_NL:
            case ASCII_KEY_ENTER:
                s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][ui4_cli_cmd_buf_idx] = ASCII_NULL;

                dbg_print("\n\r");

                /*add by mingkang start*/
                if(_cli_is_encrypt_cmd(s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx]))
                {
                    _cli_decrypt_cmd(s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx]);
                    ui4_cli_cmd_buf_idx -= 2;
                }
                /*add by mingkang end*/

                if(!strncmp(s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx], "quit", 4))
                {
                    dbg_print("Exit cli prog\n");
                    goto END;
                }
                /* Parse input CLI command */
                i4_return = cli_parser(s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx]);
                if (i4_return == CLIR_CMD_NOT_FOUND)
                {
                    dbg_print("Invalid command\n\r");
                }
                else if (i4_return == CLIR_DIR_NOT_FOUND)
                {
                    dbg_print("Invalid directory\n\r");
                }
                else if (i4_return < CLIR_OK)
                {
                    dbg_print("Command execution error\n\r");
                }

                /* Scroll to next row of CLI commad buffer */
                if (ui4_cli_cmd_buf_idx > 0)
                {
                    ui4_cli_cmd_buf_row_idx = (ui4_cli_cmd_buf_row_idx + 1) & (CLI_CMD_BUF_ROW_NUM - 1);
                    s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][0] = ASCII_NULL;
                }

                ui4_cli_cmd_buf_idx = 0;
                ui4_cli_cmd_buf_ref_row_idx = ui4_cli_cmd_buf_row_idx;

                if (e_cli_enabled == CLI_ENABLED)
                {
                    dbg_print("\n\r%s", s_cli_prompt_str);
                }
                break;

            case ASCII_KEY_BS:
                if (ui4_cli_cmd_buf_idx > 0)
                {
                    CHAR   LogBuf[10];
                    memset(LogBuf,0,sizeof(LogBuf));
                    snprintf(LogBuf,9,"%c%c%c", ASCII_KEY_BS, ASCII_KEY_SPACE, ASCII_KEY_BS);
                    dbg_print(LogBuf);
                    ui4_cli_cmd_buf_idx--;
                }
                break;

            case ASCII_KEY_ESC:
                c_char = _cli_get_char();
                if (c_char == ASCII_KEY_ARROW)
                {
                    c_char = _cli_get_char();
                    switch (c_char)
                    {
                        case ASCII_KEY_UP:
                            /* Clear terminal display */
                            while (ui4_cli_cmd_buf_idx > 0)
                            {
                                dbg_print("%c%c%c", ASCII_KEY_BS, ASCII_KEY_SPACE, ASCII_KEY_BS);
                                ui4_cli_cmd_buf_idx--;
                            }
                            s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][0] = ASCII_NULL;

                            /* Load previous reference row */
                            do
                            {
                                ui4_cli_cmd_buf_ref_row_idx = (ui4_cli_cmd_buf_ref_row_idx - 1) & (CLI_CMD_BUF_ROW_NUM - 1);
                                if (ui4_cli_cmd_buf_ref_row_idx == ui4_cli_cmd_buf_row_idx)
                                {
                                    break;
                                }
                            } while (s_cli_cmd_buf[ui4_cli_cmd_buf_ref_row_idx][0] == ASCII_NULL);

                            memcpy(&s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][0],
                                     &s_cli_cmd_buf[ui4_cli_cmd_buf_ref_row_idx][0],
                                     CLI_CMD_BUF_SIZE);

                            /* Show the row content */
                            ui4_cli_cmd_buf_idx = 0;
                            while (s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][ui4_cli_cmd_buf_idx] != ASCII_NULL)
                            {
                                dbg_print("%c", s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][ui4_cli_cmd_buf_idx]);
                                ui4_cli_cmd_buf_idx++;
                            }
                            break;

                        case ASCII_KEY_DOWN:
                            /* Clear terminal display */
                            while (ui4_cli_cmd_buf_idx > 0)
                            {
                                dbg_print("%c%c%c", ASCII_KEY_BS, ASCII_KEY_SPACE, ASCII_KEY_BS);
                                ui4_cli_cmd_buf_idx--;
                            }
                            s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][0] = ASCII_NULL;

                            /* Load next reference row */
                            do
                            {
                                ui4_cli_cmd_buf_ref_row_idx = (ui4_cli_cmd_buf_ref_row_idx + 1) & (CLI_CMD_BUF_ROW_NUM - 1);
                                if (ui4_cli_cmd_buf_ref_row_idx == ui4_cli_cmd_buf_row_idx)
                                {
                                    break;
                                }
                            } while (s_cli_cmd_buf[ui4_cli_cmd_buf_ref_row_idx][0] == ASCII_NULL);

                            memcpy(&s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][0],
                                     &s_cli_cmd_buf[ui4_cli_cmd_buf_ref_row_idx][0],
                                     CLI_CMD_BUF_SIZE);

                            /* Show the row content */
                            ui4_cli_cmd_buf_idx = 0;
                            while (s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][ui4_cli_cmd_buf_idx] != ASCII_NULL)
                            {
                                dbg_print("%c", s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][ui4_cli_cmd_buf_idx]);
                                ui4_cli_cmd_buf_idx++;
                            }
                            break;

                        case ASCII_KEY_LEFT:
                            if (ui4_cli_cmd_buf_idx > 0)
                            {
                                dbg_print("%c%c%c", ASCII_KEY_BS, ASCII_KEY_SPACE, ASCII_KEY_BS);
                                ui4_cli_cmd_buf_idx--;
                            }
                            break;

                        case ASCII_KEY_RIGHT:
                            break;
                        default:
                            dbg_print("%c", ASCII_NULL);

                    }
                }
                break;

            default:
                if ((c_char >= ASCII_KEY_PRINTABLE_MIN) &&
                    (c_char <= ASCII_KEY_PRINTABLE_MAX))
                {
                    if (ui4_cli_cmd_buf_idx == CLI_CMD_BUF_SIZE)
                    {
                        /* Command buffer overflow */
                        //continue;
                        break;
                    }
                    else
                    {
                        /* Store input character into command buffer */
                        s_cli_cmd_buf[ui4_cli_cmd_buf_row_idx][ui4_cli_cmd_buf_idx++] = c_char;
                    }
                }
                break;
        }
        u_thread_delay(40);
    }

END:

    return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: cli_init
 *
 * Description: CLI initialization function.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                     Routine successful.
 *          CLIR_ALREADY_INIT           The CLI has already been initialized.
 ----------------------------------------------------------------------------*/
INT32 cli_init(VOID)
{
    struct sched_param param;
    SIZE_T  z_stack_size;
    INT32 i4_ret;

    CLI_DBG("start CLI init.\n");

    if (b_cli_init)
    {
        dbg_print("CLI already init.\n");
        return CLIR_ALREADY_INIT;
    }

    /* Initialize internal parameters */
    ui4_cli_cmd_buf_row_idx = 0;
    ui4_cli_cmd_buf_ref_row_idx = 0;
    ui4_cli_cmd_buf_idx = 0;


    /* Initialize default command tables and set up prompt string */
    if (cli_parser_clear_cmd_tbl() != CLIR_OK) /* Mandatory table will be loaded here */
    {
        CLI_ASSERT(0);
    }

    //create cli thread
    if (OSR_OK != u_thread_create(&h_cli_thread,
                             CLI_THREAD_NAME,
                             CLI_THREAD_DEFAULT_STACK_SIZE,
                             CLI_THREAD_DEFAULT_PRIORITY,
                             _cli_input_thread,
                             0,
                             NULL))
    {
        CLI_ASSERT(0);
    }

    b_cli_init = TRUE;

    return CLIR_OK;

}

/*-----------------------------------------------------------------------------
 * Name: cli_get_prompt_str_buf
 *
 * Description: This API returns the address of CLI prompt string buffer.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: The address of CLI prompt string buffer.
 ----------------------------------------------------------------------------*/
CHAR* cli_get_prompt_str_buf(VOID)
{
    return s_cli_prompt_str;

}

/*-----------------------------------------------------------------------------
 * Name: cli_is_inited
 *
 * Description: This API returns the initialization status of CLI component.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: TRUE        The CLI has already been initialized.
 *          FALSE       The CLI has not been initialized.
 ----------------------------------------------------------------------------*/
BOOL cli_is_inited(VOID)
{
    return b_cli_init;
}


VOID cli_init_phase1(VOID)
{

    if (b_cli_init)
    {
        dbg_print("CLI already init.\n");
        return CLIR_ALREADY_INIT;
    }

    /* Initialize internal parameters */
    ui4_cli_cmd_buf_row_idx = 0;
    ui4_cli_cmd_buf_ref_row_idx = 0;
    ui4_cli_cmd_buf_idx = 0;


    /* Initialize default command tables and set up prompt string */
    if (cli_parser_clear_cmd_tbl() != CLIR_OK) /* Mandatory table will be loaded here */
    {
        CLI_ASSERT(0);
    }

    /* Initialize thread descriptor structure */

    b_cli_init = TRUE;


}
VOID cli_init_phase2(VOID)
{
    _cli_input_thread(NULL);

}

