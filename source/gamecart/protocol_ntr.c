#include "protocol_ntr.h"

#include "misc.h"

void NTR_SendCommand( u32 command[2], u32 pageSize, u32 latency, void * buffer )
{
    REG_NTRCARDMCNT = NTRCARD_CR1_ENABLE;

    for( u32 i=0; i<2; ++i )
    {
        REG_NTRCARDCMD[i*4+0] = command[i]>>24;
        REG_NTRCARDCMD[i*4+1] = command[i]>>16;
        REG_NTRCARDCMD[i*4+2] = command[i]>>8;
        REG_NTRCARDCMD[i*4+3] = command[i]>>0;
    }

    pageSize -= pageSize & 3; // align to 4 byte
    u32 pageParam = NTRCARD_PAGESIZE_4K;
    u32 transferLength = 4096;
    // make zero read and 4 byte read a little special for timing optimization(and 512 too)
    if( 0 == pageSize ) {
        transferLength = 0;
        pageParam = NTRCARD_PAGESIZE_0;
    } else if( 4 == pageSize ) {
        transferLength = 4;
        pageParam = NTRCARD_PAGESIZE_4;
    } else if( 512 == pageSize ) {
        transferLength = 512;
        pageParam = NTRCARD_PAGESIZE_512;
    } else if( 8192 == pageSize ) {
        transferLength = 8192;
        pageParam = NTRCARD_PAGESIZE_8K;
    }

    // go
    REG_NTRCARDROMCNT = 0x10000000;
    REG_NTRCARDROMCNT = KEY_PARAM | NTRCARD_ACTIVATE | NTRCARD_nRESET | pageParam | latency;

    u8 * pbuf = (u8 *)buffer;
    u32 * pbuf32 = (u32 * )buffer;
    bool useBuf = ( NULL != pbuf );
    bool useBuf32 = (useBuf && (0 == (3 & ((u32)buffer))));

    u32 count = 0;
    u32 cardCtrl = REG_NTRCARDROMCNT;

    if(useBuf32)
    {
        while( (cardCtrl & NTRCARD_BUSY) && count < pageSize)
        {
            cardCtrl = REG_NTRCARDROMCNT;
            if( cardCtrl & NTRCARD_DATA_READY  ) {
                u32 data = REG_NTRCARDFIFO;
                *pbuf32++ = data;
                count += 4;
            }
        }
    }
    else if(useBuf)
    {
        while( (cardCtrl & NTRCARD_BUSY) && count < pageSize)
        {
            cardCtrl = REG_NTRCARDROMCNT;
            if( cardCtrl & NTRCARD_DATA_READY  ) {
                u32 data = REG_NTRCARDFIFO;
                pbuf[0] = (unsigned char) (data >>  0);
                pbuf[1] = (unsigned char) (data >>  8);
                pbuf[2] = (unsigned char) (data >> 16);
                pbuf[3] = (unsigned char) (data >> 24);
                pbuf += sizeof (unsigned int);
                count += 4;
            }
        }
    }
    else
    {
        while( (cardCtrl & NTRCARD_BUSY) && count < pageSize)
        {
            cardCtrl = REG_NTRCARDROMCNT;
            if( cardCtrl & NTRCARD_DATA_READY  ) {
                u32 data = REG_NTRCARDFIFO;
                (void)data;
                count += 4;
            }
        }
    }

    // if read is not finished, ds will not pull ROM CS to high, we pull it high manually
    if( count != transferLength ) {
        // MUST wait for next data ready,
        // if ds pull ROM CS to high during 4 byte data transfer, something will mess up
        // so we have to wait next data ready
        do { cardCtrl = REG_NTRCARDROMCNT; } while(!(cardCtrl & NTRCARD_DATA_READY));
        // and this tiny delay is necessary
        //ioAK2Delay(33);
        // pull ROM CS high
        REG_NTRCARDROMCNT = 0x10000000;
        REG_NTRCARDROMCNT = KEY_PARAM | NTRCARD_ACTIVATE | NTRCARD_nRESET/* | 0 | 0x0000*/;
    }
    // wait rom cs high
    do { cardCtrl = REG_NTRCARDROMCNT; } while( cardCtrl & NTRCARD_BUSY );
    //lastCmd[0] = command[0];lastCmd[1] = command[1];
}