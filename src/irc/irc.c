#include "irc/irc.h"
#include "util/util.h"

#include <libutil/container/list.h>

#include <strings.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/*
 * IRC message handling
 */
int irc_parse_message(const char *line, struct irc_message *msg)
{
    const char *prev = line;
    const char *next = line;
    size_t len;

    memset(msg, 0, sizeof(*msg));

    /* Check for prefix */
    if (*prev == ':') {
        ++prev; /* skip the ':' */

        /* Prefix must not be the last part in a message */
        if (!(next = strchr(prev, ' ')))
            return 1;

        /* Calculate token lenght and skip the space */
        len = (size_t)(next - prev);
        while (*next == ' ')
            ++next;

        strncpy(msg->prefix, prev, MIN(len, sizeof(msg->prefix) - 1));
        prev = next;
    }

    /* Command also must not be the last argument in a message */
    if (!(next = strchr(next, ' ')))
        return 1;

    len = (size_t)(next - prev);
    while (*next == ' ')
        ++next;

    char cmdbuf[16] = {0};
    strncpy(cmdbuf, prev, MIN(len, sizeof(cmdbuf) - 1));

    if ((msg->command = irc_string_to_command(cmdbuf)) == (enum irc_command)-1)
        return 1;

    do {
        prev = next;
        next = strchr(next, ' ') ? strchr(next, ' ') : next + strlen(next);
        len = (size_t)(next - prev);

        while (*next == ' ')
            ++next;

        if (*prev != ':') {
            strncpy(msg->params[msg->paramcount++], prev,
                    MIN(len, sizeof(msg->params[0]) - 1));
        } else {
            ++prev;
            len = strlen(prev);

            strncpy(msg->msg, prev, MIN(len, sizeof(msg->msg) - 1));

            break;
        }
    } while (next < line + strlen(line));

    return 0;
}

void irc_print_message(const struct irc_message *i)
{
    if (strlen(i->prefix))
        printf(":%s ", i->prefix);

    printf("%s", irc_command_to_string(i->command));

    for (int j = 0; j < i->paramcount; j++)
        printf(" %s", i->params[j]);

    if (strlen(i->msg))
        printf(" :%s", i->msg);

    printf("\n");
}

unsigned irc_message_to_string(const struct irc_message *i,
                               char *dest,
                               size_t n)
{
    int pos = 0;
    pos += snprintf(dest + pos, MAX((int)n - pos, 0), "%s",
            irc_command_to_string(i->command));

    for (int j = 0; j < i->paramcount; ++j)
        pos += snprintf(dest + pos, MAX((int)n - pos, 0), " %s", i->params[j]);

    if (strlen(i->msg) > 0)
        pos += snprintf(dest + pos, MAX((int)n - pos, 0), " :%s", i->msg);

    return strlen(dest);
}

unsigned irc_message_size(const struct irc_message *i)
{
    char buffer[IRC_MESSAGE_MAX] = {0};

    return irc_message_to_string(i, buffer, sizeof(buffer));
}


/*
 * User comparison based on nick/host
 */
int irc_user_cmp(const char *a, const char *b)
{
    /*
     * Since nicks are enough to identify someone uniquely on a network,
     * we can skip the whole address part and compare complete prefixes
     * only until the end of the nick part.
     */
    const char *p = NULL;

    size_t reallen_a = (p = strchr(a, '!')) ? (size_t)(p - a) : strlen(a);
    size_t reallen_b = (p = strchr(b, '!')) ? (size_t)(p - b) : strlen(b);
    size_t cmplen = MIN(reallen_a, reallen_b);

    return (reallen_a != reallen_b) ? 1 : strncasecmp(a, b, cmplen);
}


const char *irc_command_to_string(enum irc_command cmd)
{
    switch (cmd) {
#define X(name, num) case num: return (("000" #num) + (num < 10 ?  1 : \
                                                      (num < 100 ? 2 : \
                                                                   3)));
        IRC_NUMERICS
#undef X
#define X(cmd) case CMD_ ## cmd: return #cmd;
        IRC_COMMANDS;
#undef X
        default: return NULL;
    }
}

enum irc_command irc_string_to_command(const char *cmd)
{
    if ((strlen(cmd) == 3) && (isdigit(*(cmd))
                            && isdigit(*(cmd + 1))
                            && isdigit(*(cmd + 2)))) {
#define X(numeric, num) \
        if (atoi(cmd) == num) return numeric; else
    IRC_NUMERICS
#undef X
        return -1;
    } else {
#define X(ecmd) \
        if (!strcmp(cmd, #ecmd)) return CMD_ ## ecmd; else
    IRC_COMMANDS
#undef X
        return -1;
    }
}
