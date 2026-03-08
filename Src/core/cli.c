#include "core/cli.h"
#include "core/uprint.h"

#include <string.h>
#include <stddef.h>

#define CLI_BUFFER_SIZE         64

static uint8_t cli_comm_id = 0;
static command_t *cli_table = NULL;
static uint32_t cli_table_len = 0;

static char cli_buffer[CLI_BUFFER_SIZE];
static uint32_t cli_idx = 0;

void cli_setup(uint8_t comm_id, command_t *table, uint32_t Len)
{
    if(table != NULL)
    {
        cli_comm_id = comm_id;
        cli_table = table;
        cli_table_len = Len;
    }
}

void cli_dispatch(char *buffer)
{
    if(cli_table == NULL) return;
    if(buffer[0] == '\0') return;

    for (uint32_t i = 0; i < cli_table_len; i++)
    {
        if (strcmp(buffer, cli_table[i].name) == 0)
        {
            cli_table[i].execute();
            return;
        }
    }

    uprint("Unknown command: '%s'\r\n", buffer);
}

void cli_update(void)
{
    while(comm_data_available(cli_comm_id))
    {
        uint8_t ch;
        comm_receive(cli_comm_id, &ch, 1);
        comm_send(cli_comm_id, &ch, 1);

        if(ch == '\r')
        {
            ch = '\n';
            comm_send(cli_comm_id, &ch, 1);
        }

        if(ch == '\n')
        {
             cli_buffer[cli_idx] = '\0'; // command end
             cli_idx = 0;
             cli_dispatch(cli_buffer); // dispatch
        }else if(cli_idx < CLI_BUFFER_SIZE - 1)
        {
            cli_buffer[cli_idx++] = (char)ch;
        }
    }
}

void cli_help(void)
{
    if(cli_table == NULL) return;

    uprint("===================================\r\n");
    for (uint32_t i = 0; i < cli_table_len; i++)
    {
    	uprint("%s: %s\r\n", cli_table[i].name, cli_table[i].help);
    }
    uprint("===================================\r\n");
}