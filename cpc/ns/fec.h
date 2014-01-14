//
//  fec.h
//  funcfec
//
//  Created by Yang YanQing on 12/12/13.
//  Copyright (c) 2013 Yang YanQing. All rights reserved.
//
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

/* Niter is the number of messages sent. */

#define Niter    1

/* Lfield is the log of the length of the field. */

#define Lfield   10

/* Nsegs is the number of segments in packet. */
/* Length of packet in bytes is 4*Nsegs*Lfield */

#define Nsegs    5

/* Mpackets is the number of message packets */

#define Mpackets 1

/* Rpackets is the number of redundant packets */

#define Rpackets 1

typedef unsigned int UNSIGNED;

class fec{
public:
    fec();
    virtual ~fec();
    //input size默认为50   output size为102
    unsigned int * encode(unsigned int * output , unsigned int * input, int size);
    //input size default 102    output size default 50
    unsigned int * decode(unsigned int * output , unsigned int * input);
    void set_msg();
    void set_msg(UNSIGNED *message_input, int size);
    

//private:
    UNSIGNED COLBIT, BIT[15];
    int iter, Nrec, TNrec, seed, return_code;
    

    UNSIGNED *ExptoFE, *FEtoExp;
    UNSIGNED *message, *rec_message;
    UNSIGNED *packets, *rec_packets;
    
    
    void prepareWork();
    void init_field();
    
    enum { Npackets = Mpackets + Rpackets,
        Mseglen = Mpackets * Lfield,
        Plen = Nsegs * Lfield,
        Plentot = Plen + 1,
        Mlen = Plen * Mpackets,
        Elen = Plen * (Mpackets + Rpackets),
        TableLength = 1 << Lfield,
        SMultField = TableLength - 1 };

};
