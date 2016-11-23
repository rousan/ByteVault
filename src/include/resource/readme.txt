Introduction:
ByteVault is a utility app by which you can store any sensitive data in a structured format with high level security. ByteVault uses
file-folder and key-value relationship to store data. So just take this as following:
    byte : folder
    byte-data : files under this folder
    byte-data-key : file name
    byte-data-key-value : file contents

It stores your data in key-value pairs. A single byte can contain multiple key-value pairs.
But key must be unique in same byte, you can add multiple bytes like this format,
Also you can add a byte in another byte like folder in another folder.
Every byte that can be parent byte or child byte will has a unique id that is called byte_id.
And you can refer a particular byte only by this id.
In ByteVault there is three type of bytes : Top-Parent-Byte, Parent-Byte and Child-Byte
    Top-Parent-Byte : which has no parent byte, i.e. parent byte id is 0.
    Parent-Byte : which has a particular parent byte and child bytes.
    Child-Byte : which has a particular parent byte and can have child bytes,
    a Parent-Byte can be Child-Byte with respect to other Parent-Byte or other Top-Parent-Byte.

You can add multiple Top-Parent-Bytes. So to show bytes, only you have to remember the Top-Parent-Bytes's id.
If you have lot of Top-Parent-Bytes then you have to remember all lot of ids?
No.., Just type 'LP' without quotes.


ByteVault uses end-to-end encryption to encrypt data so nobody can decrypt this data other than the user of this
vault. WE CAN CHALLENGE THAT NO BODY CAN HACK THIS DATA OTHER THAN THE USER OF THIS DATA VAULT.

Uses:
You can use this to store your any private account details, password, etc sensitive data without any doubt because all data will
be stored as encrypted format.
Not only sensitive data, any other data can be stored.

Limitation:
In a word there is no limitation to store data. The limitation is
    username : maximum 20 char
    password : minimum 5 char and maximum 20 char
    a byte name : maximum 100 char
    a byte-data-key-name : maximum 100 char
    one single byte-data-key-value : maximum 1024 * 1024 * 5 = 5242880 char = 5 MB (This is enough :D)

One other limitation is that you can only store characters of range in from 1 to 127 ASCII value i.e.
All the characters we commonly use are in this range.

How to use:
Now, lets start of using it. Steps are
    1. Copy the full path of 'bin' folder and append it to the PATH environment variable.
    2. Open your command prompt and just type 'bv' or 'bytevault' without quotes, then you will see a setup
    instructions. So just follow it.
    3. add a new user, though you can add multiple users in same desktop. To add a new user just
    type '.adduser' without quotes any time.
    4. To login type '.login' without quotes.
    5. And to add a byte type 'addbyte' without quotes, now type byte name, byte's parent byte id, if no,
    then just skip, now your byte is created,
    6. Now to add data in this byte just type 'addbyetdata' without quotes and give your data key name
    and then give the data value.
    7. Now you have successfully store your data.
    8. To show bytes enter 'showbyte' without quotes, and enter '-i <byte_id>' without quotes or 'all' without quotes or type
    '-n <byte_name>' without quotes.
    8. To log out just type '.logout' without quotes and to exit just type '.' without quotes.
    9. To get full docs of all available commands type '.help' without quotes.








