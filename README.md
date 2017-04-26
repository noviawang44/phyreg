# phyreg

This is a utility to make it easy to interact with the PHY(s) attached to the MDIO bus from usermode under linux.

It is intended for debuging PHY issues, and experiementing with register settings. 

It therefore must be run with rights to  `/dev/mem`  get direct accesss to the MDIO bus. 

## Exmaple uses

### Show all attached PHYs (bnased on reports from the MDIO subsystem)

When run without args, it will scan all attached and linked PHYs on the bus...

Here is a single PHY at address 0 and it currently has an active link...
```
phyreg
ALIVE ADDRESSES:0000-0000-0000-0001
LINK  ADDRESSES:0000-0000-0000-0001
```

Here is a single PHY at address 2 and it currently does not have an active link...
```
phyreg
ALIVE ADDRESSES:0000-0000-0000-0001
LINK  ADDRESSES:0000-0000-0000-0001
```

(sorry, I only have one PHY on my machine, so can't show an example of two PHY attached, but there would be '1's for each PHY)

### Read a register from an attached PHY...

Here we are reading register 27 from PHY at address 0...

```
phyreg 0 27
PHY=00 REG=27 : IDLE READ  ACK 1010-0000-0000-1010
```

The output is in binary, seporated into nibbles to make it easier to read. Bit 15 on the left, bit 0 on the right.

### Write a register on an attached PHY....

 Here wer are writing the binary value 1000-0000-0000-0000 (which is 0x8000) to register 0 on PHY at address 0...

```
phyreg 0 0 1000-0000-0000-0000
PHY=00 REG=00 : IDLE WRITE ACK 1000-0000-0000-0000 (DATA 1000-0000-0000-0000)
```
The first binary string is the value read back from the register after the write completed, the second value is the data that was written (they are the same in this case). It is printed in this order so that all the reads line up vertically for multiplue commands to be easier to compare. 

The data can also be specified in hex like this...

```
phyreg 0 0 8000
PHY=00 REG=00 : IDLE WRITE ACK 1000-0000-0000-0000 (DATA 1000-0000-0000-0000)
```

This is exactly the same as the command above. 

-Note that this command happens to execute a soft reset on the attached PHY if implements the IEEE
802.3 (clause 22.2.4) management register set.-

### Dump all registers

You can also automatically dump all registers of an attached PHY but not putting a register on the command line...

```
phyreg 0
PHY=00 REG=00 : IDLE READ  ACK 0011-0001-0000-0000
PHY=00 REG=01 : IDLE READ  ACK 0111-1000-0010-1101
PHY=00 REG=02 : IDLE READ  ACK 0000-0000-0000-0111
PHY=00 REG=03 : IDLE READ  ACK 1100-0000-1111-0001
PHY=00 REG=04 : IDLE READ  ACK 0000-0001-1110-0001
PHY=00 REG=05 : IDLE READ  ACK 1100-1101-1110-0001
PHY=00 REG=06 : IDLE READ  ACK 0000-0000-0000-1011
PHY=00 REG=07 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=08 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=09 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=10 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=11 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=12 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=13 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=14 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=15 : IDLE READ  ACK 0000-0000-0000-0000
PHY=00 REG=16 : IDLE READ  ACK 0000-0000-0100-0000
PHY=00 REG=17 : IDLE READ  ACK 0010-0000-0000-0010
PHY=00 REG=18 : IDLE READ  ACK 0000-0000-1110-0000
PHY=00 REG=19 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=20 : IDLE READ  ACK 0000-0000-0000-0000
PHY=00 REG=21 : IDLE READ  ACK 0000-0000-0000-0000
PHY=00 REG=22 : IDLE READ  ACK 0000-0000-0000-0000
PHY=00 REG=23 : IDLE READ  ACK 0000-0000-0000-0000
PHY=00 REG=24 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=25 : IDLE READ  ACK 1111-1111-1111-1111
PHY=00 REG=26 : IDLE READ  ACK 0000-0000-0000-0000
PHY=00 REG=27 : IDLE READ  ACK 1010-0000-0000-1010
PHY=00 REG=28 : IDLE READ  ACK 0000-0000-0000-0000
PHY=00 REG=29 : IDLE READ  ACK 0000-0000-1100-1000
PHY=00 REG=30 : IDLE READ  ACK 0000-0000-0000-0000
PHY=00 REG=31 : IDLE READ  ACK 0001-0000-0101-1000
```

This is just a short cut for manually reading each register one at a time, and it hand for doing a diff to see which registers changed. 

## Install

```
git clone https://github.com/bigjosh/phyreg
cd phyreg
make
make install
```

## FAQ 

Q: Doesn't the driver keep reseting the values while you are playing with them?

A: The driver seems to only know about the standard bottom 7 registers, so will not mess with stuff you are doing to others. To be safe, I usually will move the PHY to a different address before I start messing with it so that linux can not find it (the driver only looks for the address once at startup). 

Q: What does `IDLE READ ACK` mean?

A: These are status messages printed as the tool runs the sequence to access the register.

It prints `IDLE` when it finds the MDIO bus idle, or else it will print `WAIT` while it wait for it to be idle. 
It prints `READ` when it issues the read command (`GO` in the MDIO lingo).
It prints `ACK ` when it seen the `ACK` bit set showing that the read completed and the data is ready. 

It usually prints these so fast that you can't see any delay between them. 

If it gets stuck in any of these states then either there is something wrong with the bus, the PHY, or you are trying to talk to a PHY that doesn't exist (or at least respond). 

## Background

This code was orginally based on the `devmem2` utility, but has almost none of that code in it anymore. There is an included fork of `devemem2` that fixes some of the bugs in the orginal. 

I wrote this utility to help debug this bear of an issue on the BeagleBone that causes it to power up without network connectivity sometimes...

https://groups.google.com/forum/#!topic/beagleboard/9mctrG26Mc8%5B1-25%5D

As of 4/26/17, this problem remains unsolved. If you are an expert in the LAN8710A PHY and want to help, many people (including me!) would be forever greateful. This is a tough problem. 
