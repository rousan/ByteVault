//
// Created by Ariyan Khan on 07-10-2016.
//

#ifndef BYTEVAULT_COMMANDS_H
#define BYTEVAULT_COMMANDS_H


#include "common.h"

#define BV_CONSTANT_COMMAND_CHANGE_PASSWORD "/p"
#define BV_CONSTANT_COMMAND_CHANGE_HINTS "/h"
#define BV_CONSTANT_COMMAND_CHANGE_SEC_QUES "/s"
#define BV_CONSTANT_COMMAND_CHANGE_SEC_QUES_ANS "/a"

#define BV_COMMAND_ADDUSER_1 "adduser"
#define BV_COMMAND_ADDUSER_2 "au"

#define BV_COMMAND_ADDBYTE_1 "addbyte"
#define BV_COMMAND_ADDBYTE_2 "ab"

#define BV_COMMAND_SHOWBYTE_1 "showbyte"
#define BV_COMMAND_SHOWBYTE_2 "sb"

#define BV_COMMAND_ADDBYTEDATA_1 "addbytedata"
#define BV_COMMAND_ADDBYTEDATA_2 "abd"

#define BV_COMMAND_EDITBYTEDATA_1 "editbytedata"
#define BV_COMMAND_EDITBYTEDATA_2 "ebd"

#define BV_COMMAND_EDITBYTE_1 "editbyte"
#define BV_COMMAND_EDITBYTE_2 "eb"

#define BV_COMMAND_SHOWME_1 "showme"
#define BV_COMMAND_SHOWME_2 "sm"

#define BV_COMMAND_CHANGEME_1 "changeme"
#define BV_COMMAND_CHANGEME_2 "cm"

#define BV_COMMAND_BACKUPBYTE_1 "backupbyte"
#define BV_COMMAND_BACKUPBYTE_2 "bb"

#define BV_COMMAND_ENCFILE_1 "encfile"
#define BV_COMMAND_ENCFILE_2 "ef"

#define BV_COMMAND_DECFILE_1 "decfile"
#define BV_COMMAND_DECFILE_2 "df"

#define BV_COMMAND_DELETEBYTEDATA_1 "deletebytedata"
#define BV_COMMAND_DELETEBYTEDATA_2 "dbd"

#define BV_COMMAND_DELETEBYTE_1 "deletebyte"
#define BV_COMMAND_DELETEBYTE_2 "db"

#define BV_COMMAND_FORGOTPASSWORD_1 "forgotpassword"
#define BV_COMMAND_FORGOTPASSWORD_2 "fp"

#define BV_COMMAND_DELETEME_1 "deleteme"
#define BV_COMMAND_DELETEME_2 "dm"

#define BV_COMMAND_LIST_TOP_PARENT_BYTE_IDS_1 "ltpb"
#define BV_COMMAND_LIST_TOP_PARENT_BYTE_IDS_2 "lp"


#define BV_COMMAND_HELP_1 "/?"


void command_processor(int argc, char** argv, int app_execution_mode);



#endif //BYTEVAULT_COMMANDS_H



















