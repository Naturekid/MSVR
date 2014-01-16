//
//  fec.cpp
//  funcfec
//
//  Created by Yang YanQing on 12/12/13.
//  Copyright (c) 2013 Yang YanQing. All rights reserved.
//

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fec.h"


Fec::Fec(){
    this->Prepare();
    this->Init_field(&COLBIT, BIT, ExptoFE, FEtoExp);
}


Fec::~Fec(){

}







void Fec::Prepare(){
    ExptoFE = (UNSIGNED *)calloc(TableLength + Lfield, sizeof(UNSIGNED));
    if (!(ExptoFE)) {
        printf("\ndriver: ExptoFE malloc failed\n"); exit(434);
    }

    FEtoExp = (UNSIGNED *)calloc(TableLength, sizeof(UNSIGNED));
    if (!(FEtoExp)) {
        printf("\ndriver: FEtoExp malloc failed\n"); exit(434);
    }

    rec_message = (UNSIGNED *)calloc(Mlen, sizeof(UNSIGNED));
    if (!(rec_message)) {
        printf("\ndriver: rec_message malloc failed\n"); exit(434);
    }

    message = (UNSIGNED *)calloc(Mlen, sizeof(UNSIGNED));
    printf("Message len %d * unsigned int\n",Mlen);
    if (!(message)) {
        printf("\ndriver: message malloc failed\n"); exit(434);
    }

    packets = (UNSIGNED *)calloc(Npackets * Plentot, sizeof(UNSIGNED));
    printf("packets len %d * unsigned int\n",Npackets * Plentot);
    if (!(packets)) {
        printf("\ndriver: packets malloc failed\n"); exit(434);
    }

    rec_packets = (UNSIGNED *)calloc(Npackets * Plentot, sizeof(UNSIGNED));
    if (!(rec_packets)) {
        printf("\ndriver: rec_packets malloc failed\n"); exit(434);
    }
}

void Fec::Init_field(UNSIGNED *pCOLBIT, UNSIGNED *BIT, UNSIGNED *ExptoFE,UNSIGNED *FEtoExp)
{
    /* POLYMASK is the irreducible polynomial */
    /* CARRYMASK is used to see when there is a carry in the polynomial
     and when it should be XOR'd with POLYMASK */

    int i ;
    static UNSIGNED CARRYMASK ;
    static UNSIGNED POLYMASK[16] =
    {0x0,     0x3,   0x7,   0xB,   0x13,   0x25,  0x43,    0x83,
        0x11D, 0x211, 0x409, 0x805, 0x1053, 0x201B, 0x402B, 0x8003} ;

    /* Lfield is the length of the field (This can from the table above
     be between 1 and 15, but because of restrictions in the driver.c
     program this value currently can be at most 10).
     SMultField = TableLength - 1 is the number of elements in the multiplicative
     group of the field.  COLBIT is used to make sure rows and columns have
     distinct field elements associated with them
     The BIT array is used to mask out single bits in equations: bit
     ExptoFE is the table that goes from the exponent to the finite field element
     FEtoExp is the table that goes from the finite field element to the exponent */

    BIT[0] = 0x1 ;
    for (i=1; i < Lfield ; i++) BIT[i] = BIT[i-1] << 1 ;
    *pCOLBIT = BIT[Lfield-1] ;
    CARRYMASK = *pCOLBIT << 1 ;

    ExptoFE[0] = 0x1 ;
    for (i=1; i < SMultField + Lfield - 1 ; i++)
    {
        ExptoFE[i] = ExptoFE[i-1] << 1 ;
        if (ExptoFE[i]&CARRYMASK)
            ExptoFE[i] ^= POLYMASK[Lfield] ;
    } ;

    FEtoExp[0] = -1 ;
    for (i=0; i < SMultField ; i++)
        FEtoExp[ExptoFE[i]] = i ;
}


void Fec::Get_msg(UNSIGNED *message)
{
    //int i;
    char *x = (char *)message;
    char hello[14]="Hello World!\n";
    //for (i=0; i < Mlen; i++) message[i] = i ;
    strcpy(x, hello);
    //memcpy(x, hello, sizeof(100));
    //for (i=0; i < 4*Mlen; i++)
    //    x[i] = "abcdefghijklmnopwrstuvwxyz"[rand()%26] ;
}

void Fec::Encode(UNSIGNED COLBIT, UNSIGNED *BIT, UNSIGNED *ExptoFE,
            UNSIGNED *FEtoExp, UNSIGNED *packets, UNSIGNED *message)
{
    int i,j,k,l,m, ind_seg,col_eqn,row_eqn,ind_eqn ;
    int row, col, ExpFE ;
    //struct timeval start_time, end_time ;

    /* Set the identifier in all the packets to be sent */

    for (i=0; i < Npackets ; i++)
        packets[i*Plentot] = i;

    /* Copy the message into the first Mpackets packets */


    k = 0 ;
    j = 0 ;
    for (i=0; i < Mpackets ; i++)//填充从0到200个 packet的位置，把Message从头到尾填入 packets
    {
        k ++ ;
        for (ind_eqn=0; ind_eqn < Lfield ; ind_eqn++)
        {
            for (ind_seg =0; ind_seg  < Nsegs ; ind_seg ++)
            {
                packets[k] = message[j] ;
                j++ ;
                k++ ;
            } ;
        } ;
    } ;

    /* Fill in values for remaining Rpackets packets */

    for (row=0; row < Rpackets ; row++)
    {

        /* Compute values of equations applied to message
         and fill into packet(row+Mpackets).  */

        /* First, zero out contents relevant portions of packet */

        j = (row+Mpackets)*Plentot ;
        for (i =1 ; i  < Plentot ; i ++)
            packets[j+i] = 0 ;

        /* Second, fill in contents relevant portions of packet */

        for (col=0 ; col < Mpackets ; col++)
        {
            m = col*Lfield * Nsegs ;
            ExpFE = (SMultField - FEtoExp[row^col^COLBIT]) % SMultField ;
            for (row_eqn=0 ; row_eqn < Lfield ; row_eqn++)
            {
                k = row_eqn * Nsegs ;
                for (col_eqn=0 ; col_eqn < Lfield ; col_eqn++)
                {
                    if (ExptoFE[ExpFE+row_eqn] & BIT[col_eqn])
                    {
                        l = col_eqn * Nsegs + m ;
                        for (ind_seg=0 ; ind_seg < Nsegs ; ind_seg++)
                        {
                            packets[j+1+ind_seg+k] ^= message[ind_seg+l] ;
                        } ;
                    } ;
                } ;
            } ;
        } ;
    } ;

#ifdef PRINT
    printf ("\n------------------------------------------------") ;
    printf ("\n encode: number of seconds is %f",
            (float)(end_time.tv_sec-start_time.tv_sec)+
            (float)(end_time.tv_usec-start_time.tv_usec)/1000000.0) ;
    printf ("\n------------------------------------------------\n\n") ;
    fflush(stdout) ;
#endif
}


void Fec::Lose_Packets(UNSIGNED *packets, UNSIGNED *rec_packets, int *pNrec)

{
    int i,j,k,m;

    *pNrec = 0 ;
    k = 0 ;
    m = 0 ;
    for (i=(Npackets-Mpackets)/2 ; i < (Npackets+Mpackets)/2 ; i++)
        //for (i=1 ; i < 400 ; i+= 2)

    {
        k = Plentot * i ;
        m = Plentot * (*pNrec) ;
        for (j=0; j < Plentot; j++)
            rec_packets[m+j] = packets[k+j] ;
        (*pNrec)++ ;
    } ;

}


void Fec::Decode(UNSIGNED COLBIT, UNSIGNED *BIT, UNSIGNED *ExptoFE,
       UNSIGNED *FEtoExp, UNSIGNED *rec_packets, int *pNrec,
       UNSIGNED *rec_message)
{
    int i,j,k,l,m, index, seg_ind ;
    int col_ind, row_ind, col_eqn, row_eqn ;
    int Nfirstrec, Nextra ;
    int *Rec_index ;
    int *Col_Ind, *Row_Ind, ExpFE ;
    UNSIGNED  *M ;
    UNSIGNED *C, *D, *E, *F ;
    //struct timeval start_time, end_time;

    Rec_index = (int *) calloc(Mpackets, sizeof(int));
    if (!(Rec_index)) {printf("\ndecode: Rec_index malloc failed\n"); exit(434); }

    Col_Ind = (int *) calloc(Mpackets, sizeof(int));
    if (!(Col_Ind)) {printf("\ndecode: Col_Ind malloc failed\n"); exit(434); }

    Row_Ind = (int *) calloc(Rpackets, sizeof(int));
    if (!(Row_Ind)) {printf("\ndecode: Row_Ind malloc failed\n"); exit(434); }

    C = (UNSIGNED *) calloc(Rpackets, sizeof(UNSIGNED));
    if (!(C)) {printf("\ndecode: C malloc failed\n"); exit(434); }

    D = (UNSIGNED *) calloc(Mpackets, sizeof(UNSIGNED));
    if (!(D)) {printf("\ndecode: D malloc failed\n"); exit(434); }

    E = (UNSIGNED *) calloc(Mpackets, sizeof(UNSIGNED));
    if (!(E)) {printf("\ndecode: E malloc failed\n"); exit(434); }

    F = (UNSIGNED *) calloc(Rpackets, sizeof(UNSIGNED));
    if (!(F)) {printf("\ndecode: F malloc failed\n"); exit(434); }

    M = (UNSIGNED *) calloc(Nsegs*Rpackets*Lfield, sizeof(UNSIGNED));
    if (!(M)) {printf("\ndecode: M malloc failed\n"); exit(434); }

    if (*pNrec < Mpackets)
    {
        printf("*** Need %d packets to recover message",Mpackets) ;
        printf(" but only %d packets received *** \n",*pNrec) ;
    } ;

    /* Initialize the received message */

    for (i=0; i < Mlen ; i++) rec_message[i] = 0 ;

    /* Move information from packets into received message.
     Fill in parts of received message that
     requires no processing and figure out how
     many of the redundant packets are needed.
     Nfirstrec is the number of packets received
     from among the first Mpackets that carry portions
     of the unprocessed original message.
     Rec_index is an array that indicates which parts
     of the message are received.  The pattern is
     the same within all Nsegs segments,  */


    Nfirstrec = 0 ;
    for (i=0; i < Mpackets; i++) Rec_index[i] = 0 ;
    m = 0 ;
    for (i=0; i < *pNrec ; i++)
    {
        index = rec_packets[m] ;
        if (index < Mpackets)
        {
            j = index * Plen ;
            Rec_index[index] = 1 ;
            for (row_eqn=0; row_eqn < Lfield ; row_eqn++)
            {
                k = row_eqn * Nsegs ;
                l = j + k ;
                for (seg_ind=0; seg_ind < Nsegs ; seg_ind++)
                    rec_message[seg_ind+l] = rec_packets[m+1+seg_ind+k] ;
            } ;
            Nfirstrec++ ;
        } ;
        m += Plentot ;
    } ;

    /* Nextra is the number of redundant packets that need to be processed */

    Nextra = Mpackets - Nfirstrec ;
#ifdef PRINT
    printf("Nfirstrec= %d, Nextra= %d \n",Nfirstrec,Nextra) ;
#endif

    /* Compute the indices of the missing words in the message */

    col_ind = 0 ;
    for (i=0; i < Mpackets ; i++)
    {
        if (Rec_index[i] == 0)
            Col_Ind[col_ind++] = i ;
    } ;

    /* Keep track of indices of extra packets in Row_Ind array
     and initialize M array from the received extra packets */

    row_ind = 0 ;
    m = 0 ;
    for (i=0; i < *pNrec ; i++)
    {
        if (rec_packets[m] >= Mpackets)
        {
            k = Nsegs*row_ind*Lfield ;
            Row_Ind[row_ind] = rec_packets[m] - Mpackets ;
            for (row_eqn=0 ; row_eqn < Lfield ; row_eqn++)
            {
                j = row_eqn*Nsegs ;
                for (seg_ind=0 ; seg_ind < Nsegs ; seg_ind++)
                {
                    M[k] = rec_packets[m+1+seg_ind+j] ;
                    k++ ;
                } ;
            } ;
            row_ind ++ ;
            if (row_ind >= Nextra) break ;
        } ;
        m += Plentot ;
    } ;

    /* Adjust M array according to the equations and the contents of rec_message */

    for (row_ind = 0 ; row_ind < Nextra ; row_ind++)
    {
        for (col_ind=0 ; col_ind < Mpackets ; col_ind++)
        {
            if (Rec_index[col_ind] == 1)
            {
                ExpFE = (SMultField - FEtoExp[Row_Ind[row_ind]^col_ind^COLBIT]) % SMultField ;
                for (row_eqn=0 ; row_eqn < Lfield ; row_eqn++)
                {
                    j = Nsegs*(row_eqn + row_ind*Lfield) ;
                    for (col_eqn=0 ; col_eqn < Lfield ; col_eqn++)
                    {
                        k = Nsegs*(col_eqn + col_ind*Lfield) ;
                        if (ExptoFE[ExpFE+row_eqn] & BIT[col_eqn])
                        {
                            for (seg_ind=0 ; seg_ind < Nsegs ; seg_ind++)
                            {
                                M[j+seg_ind] ^= rec_message[k+seg_ind] ;
                            } ;
                        } ;
                    } ;
                } ;
            } ;
        } ;
    } ;

    /* Compute the determinant of the matrix in the finite field
     and then compute the inverse matrix */

    for (row_ind = 0 ; row_ind < Nextra ; row_ind++)
    {
        for (col_ind = 0 ; col_ind < Nextra ; col_ind++)
        {
            if (col_ind != row_ind)
            {
                C[row_ind] += FEtoExp[Row_Ind[row_ind]^Row_Ind[col_ind]] ;
                D[col_ind] += FEtoExp[Col_Ind[row_ind]^Col_Ind[col_ind]] ;
            } ;
            E[row_ind] += FEtoExp[Row_Ind[row_ind]^Col_Ind[col_ind]^COLBIT] ;
            F[col_ind] += FEtoExp[Row_Ind[row_ind]^Col_Ind[col_ind]^COLBIT] ;
        } ;
    } ;

    /* Fill in the recovered information in the message from
     the inverted matrix and from M */

    for (row_ind = 0 ; row_ind < Nextra ; row_ind++)
    {
        for (col_ind = 0 ; col_ind < Nextra ; col_ind++)
        {
            ExpFE = E[col_ind] + F[row_ind] - C[col_ind] - D[row_ind]
            - FEtoExp[Row_Ind[col_ind]^Col_Ind[row_ind]^COLBIT] ;
            if (ExpFE < 0) ExpFE = SMultField-((-ExpFE) % SMultField) ;
            ExpFE = ExpFE % SMultField ;
            j = Col_Ind[row_ind] * Lfield * Nsegs ;
            for (row_eqn=0 ; row_eqn < Lfield ; row_eqn++)
            {
                k = row_eqn*Nsegs + j ;
                for (col_eqn=0 ; col_eqn < Lfield ; col_eqn++)
                {
                    l = Nsegs *(col_eqn + col_ind*Lfield) ;
                    if (ExptoFE[ExpFE+row_eqn] & BIT[col_eqn])
                    {
                        for (seg_ind=0 ; seg_ind < Nsegs ; seg_ind++)
                        {
                            rec_message[seg_ind+k] ^= M[l] ;
                            l++ ;
                        } ;
                    } ;
                } ;
            } ;
        } ;
    } ;

#ifdef PRINT
    printf ("\n------------------------------------------------") ;
    printf ("\n decode: number of seconds is %f",
            (float)(end_time.tv_sec-start_time.tv_sec)+
            (float)(end_time.tv_usec-start_time.tv_usec)/1000000.0) ;
    printf ("\n------------------------------------------------\n\n") ;
    fflush(stdout) ;
#endif

}
