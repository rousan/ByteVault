# ByteVault

A command line application that stores sensitive data like `password`, `username`, `email id` etc as key-value pair securely 
in local machine.

## Usages

Here all your data is considered as a single `Byte`, for example suppose I want to store my `Github` account details, so I have
to add a `Byte` named `Github Account`(whatever you name it) and this byte has two key-value pairs:

 * {key: `Username`, value: `ariyankhan`},
 * {key: `Password`, value: `Sorry! I can't tell`}
 
 Thats it!
 
You can use the following commands to manipulate and store your data into `ByteVault`,

 * `adduser`    : create an user account in `ByteVault` user registry
 * `addbyte`    : add a new `Byte` to the Vault
 * `showbyte`   : show all your private data as structured format
     
    
