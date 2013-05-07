#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/netdevice.h>

#include "cpc_rt.h"

#define list_is_first(rt) (&rt->l == rt_head.next)

#define RT_MAX_LEN 1024

static unsigned int rt_len;
static rwlock_t rt_lock = RW_LOCK_UNLOCKED;
static LIST_HEAD(rt_head);

static struct timer_list rt_timer;

static void cpc_rt_timeout(unsigned long data);

static inline void
__cpc_rt_set_next_timeout(void)
{
	struct rt_entry *rt;

	if (list_empty(&rt_head))
		return;

	rt = (struct rt_entry *)rt_head.next;

	if (timer_pending(&rt_timer))
		mod_timer(&rt_timer, rt->expires);
	else  {
		rt_timer.function = cpc_rt_timeout;
		rt_timer.expires = rt->expires;
		rt_timer.data = 0;
		add_timer(&rt_timer);
	}
}

static void
cpc_rt_timeout(unsigned long data)
{
	struct list_head *pos, *tmp;
	int time = jiffies;

	write_lock_bh(&rt_lock);

	list_for_each_safe(pos, tmp, &rt_head) {
		struct rt_entry *rt = (struct rt_entry *)pos;

		if (rt->expires > time)
			break;

		list_del(&rt->l);
		rt_len--;

		/*
		 * TODO: flush queue packets
		 */

		/*
		 * TODO: notify rt timeout to userspace
		 */
	}

	__cpc_rt_set_next_timeout();

	write_unlock_bh(&rt_lock);
}

static inline void
__cpc_rt_flush(void)
{
	struct list_head *pos, *tmp;

	list_for_each_safe(pos, tmp, &rt_head) {
		struct rt_entry *rt = (struct rt_entry *)pos;

		list_del(&rt->l);
		rt_len--;
		kfree(rt);
	}
}

static inline int
__cpc_rt_add(struct rt_entry *rt)
{
	if (rt_len > RT_MAX_LEN) {
		printk(KERN_WARNING "CPC: Max rt list len reached\n");
		return -ENOSPC;
	}

	if (list_empty(&rt_head))
		list_add(&rt->l, &rt_head);
	else {
		struct list_head *pos;

		list_for_each(pos, &rt_head) {
			struct rt_entry *curr = (struct rt_entry *)pos;

			if (curr->expires > rt->expires)
				break;
		}

		list_add(&rt->l, pos->prev);
	}

	return 1;
}

static inline struct rt_entry *
__cpc_rt_find(__u32 daddr)
{
	struct list_head *pos;

	list_for_each(pos, &rt_head) {
		struct rt_entry *rt = (struct rt_entry *)pos;

		if (rt->daddr == daddr)
			return rt;
	}

	return NULL;
}

static inline int
__cpc_rt_del(struct rt_entry *rt)
{
	if (rt == NULL)
		return 0;

	if (list_is_first(rt)) {
		list_del(&rt->l);

		if (!list_empty(&rt_head)) {
			struct rt_entry *next = (struct rt_entry *)rt_head.next;

			mod_timer(&rt_timer, next->expires);
		}
	} else
		list_del(&rt->l);

	rt_len--;

	return 1;
}

int
cpc_rt_del(__u32 daddr)
{
	int res;
	struct rt_entry *rt;

	write_lock_bh(&rt_lock);

	rt = __cpc_rt_find(daddr);

	if (rt == NULL) {
		res = 0;
		goto unlock;
	}

	res = __cpc_rt_del(rt);

	if (res)
		kfree(rt);

unlock:
	write_unlock_bh(&rt_lock);
	return res;
}

int
cpc_rt_get(__u32 daddr, struct rt_entry *rt)
{
	struct rt_entry *tmp = NULL;
	int res = 0;

	read_lock_bh(&rt_lock);
	tmp = __cpc_rt_find(daddr);

	if (tmp) {
		res = 1;
		if (rt)
			memcpy(rt, tmp, sizeof(struct rt_entry));
	}

	read_unlock_bh(&rt_lock);
	return res;
}

int cpc_rt_add(__u32 daddr, __u32 nhop, unsigned long time, int ifindex)
{
	struct rt_entry *rt;
	int status = 0;

	if (cpc_rt_get(daddr, NULL))
		return 0;

	rt = kmalloc(sizeof(struct rt_entry), GFP_ATOMIC);
	if (rt == NULL) {
		printk(KERN_ERR "CPC: Out of memory in cpc_rt\n");
		return -ENOMEM;
	}

	rt->daddr = daddr;
	rt->nhop = nhop;
	rt->ifindex = ifindex;
	rt->expires = jiffies + (time * HZ) / 1000;

	write_lock_bh(&rt_lock);

	status = __cpc_rt_add(rt);
	if (status)
		rt_len++;

	if (status && list_is_first(rt)) {
		if (timer_pending(&rt_timer))
			mod_timer(&rt_timer, rt->expires);
		else {
			rt_timer.function = cpc_rt_timeout;
			rt_timer.expires = rt->expires;
			rt_timer.data = 0;
			add_timer(&rt_timer);
		}
	}

	write_unlock_bh(&rt_lock);

	if (status < 0)
		kfree(rt);

	return status;
}

int
cpc_rt_update(__u32 daddr, __u32 nhop, unsigned long time, int ifindex)
{
	int ret = 0;
	struct rt_entry *rt;

	write_lock_bh(&rt_lock);

	rt = __cpc_rt_find(daddr);
	if (rt == NULL) {
		ret = -1;
		goto unlock;
	}

	rt->nhop = nhop;
	rt->ifindex = ifindex;
	rt->expires = jiffies + (time * HZ) / 1000;

	list_del(&rt->l);
	__cpc_rt_add(rt);

	__cpc_rt_set_next_timeout();

unlock:
	write_unlock_bh(&rt_lock);
	return ret;
}

void
cpc_rt_flush(void)
{
	if (timer_pending(&rt_timer))
		del_timer(&rt_timer);

	write_lock_bh(&rt_lock);
	__cpc_rt_flush();
	write_unlock_bh(&rt_lock);
}

void
cpc_rt_init(void)
{
	rt_len = 0;
	init_timer(&rt_timer);
}

void
cpc_rt_fini(void)
{
	cpc_rt_flush();
}
