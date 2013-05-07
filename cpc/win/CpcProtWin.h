#ifndef CPC_PROT_WIN_H
#define CPC_PROT_WIN_H

void CpcProtInitWin(void);
void CpcProtDesWin(void);

void CpcProtSendWin(char *p, int n, struct in_addr dst);
void CpcProtSendWinWithData(char *p, int n, struct in_addr dst);

#endif // CPC_PROT_WIN_H