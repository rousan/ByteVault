# ByteVault

A command line application that stores sensitive data like `password`, `username`, `email id` etc as key-value pair securely in local machine.

## Usages

Here all your data is considered as a single `Byte`, for example suppose I want to store my `Github` account details, so I have
to add a `Byte` named `Github Account`(whatever you name it) and this byte has two key-value pairs:

 * {&nbsp; key: "Username",&nbsp; value: "ariyankhan" &nbsp;},
 * {&nbsp; key: "Password",&nbsp; value: "Sorry! I can't tell" &nbsp;}
 
 That's it!
 
You can use the following commands to manipulate and store your data into `ByteVault`,

 * `adduser` ( create an user account in `ByteVault` user registry )
 
 * `addbyte` ( add a new `Byte` to the Vault )
 
 * `showbyte` ( show all your private data as structured format )

 * `editbyte` ( edit any particular `Byte` by their `Byte ID` )

 * `deletebyte` ( delete all `Bytes` or any particular `Byte` by their `Byte ID` )

 * `backup` ( keep backup of your data in `backups` folder in xml format )
 
 * `change` ( change Vault account password )

 * `showme` ( show your Vault account details )

 * `delete me` ( delete your own account and all your data )
 
 * `addbytedata` ( add a single key-value paired data into a particular `Byte` ) 
 
 * `editbytedata` ( edit a single key-value paired data in a particular `Byte` )
 
 * `deletebytedata` ( delete a single key-value pair from a particular `Byte`)
 
 * `encfile` ( encrypt any file with your Vault account encryption key, can't be decrypted without your auth )
 
 * `decfile` ( decrypt the encrypted file previously you encrypt by `encfile` command )
 
 * `/?` ( show help docs )
 
 
## Installation
 
 Here `CMake` is used as build tool, so download project files and compile it with `CMAKE` and run it.
 
## Contributors

   * [Rousan Ali](https://github.com/ariyankhan)
   
   Contributions are welcome
   
## License

MIT License

Copyright (c) 2017 Rousan Ali

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
