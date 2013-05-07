#include <stdlib.h>

#include "list.h"

static int listelm_detach(list_t * prev, list_t * next)
{
	next->prev = prev;
	prev->next = next;

	return LIST_SUCCESS;
}

static int listelm_add(list_t * le, list_t * prev, list_t * next)
{
	prev->next = le;
	le->prev = prev;
	le->next = next;
	next->prev = le;

	return LIST_SUCCESS;
}

int list_add(list_t * head, list_t * le)
{

	if (!head || !le)
		return LIST_NULL;

	listelm_add(le, head, head->next);

	return LIST_SUCCESS;
}

int list_add_tail(list_t * head, list_t * le)
{

	if (!head || !le)
		return LIST_NULL;

	listelm_add(le, head->prev, head);

	return LIST_SUCCESS;
}

int list_detach(list_t * le)
{
	if (!le)
		return LIST_NULL;

	listelm_detach(le->prev, le->next);

	le->next = le->prev = NULL;

	return LIST_SUCCESS;
}
