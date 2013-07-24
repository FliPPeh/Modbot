#ifndef _IRC_MODES_H_
#define _IRC_MODES_H_

#include "irc/irc.h"
#include "util/list.h"

struct irc_mode
{
    char mode;
    char arg[IRC_PARAM_MAX];
};

struct irc_user
{
    char prefix[IRC_PREFIX_MAX];
    char modes[IRC_FLAGS_MAX];
};

struct irc_channel
{
    char name[IRC_CHANNEL_MAX];
    time_t created;

    char topic[IRC_TOPIC_MAX];
    char topic_setter[IRC_PREFIX_MAX];
    time_t topic_set;

    struct list *users;
    struct list *modes;
};

/*
 * User comparison based on nick/host
 */
int irc_user_cmp(const char *a, const char *b);

/*
 * List management
 */
void irc_channel_free(void *data);
void irc_channel_copy(void *dst, const void *src);

/* Channel management */
int irc_channel_add(struct list **chanlist, const char *chan);
int irc_channel_del(struct list **chanlist, struct irc_channel *chan);
struct irc_channel *irc_channel_get(struct list *chanlist, const char *chan);

/* Channel user management */
int irc_channel_add_user(struct irc_channel *chan, const char *prefix);
int irc_channel_del_user(struct irc_channel *chan, struct irc_user *user);
struct irc_user *irc_channel_get_user(
        struct irc_channel *chan,
        const char *user);

/*
 * Modes
 */

/* Mode lists (+b, +e, ...) */
int irc_channel_add_mode(struct irc_channel *c, char mode, const char *arg);
int irc_channel_del_mode(struct irc_channel *c, char mode, const char *arg);

/* Single flags with or without arguments */
int irc_channel_set_mode(struct irc_channel *c, char mode, const char *arg);
int irc_channel_unset_mode(struct irc_channel *c, char mode);

/* User flags */
int irc_channel_user_set_mode(struct irc_user *u, char mode);
int irc_channel_user_unset_mode(struct irc_user *u, char mode);


#endif /* defined _IRC_MODES_H_ */