/*
 * phyreg.c: Simple program to read/write from/to LAN8710A PHY registers from an AM335x using MDIO
 *
 *  (c) 2017 josh.com
 *
 *  Based on:
 *  Copyright (C) 2000, Jan-Derk Bakker (jdb@lartmaker.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <string.h>

#define FATAL do { fprintf( stderr ,stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)


// based on...
// http://stackoverflow.com/a/3974138/3152071

//assumes little endian

void int2binstr(char *buffer, size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            *buffer = '0' + byte;
            buffer++;
        }
        *buffer = ' ';		// makes it easier to see
        buffer++;
    }
    *buffer = 0x00;	// null terminal string
}


#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

// Map an ARM base address to a pointer we can use via devmem
// Must be on a page boundary

unsigned *map_base( unsigned target )  {
	
	if ( target != (target & ~MAP_MASK )  ) {
		fprintf( stderr , "Base address must be on boundary,.\r\n");
		return(NULL);
	}

    
    // Open the file descriptor
    
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    
    if (fd == -1 ) {
		fprintf( stderr , "Could not open /dev/mem (are you root?).\r\n");
		return(NULL);
    }

	// And map it

    /* Map one page */
    
    unsigned *map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target );
    
    if (map_base == (void *) -1) {
	    fprintf( stderr , "MMAP FAILED.\r\n");
	    close(fd);
	    return(NULL);
    }
    
    close(fd);		// Ok to close now http://stackoverflow.com/questions/17490033/do-i-need-to-keep-a-file-open-after-calling-mmap-on-it	        
    
    return(map_base);
	
}

void unmap_base( unsigned *map_base ) {
    if(munmap(map_base, MAP_SIZE) == -1) {
	    		fprintf( stderr , "unmapped failed!.\r\n");
    }
}

// Print bottom 16 bits of u

void printbits( unsigned u ) {
	

	int bit = 16;
	
	while (bit) {
		
		bit--;
		
		fprintf( stderr , (u & (1<<bit) ) ? "1" : "0");
		
		if (  ((bit % 4) == 0) && (bit>0) ) fprintf( stderr ,"-");		// make easier to read bybreaking into nibbles (but suppress trailing -)
		
	}	
	
}

#define MDIO_BASE_TARGET 0x4a101000 // ARM address of the MDIO controller

#define MDIO_ALIVE_OFFSET 	 	0x08  // PHY Alive Status Register
#define MDIO_LINK_OFFSET 	 	0x0c  // PHY Link Status Register

#define MDIO_USERACCESS0_OFFSET 	0x80  // MDIO User Access Register 0

#define MDIO_USERACCESS0_GO_BIT		(1<<31)	// Set to start a transaction, check to see if transaction in progress
#define MDIO_USERACCESS0_WRITE_BIT		(1<<30)	// Set to write to the PHY register
#define MDIO_USERACCESS0_ACK_BIT		(1<<29)	// "Acknowledge. This bit is set if the PHY acknowledged the read transaction."


// We need this becuase pointer arithmatic on unsigned * goes by 4's
#define OFFSET_PTR( base , offset ) ( base + (offset/sizeof( *base) )) 

void printscan( const char *s , unsigned *address ) {
	
	//fprintf( stderr ,"add=%x\n",address);
	
	unsigned a = *address;		// read the array of bits fomr the MDIO controller
	
	fprintf( stderr , s );
	printbits( a );
	fprintf( stderr ,"\n");
	
}

unsigned short parsebin( const char *s ) {
	
	unsigned short x=0;
	
	while (*s) {		
		if (isdigit(*s)){
			x*=2;			
			x+= (*s -'0');
		}
		s++;
	}
	
	return(x);
	
}

// Will write writeval if flag is set
// returns register contents (before write if write specified)

int accessreg( volatile unsigned *useraccessaddress , unsigned short phy_address, unsigned short reg , unsigned char writeflag , unsigned writeval ) {
	fprintf( stderr ,"PHY=%2.02d REG=%2.02d : " , phy_address , reg );
	
	if ( *useraccessaddress & MDIO_USERACCESS0_GO_BIT ) {
		fprintf( stderr ,"WAIT ");
		while (*useraccessaddress & MDIO_USERACCESS0_GO_BIT);
	} else {
		fprintf( stderr ,"IDLE ");
	}

	if (writeflag) {
		fprintf( stderr , "WRITE ");
		*useraccessaddress = MDIO_USERACCESS0_GO_BIT | MDIO_USERACCESS0_WRITE_BIT | (reg << 21) | (phy_address << 16) | writeval;	// Send the  command as defined by 14.5.10.11 in TRM		
	} else {
		fprintf( stderr , "READ  ");		
		*useraccessaddress = MDIO_USERACCESS0_GO_BIT |                              (reg << 21) | (phy_address << 16);			// Send the actual read command as defined by 14.5.10.11 in TRM
	}
	
	// Now wait for the MDIO transaction to complete

	while (*useraccessaddress & MDIO_USERACCESS0_GO_BIT);
	
	if ( *useraccessaddress & MDIO_USERACCESS0_ACK_BIT ) {
		fprintf( stderr ,"ACK ");
		while (*useraccessaddress & MDIO_USERACCESS0_GO_BIT);
	} else {
		fprintf( stderr ,"NAK ");
	}
	
	int data = 	 *useraccessaddress ;			// THe bottom 16 bits are the read value
	
	printbits( data );
	
	if (writeflag) {
		fprintf( stderr , " (WROTE %d) " , writeval );
	}
	fprintf( stderr ,"\n");
	
	return data; 
	
}

int readreg( unsigned *useraccessaddress , unsigned short phy_address, unsigned short reg ) {
	return accessreg( useraccessaddress , phy_address , reg , 0, 0 );
}

int  writereg( unsigned *useraccessaddress , unsigned short phy_address, unsigned short reg , unsigned short data ) {
	return accessreg( useraccessaddress , phy_address , reg , 1, data );
}

int main(int argc, char **argv) {
	
    
    if(argc ==2 && !strcmp( argv[1] , "-?" )) {
        fprintf( stderr, "\nUsage:phyreg [TEST reg bit] | [address [reg [  data ] ]]\n"
			"To test a bit in a register ...\n"
			"   reg     : register 0-31\n"
			"   bit     : bit 0-31\n"			
			"Automatically selects first PHY address found\n"
			"Prints 0 or 1 to stdout\n"
			"\n"
			"To debug phy addresses & registers\n"			
            "   address : phy address to act upon, or scan addresses if none specified\n"
            "   reg     : phy register to act on  (if not spoecified, 0-31 will be dumped)\n"
            "   data    : optional 16 bits of data to be written to reg in format xxxx or bbbb-bbbb-bbbb-bbbb \n\n"
            );
        exit(1);
    }
    
	//fprintf( stderr ,"mapping\n");
	unsigned *mdiobase = map_base( MDIO_BASE_TARGET );
	
	if (!mdiobase) {
		fprintf( stderr ,"mmap failed. Check stderr for reeason.\n");
		return(1);
	}
	
	//fprintf( stderr ,"mapped\n");

	if (argc<2) {		// no command line args, go scan mode
    
		printscan( "ALIVE ADDRESSES:" , OFFSET_PTR( mdiobase , MDIO_ALIVE_OFFSET ) );
		printscan( "LINK  ADDRESSES:" , OFFSET_PTR( mdiobase , MDIO_LINK_OFFSET  ) );
		fprintf( stderr ,"Try -? for usage.\n");
		
	} else {
		
		if ( ( argc == 4) && !strcasecmp( argv[1] , "TEST" ) ) {
			
			// Find address of first phy 
			
			unsigned alivebits =  *OFFSET_PTR( mdiobase , MDIO_ALIVE_OFFSET );
			
			fprintf( stderr , "Alive bits:");			
			printbits( alivebits);
			fprintf( stderr , "\n");			
			
			int phy_address=0; 
			
			while ( phy_address < 32 && (( alivebits & (1<<phy_address ))== 0 ) ) {				
				phy_address++;
			}
			
			
			
			if (phy_address==32) {
				fprintf( stderr , "No PHY at all found!\n");
				fprintf( stdout ,"0\n");
			} else {
				
				fprintf( stderr , "First PHY found at address %d.\n" , phy_address);
			
				int phy_reg = atoi( argv[2] );
				int phy_bit = atoi( argv[3] );				
				
				if ( readreg( OFFSET_PTR( mdiobase , MDIO_USERACCESS0_OFFSET) , phy_address , phy_reg ) & (1<<phy_bit) ) {
					fprintf( stdout ,"1");
				} else {
					fprintf( stdout ,"0\n");
				}
			}
			
		} else {
			
			int phy_address = atoi( argv[1] );
			
			if (argc<3) {		// No reg specified, dump all
			
				int r;
				
				for(r=0; r<32; r++ ) {

					readreg( OFFSET_PTR( mdiobase , MDIO_USERACCESS0_OFFSET) , phy_address , r )  ;
					
				}
			} else {
				
				int phy_reg = atoi( argv[2] );
				
				if (argc < 4 ) {	// Read specified reg
						
					readreg( OFFSET_PTR( mdiobase , MDIO_USERACCESS0_OFFSET) , phy_address , phy_reg ) ;
					
				} else {			// Write
				
					const char *datastr = argv[3];
							
					int data;
					
					if (strlen(datastr) == 4) {	// Parse as hex
						data= (short unsigned) strtol( datastr , NULL , 16); 
					} else {	// parse as bin
						data=parsebin( datastr );
					}
				
					writereg( OFFSET_PTR( mdiobase , MDIO_USERACCESS0_OFFSET) , phy_address , phy_reg , data );
					
				}
				
			}
		}
    }

	unmap_base( mdiobase );

    return 0;
}
