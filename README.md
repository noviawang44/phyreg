# phyreg

This is a utility to make it easy to interact with the PHY(s) attached to the MDIO bus from usermode under linux.

It is intended for debuging PHY issues, and experiementing with register settings. 

It therefore must be run with rights to  `/dev/mem`  get direct accesss to the MDIO bus. 

# Show all attached PHYs (bnased on reports from the MDIO subsystem)

`
