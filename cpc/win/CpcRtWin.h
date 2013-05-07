#ifndef CPC_RT_WIN_H
#define CPC_RT_WIN_H

void CpcRtAddWin(struct in_addr dst, struct in_addr next);
void CpcRtRemoveWin(struct in_addr dst);
void CpcRtUpdateWin(struct in_addr dst, struct in_addr next);
struct in_addr *CpcRtFindWin(struct in_addr dst);

#endif /* CPC_RT_H */