//
// Created by Ariyan Khan on 07-10-2016.
//



#include "include/commands.h"
#include "include/logger.h"
#include "include/command_actions.h"


/**
 * All commands :- [Here 'argc' is the minimum value of argc argument of main function for particular command]
 * Here {{}} is the MAIN COMMAND
 *  1. {{adduser}} {argc = 2}
 *  2. -u <username> {{addbyte}} -n <name> -k <key> -v <value> [-k <key> -v <value> .....] {argc >= 10}
 *  3. -u <username> {{showbyte /i | /d}} all | -i <id> | -n <name> {argc = 5 | 6}
 *  4. -u <username> {{editbyte}} -i <id> -k <key> -v <value> [-k <key> -v <value> .....] {argc >= 10}
 *  5. -u <username> {{deletebyte}} all | -i <id> {argc = 5 | 6} {It will keep backups in 'backups' folder in xml format in user wise folder}
 *  6. -u <username> {{backup}} all | -i <id> {argc = 5 | 6} {It will keep backups in 'backups' folder in xml format in user wise folder}
 *  7. -u <username> {{change /p | /h | /s | /a}} {argc = 5}
 *  8. -u <username> {{forgot password}} {argc = 5}
 *  9. -u <username> {{showme}} {argc = 5}
 *  10. -u <username> {{delete me}} {argc = 5}
 *  11. /? {Print help docs} {argc = 2}
 *  12. {{addbytedata}} -i <byte_id>
 *  13 {{editbytedata}} -i <byte_id> -k <byte_data_key>
 *  14. {{deletebytedata}} -i <byte_id> -k <byte_data_key>
 *  15. {{encfile}} filename_path {encryption with current user enc key}
 *  16. {{decfile}} filename_path {encryption with current user enc key}
 *
 */



void command_processor(int argc, char** argv, int app_execution_mode) {
    execution_mode = app_execution_mode;
    if(argc <= 1) {
        bv_start_shell_mode(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_ADDUSER_1) || !strcmpi(argv[1], BV_COMMAND_ADDUSER_2))) {
        bv_ca_adduser(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_ADDBYTE_1) || !strcmpi(argv[1], BV_COMMAND_ADDBYTE_2))) {
        bv_ca_addbyte(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_SHOWBYTE_1) || !strcmpi(argv[1], BV_COMMAND_SHOWBYTE_2))) {
        bv_ca_showbyte(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_ADDBYTEDATA_1) || !strcmpi(argv[1], BV_COMMAND_ADDBYTEDATA_2))) {
        bv_ca_addbytedata(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_EDITBYTEDATA_1) || !strcmpi(argv[1], BV_COMMAND_EDITBYTEDATA_2))) {
        bv_ca_editbytedata(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_EDITBYTE_1) || !strcmpi(argv[1], BV_COMMAND_EDITBYTE_2))) {
        bv_ca_editbyte(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_SHOWME_1) || !strcmpi(argv[1], BV_COMMAND_SHOWME_2))) {
        bv_ca_show_me(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_CHANGEME_1) || !strcmpi(argv[1], BV_COMMAND_CHANGEME_2))) {
        bv_ca_change_user_info(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_BACKUPBYTE_1) || !strcmpi(argv[1], BV_COMMAND_BACKUPBYTE_2))) {
        bv_ca_backup(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_ENCFILE_1) || !strcmpi(argv[1], BV_COMMAND_ENCFILE_2))) {
        bv_ca_encrypt_file(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_DECFILE_1) || !strcmpi(argv[1], BV_COMMAND_DECFILE_2))) {
        bv_ca_decrypt_file(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_DELETEBYTEDATA_1) || !strcmpi(argv[1], BV_COMMAND_DELETEBYTEDATA_2))) {
        bv_ca_deletebytedata(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_FORGOTPASSWORD_1) || !strcmpi(argv[1], BV_COMMAND_FORGOTPASSWORD_2))) {
        bv_ca_forgot_password(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_DELETEBYTE_1) || !strcmpi(argv[1], BV_COMMAND_DELETEBYTE_2))) {
        bv_ca_deletebyte(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_DELETEME_1) || !strcmpi(argv[1], BV_COMMAND_DELETEME_2))) {
        bv_ca_delete_me(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_LIST_TOP_PARENT_BYTE_IDS_1) || !strcmpi(argv[1], BV_COMMAND_LIST_TOP_PARENT_BYTE_IDS_2))) {
        bv_ca_print_top_parent_byte_ids(argc, argv);
    } else if(argc >= 2 && (!strcmpi(argv[1], BV_COMMAND_HELP_1))) {
        bv_ca_print_help_docs(argc, argv);
    } else {
        if(execution_mode == BV_EXECUTION_MODE_NORMAL) {
            bv_error("Please type any command, for help type /?");
        } else {
            bv_error("Invalid command, type .help to get help docs");
        }
    }
}






























