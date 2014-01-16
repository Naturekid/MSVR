//
//  fec.h
//  funcfec
//
//  Created by Yang YanQing on 12/12/13.
//  Copyright (c) 2013 Yang YanQing. All rights reserved.
//
//	useless here , might be moving to PBC
typedef unsigned int   UNSIGNED;


#include <iostream>


class Fec{
public:
    Fec();
    ~Fec();
    void Decode();
    void Prepare();
    void Init_field( UNSIGNED* a, UNSIGNED* b , UNSIGNED *c , UNSIGNED *d);
    void Get_msg(UNSIGNED *a);
    void Encode(UNSIGNED a,UNSIGNED *b ,UNSIGNED *c ,UNSIGNED *d ,UNSIGNED *e ,UNSIGNED *f);
    void Lose_Packets(UNSIGNED *a, UNSIGNED *b, int *c);
    void Decode(UNSIGNED a, UNSIGNED *b, UNSIGNED *c,UNSIGNED *d, UNSIGNED *e, int *f,UNSIGNED *g);




    /* Niter is the number of messages sent. */

#define Niter    1

    /* Lfield is the log of the length of the field. */

#define Lfield   10

    /* Nsegs is the number of segments in packet. */
    /* Length of packet in bytes is 4*Nsegs*Lfield */

#define Nsegs    5

    /* Mpackets is the number of message packets */

#define Mpackets 3

    /* Rpackets is the number of redundant packets */

#define Rpackets 2

    /* IMPORTANT: The max of Mpackets and Rpackets is at most 2^{Lfield-1} */
    /* Lfield must be set large enough to make this true else the encoding
     and decoding won't work */

    /* Npackets is the total number of packets sent.
     Mseglen is the message segment length.
     Plen is the packet length in words excluding the overhead
     for storing the index.
     Plentot is the packet length in words including the overhead
     for storing the index.
     Mlen is the length of the message in words
     Elen is the length of the encoding in words
     TableLength is 2^{Lfield}
     SMultField is the size of the multiplicative field (2^{Lfield}-1)
     */

    /* THIS INFORMATION DEPENDS ON THE LENGTH OF THE FIELD (set above) */

    enum { Npackets = Mpackets + Rpackets,
        Mseglen = Mpackets * Lfield,
        Plen = Nsegs * Lfield,
        Plentot = Plen + 1,
        Mlen = Plen * Mpackets,
        Elen = Plen * (Mpackets + Rpackets),
        TableLength = 1 << Lfield,
        SMultField = TableLength - 1 };

    UNSIGNED *ExptoFE, *FEtoExp;
    UNSIGNED *message, *rec_message;
    UNSIGNED *packets, *rec_packets;
    UNSIGNED COLBIT, BIT[15];

    int iter, Nrec, TNrec, seed, return_code;

};
