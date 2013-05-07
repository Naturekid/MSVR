#ifndef CPC_QUEUE_H
#define CPC_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

int cpc_queue_send_packets(struct in_addr dst);
int cpc_queue_drop_packets(struct in_addr dst);

#ifdef CPC_NS
int cpc_queue_init_ns(CpcAgent *agent);
int cpc_queue_des_ns();
#endif /* CPC_NS */

#ifdef CPC_LINUX
int cpc_queue_init_linux(void);
int cpc_queue_des_linux(void);
#endif /* CPC_LINUX */

#ifdef WIN32
int cpc_queue_init_win(void);
int cpc_queue_des_win(void);
#endif /* WIN32 */

#ifdef __cplusplus
}
#endif

#endif /* CPC_QUEUE_H */
