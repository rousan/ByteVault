//
// Created by Ariyan Khan on 07-10-2016.
//

#ifndef BYTEVAULT_COMMAND_ACTIONS_H
#define BYTEVAULT_COMMAND_ACTIONS_H

#include "common.h"
#include "user.h"


/**
 * All commands :- [Here 'argc' is the minimum value of argc argument of main function for particular command]
 * Here {{}} is the MAIN COMMAND
 *  1. {{adduser}} {argc = 2}
 *  2. -u <username> {{addbyte}} -n <name> -k <key> -v <value> [-k <key> -v <value> .....] {argc >= 10}
 *  3. -u <username> {{showbyte}} all | -i <id> | -n <name> {argc = 5 | 6}
 *  4. -u <username> {{editbyte}} -i <id> -k <key> -v <value> [-k <key> -v <value> .....] {argc >= 10}
 *  5. -u <username> {{deletebyte}} all | -i <id> | -n <name> {argc = 5 | 6} {It will keep backups in 'backups' folder in xml format in user wise folder}
 *  6. -u <username> {{backup}} all | -i <id> | -n <name> {argc = 5 | 6} {It will keep backups in 'backups' folder in xml format in user wise folder}
 *  7. -u <username> {{change password}} {argc = 5}
 *  8. -u <username> {{forgot password}} {argc = 5}
 *  9. -u <username> {{show me}} {argc = 5}
 *  10. -u <username> {{delete me}} {argc = 5}
 *  11. /? {Print help docs} {argc = 2}
 *  12. *  5. -u <username> {{deletebytedata}} -i <id>  {It will not keep byte data}
 */

#define BV_SPECIAL_COMMAND_ACTION_EXIT_INPUT_STRING  "."
#define BV_SPECIAL_COMMAND_ACTION_PREV_INPUT_STRING  ".p"

#define BV_SPECIAL_COMMAND_ACTION_NOTHING_CODE -1
#define BV_SPECIAL_COMMAND_ACTION_EXIT_CODE 1
#define BV_SPECIAL_COMMAND_ACTION_PREV_CODE 2


#define BV_EXECUTION_MODE_NORMAL 0
#define BV_EXECUTION_MODE_SHELL 1

#define BV_SHELL_COMMAND_EXIT_CODE 0
#define BV_SHELL_COMMAND_CLEAR_SCREEN_CODE 1
#define BV_SHELL_COMMAND_NORMAL_CODE 2
#define BV_SHELL_COMMAND_SWITCH_USER_CODE 3
#define BV_SHELL_COMMAND_WRONG_COMMAND_CODE 4
#define BV_SHELL_COMMAND_HELP_DOCS_CODE 5
#define BV_SHELL_COMMAND_ADD_USER_CODE 6
#define BV_SHELL_COMMAND_LOG_IN_CODE 7
#define BV_SHELL_COMMAND_LOG_OUT_CODE 8
#define BV_SHELL_COMMAND_SESSION_START_CODE 9
#define BV_SHELL_COMMAND_SESSION_STOP_CODE 10
#define BV_SHELL_COMMAND_SHOWBYTE_SHORTCUT_BY_ID_CODE 11

#define BV_SHELL_COMMAND_EXIT_INPUT_STRING ".exit"
#define BV_SHELL_COMMAND_CLEAR_SCREEN_INPUT_STRING ".cls"
#define BV_SHELL_COMMAND_EXIT2_INPUT_STRING "."
#define BV_SHELL_COMMAND_EXIT3_INPUT_STRING ".bye"
#define BV_SHELL_COMMAND_CLEAR_SCREEN2_INPUT_STRING ".c"
#define BV_SHELL_COMMAND_SWITCH_USER_INPUT_STRING ".switchuser"
#define BV_SHELL_COMMAND_HELP_DOCS_INPUT_STRING ".help"
#define BV_SHELL_COMMAND_ADD_USER_INPUT_STRING ".adduser"
#define BV_SHELL_COMMAND_LOG_IN_INPUT_STRING ".login"
#define BV_SHELL_COMMAND_LOG_OUT_INPUT_STRING ".logout"
#define BV_SHELL_COMMAND_SESSION_START_INPUT_STRING ".sessionstart"
#define BV_SHELL_COMMAND_SESSION_STOP_INPUT_STRING ".sessionstop"



extern int execution_mode;

void bv_ca_adduser(int argc, char **argv);
void bv_ca_addbyte(int argc, char **argv);
void bv_ca_addbytedata(int argc, char **argv);
void bv_ca_showbyte(int argc, char **argv);
void bv_ca_editbyte(int argc, char **argv);
void bv_ca_editbytedata(int argc, char **argv);
void bv_ca_deletebyte(int argc, char **argv);
void bv_ca_deletebytedata(int argc, char **argv);
void bv_ca_backup(int argc, char **argv);
void bv_ca_change_user_info(int argc, char **argv);
void bv_ca_forgot_password(int argc, char **argv);
void bv_ca_show_me(int argc, char **argv);
void bv_ca_delete_me(int argc, char **argv);
void bv_ca_print_help_docs(int argc, char **argv);
void bv_ca_encrypt_file(int argc, char **argv);
void bv_ca_decrypt_file(int argc, char **argv);
void bv_ca_start_user_session();
void bv_ca_stop_user_session();
void bv_ca_print_top_parent_byte_ids(int argc, char **argv);

void bv_start_shell_mode(int argc, char** argv);
int bv_process_shell_command(const char* shell_command);
int bv_process_special_command(const char* command);
int bv_get_username_from_session(char* dest);
bv_bool bv_check_password(const char* correct_pass, const char* testing_pass);

#endif //BYTEVAULT_COMMAND_ACTIONS_H















