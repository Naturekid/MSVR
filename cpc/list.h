#ifndef LIST_H
#define LIST_H

/* Simple linked list inspired from the Linux kernel list implementation */
typedef struct list_t {
	struct list_t *prev, *next;
} list_t;

#define LIST_NULL -1
#define LIST_SUCCESS 1

#define LIST(name) list_t name = { &(name), &(name) }

#define INIT_LIST_HEAD(h) do { \
	(h)->next = (h); (h)->prev = (h); \
} while (0)

#define INIT_LIST_ELM(le) do { \
	(le)->next = NULL; (le)->prev = NULL; \
} while (0)

int list_detach(list_t * le);
int list_add_tail(list_t * head, list_t * le);
int list_add(list_t * head, list_t * le);

#define list_foreach(curr, head) \
	for (curr = (head)->next; curr != (head); curr = curr->next)

#define list_foreach_safe(pos, tmp, head) \
	for (pos = (head)->next, tmp = pos->next; pos != (head); \
			pos = tmp, tmp = pos->next)

#define list_empty(head) ((head) == (head)->next)

#define list_first(head) ((head)->next)

#define list_unattached(le) ((le)->next == NULL && (le)->prev == NULL)

#endif /* LIST */
