#ifndef DEFS_H
#define DEFS_H

typedef void (*callback_func) (int);
extern int register_callback_func(int fd, callback_func func);

#endif /* DEFS_H */
