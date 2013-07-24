#ifndef _SESSION_H_
#define _SESSION_H_

#include <time.h>
#include <stdint.h>

#include "irc/irc.h"
#include "util/log.h"


/* Some generic buffer sizes */
#define HOSTNAME_MAX 256
#define BUFFER_MAX (1024 * 8) /* 8 KiB */

#define NICK_MAX 32
#define USER_MAX 16
#define REAL_MAX 64

/* Time in seconds after which the connection is tested for aliveness */
#define TIMEOUT 300 /* 5 minutes */

/* Time in seconds after which an idle event should be issued */
#define IDLE_INTERVAL 1


struct irc_callbacks
{
    void *arg;

    /*
     * Callbacks with modifiable arguments.
     */

    /* Called when sending a message. Can modify message. Nonzero return value
     * aborts sending. Can be used to filter or substitute. */
    int (*on_send_message)(void *arg, struct irc_message *m);

    /*
     * Read-only callbacks for IRC events.
     */
    int (*on_event)(void *arg, const struct irc_message *m);
    int (*on_ping)(void *arg);

    int (*on_privmsg)(void *arg,
            const char *prefix,
            const char *target,
            const char *msg);

    int (*on_notice)(void *arg,
            const char *prefix,
            const char *target,
            const char *msg);

    int (*on_join)(void *arg, const char *prefix, const char *channel);

    int (*on_part)(void *arg,
            const char *prefix,
            const char *channel,
            const char *reason);

    int (*on_quit)(void *arg, const char *prefix, const char *reason);

    int (*on_kick)(void *arg,
            const char *prefix_kicker,
            const char *prefix_kicked,
            const char *channel,
            const char *reason);

    int (*on_nick)(void *arg, const char *prefix_old, const char *prefix_new);
    int (*on_invite)(void *arg, const char *prefix, const char *channel);

    int (*on_topic)(void *arg,
            const char *prefix,
            const char *channel,
            const char *topic_old,
            const char *topic_new);

    int (*on_mode_set)(void *arg,
            const char *prefix,
            const char *channel,
            char mode,
            const char *target);

    int (*on_mode_unset)(void *arg,
            const char *prefix,
            const char *channel,
            char mode,
            const char *target);

    int (*on_modes)(void *arg,
            const char *prefix,
            const char *channel,
            const char *modes);

    /*
     * Read only callbacks for aux. events.
     */
    int (*on_idle)(void *arg, time_t lastidle);

    int (*on_connect)(void *arg);
    int (*on_disconnect)(void *arg);
};

struct irc_session
{
    int fd;

    time_t start;
    time_t session_start;

    char hostname[HOSTNAME_MAX];
    uint16_t portno;

    int kill;

    char buffer[BUFFER_MAX];
    size_t bufuse;

    struct log_context log;

    char nick[NICK_MAX];
    char user[USER_MAX];
    char real[REAL_MAX];

    struct list *channels;
    struct list *capabilities;

    struct irc_callbacks cb;
};

/*
 * Util
 */
int sess_getln(struct irc_session *sess, char *linebuf, size_t linebufsiz);
int sess_sendmsg(struct irc_session *sess, const struct irc_message *msg);

int sess_connect(struct irc_session *sess);
int sess_disconnect(struct irc_session *sess);

/*
 * Main loop
 */
int sess_main(struct irc_session *sess);

/*
 * Logic
 */
int sess_handle_message(struct irc_session *sess, struct irc_message *msg);
int sess_handle_mode_change(
        struct irc_session *sess,
        const char *prefix,
        const char *chan,
        const char *modestr,
        char args[][IRC_PARAM_MAX],
        size_t argstart);

#endif /* defined _SESSION_H_ */