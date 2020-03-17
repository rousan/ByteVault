//
// Created by Rousan Ali on 07-10-2016.
//

#include <time.h>
#include <windows.h>
#include <conio.h>

#include "include/command_actions.h"
#include "include/logger.h"
#include "include/utils.h"
#include "include/viewer.h"
#include "include/sqlite/sqlite3.h"
#include "include/database.h"
#include "include/user.h"
#include "include/constants.h"
#include "include/commands.h"
#include "include/encryption.h"

int execution_mode;
short bv_backup_back_to_chars;
short bv_encrypt_file_back_to_chars;
short bv_decrypt_file_back_to_chars;
short bv_delete_byte_back_to_chars;
static void bv_backup_callback(int code, size_t total_backed_up, size_t total_bytes);
static void bv_encrypt_file_callback(int code, size_t bytes_encrypted, size_t total_bytes);
static void bv_decrypt_file_callback(int code, size_t bytes_decrypted, size_t total_bytes);
static void bv_delete_byte_callback(int code, size_t bytes_deleted, size_t total_bytes);

/**
 * TOO CHECK IS USER LOGGED IN OR NOT USE bv_logged_in_user global variable
 *  bv_logged_in_user == NULL -> NOT LOGGED IN
 *  bv_logged_in_user points CURRENT_USER
 *
 * @param argc
 * @param argv
 */
void bv_start_shell_mode(int argc, char **argv)
{
    char *username = malloc(20 + 5); //max 20 chars
    char *password = malloc(20 + 5); //max 20 chars
    char *temp = malloc(300);
    size_t ln = 0;
    size_t temp_size_t = 0;
    char *errMSG;
    int rc;
    int scc;
    bv_user user_temp;
    char *shell_command = malloc(300);
    size_t argc_temp;
    char **argv_temp;
    char **argv_temp2;
    char *char_ptr_temp = null;
    bv_node *temp_node = null;
    char *char_temp_ptr2 = null;

    /*
     * Set the execution mode it is important
     */
    execution_mode = BV_EXECUTION_MODE_SHELL;
    /*
     * Initially bv_logged_in_user is null.
     */
    bv_logged_in_user = null;

    bv_clear_screen();

    sprintf(temp, "ByteVault Version [%s]", BV_APP_VERSION_NAME);
    fprintf(stdout, "%s\n\n", temp);

    rc = bv_get_username_from_session(username);
    if (rc == BV_OP_SUCCESS)
    {
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = malloc(sizeof(bv_user));
        rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
        if (rc == BV_OP_SUCCESS)
        {
            fprintf(stdout, "Login session found as: ");
            bv_console_text_color(0x2);
            fprintf(stdout, "%s", bv_logged_in_user->username);
            bv_apply_normal_color();
            fprintf(stdout, "\n");

        GOTO_FOR_SESSION_PASSWORD:
            while (1)
            {
                bv_apply_input_string_key_color();
                fprintf(stdout, "Enter password: ");
                fseek(stdin, 0, SEEK_END);
                bv_apply_input_string_value_color();
                bv_read_password_from_console(temp, 110, 1);
                ln = strlen(temp);
                bv_apply_normal_color();
                scc = bv_process_special_command(temp);
                switch (scc)
                {
                case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
                    bv_clear_screen();
                    sprintf(temp, "ByteVault Version [%s]", BV_APP_VERSION_NAME);
                    fprintf(stdout, "%s\n\n", temp);
                    if (bv_logged_in_user != NULL)
                    {
                        free(bv_logged_in_user);
                    }
                    bv_logged_in_user = null;
                    goto GOTO_FOR_SHELL_COMMAND_LOOP;
                    break;
                case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
                    goto GOTO_FOR_SESSION_PASSWORD;
                    break;
                default:
                    break;
                }
                if (!(ln >= 5 && ln <= 20))
                {
                    bv_error("Password should contain at least 5 characters and maximum 20 characters");
                    continue;
                }
                else
                {
                    if (bv_check_password(bv_logged_in_user->password, temp))
                    {
                        bv_open_db_connections();
                        bv_clear_screen();
                        fprintf(stdout, "ByteVault [Logged in as: ");
                        bv_console_text_color(0x2);
                        fprintf(stdout, "%s", bv_logged_in_user->username);
                        bv_apply_normal_color();
                        fprintf(stdout, "]\n\n");
                        break;
                    }
                    else
                    {
                        bv_error("Password is not correct");
                        continue;
                    }
                }
            }
        }
        else
        {
            if (bv_logged_in_user != NULL)
            {
                free(bv_logged_in_user);
            }
            bv_logged_in_user = null;
        }
    }

    goto GOTO_FOR_SHELL_COMMAND_LOOP;

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            goto GOTO_FOR_SHELL_COMMAND_LOOP;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        ln = strlen(temp);
        bv_apply_normal_color();
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            goto GOTO_FOR_SHELL_COMMAND_LOOP;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(shell_command);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

    bv_clear_screen();
    fprintf(stdout, "ByteVault [Logged in as: ");
    bv_console_text_color(0x2);
    fprintf(stdout, "%s", username);
    bv_apply_normal_color();
    fprintf(stdout, "]\n\n");

GOTO_FOR_SHELL_COMMAND_LOOP:
    while (1)
    {
        bv_apply_normal_color();
        fprintf(stdout, ">> ");
        fseek(stdin, 0, SEEK_END);
        fgets(temp, 200, stdin);
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        if (ln < 1)
        {
            //bv_error("PLease enter some command");
        }
        else
        {
            strcpy(shell_command, temp);
            scc = bv_process_shell_command(shell_command);
            if (scc == BV_SHELL_COMMAND_NORMAL_CODE)
            {
                char_ptr_temp = bv_split_str(shell_command, " ", &argv_temp, &argc_temp);
                if (argv_temp == null || argc_temp == 0)
                {
                    //bv_error("Please enter some command");
                    free(char_ptr_temp);
                    continue;
                }
                argv_temp2 = malloc((argc_temp + 1) * sizeof(char *));
                memcpy(argv_temp2 + 1, argv_temp, argc_temp * sizeof(char *));
                argv_temp2[0] = argv[0];
                free(argv_temp);
                command_processor((int)argc_temp + 1, argv_temp2, execution_mode);
                free(char_ptr_temp);
                free(argv_temp2);
            }
            else
            {
                switch (scc)
                {
                case BV_SHELL_COMMAND_EXIT_CODE:
                    free(username);
                    free(password);
                    free(temp);
                    free(shell_command);
                    return;
                case BV_SHELL_COMMAND_CLEAR_SCREEN_CODE:
                    if (bv_logged_in_user != null)
                    {
                        bv_clear_screen();
                        fprintf(stdout, "ByteVault [Logged in as: ");
                        bv_console_text_color(0x2);
                        fprintf(stdout, "%s", bv_logged_in_user->username);
                        bv_apply_normal_color();
                        fprintf(stdout, "]\n\n");
                    }
                    else
                    {
                        bv_clear_screen();
                        sprintf(temp, "ByteVault Version [%s]", BV_APP_VERSION_NAME);
                        fprintf(stdout, "%s\n\n", temp);
                    }
                    break;
                case BV_SHELL_COMMAND_SWITCH_USER_CODE:
                    goto GOTO_FOR_USERNAME;
                    break;
                case BV_SHELL_COMMAND_WRONG_COMMAND_CODE:
                    bv_error("Invalid command, type .help to get help docs");
                    break;
                case BV_SHELL_COMMAND_HELP_DOCS_CODE:
                    bv_ca_print_help_docs(argc, argv);
                    break;
                case BV_SHELL_COMMAND_ADD_USER_CODE:
                    if (argc >= 1)
                    {
                        argv_temp = malloc(sizeof(char *) * 1);
                        argv_temp[0] = argv[0];
                        bv_ca_adduser(1, argv_temp);
                    }
                    else
                    {
                        bv_ca_adduser(0, null);
                    }
                    break;
                case BV_SHELL_COMMAND_LOG_IN_CODE:
                    goto GOTO_FOR_USERNAME;
                    break;
                case BV_SHELL_COMMAND_LOG_OUT_CODE:
                    if (bv_logged_in_user == null)
                    {
                        bv_error("Please login to access database");
                        continue;
                    }
                    free(bv_logged_in_user);
                    bv_logged_in_user = null;
                    bv_clear_screen();
                    sprintf(temp, "ByteVault Version [%s]", BV_APP_VERSION_NAME);
                    fprintf(stdout, "%s\n\n", temp);
                    break;
                case BV_SHELL_COMMAND_SESSION_START_CODE:
                    bv_ca_start_user_session();
                    break;
                case BV_SHELL_COMMAND_SESSION_STOP_CODE:
                    bv_ca_stop_user_session();
                    break;
                case BV_SHELL_COMMAND_SHOWBYTE_SHORTCUT_BY_ID_CODE:
                    if (bv_logged_in_user == null)
                    {
                        bv_error("Please login to access database");
                        continue;
                    }
                    temp_node = null;
                    bv_trim(shell_command, temp);
                    char_ptr_temp = strtok(temp + 1, " ");
                    temp_size_t = 0;
                    sscanf(char_ptr_temp, "%llu", &temp_size_t);

                    char_ptr_temp = strtok(NULL, " ");
                    char_temp_ptr2 = malloc(10);
                    if (char_ptr_temp == null)
                    {
                        strcpy(char_temp_ptr2, "-d");
                    }
                    else
                    {
                        if (!strcmpi(char_ptr_temp, "/i"))
                        {
                            strcpy(char_temp_ptr2, "-i");
                        }
                        else
                        {
                            strcpy(char_temp_ptr2, "-d");
                        }
                    }

                    sprintf(temp, "-i %llu", temp_size_t);
                    rc = bv_query_bytes(bv_logged_in_user->username, temp, 1, null, 1, &temp_node, &errMSG);
                    if (rc != BV_OP_SUCCESS)
                    {
                        bv_error("Could not proceed due to: %s", errMSG);
                        free(char_temp_ptr2);
                        continue;
                    }
                    if (temp_node == null)
                    {
                        bv_error("This byte id does not exist");
                        free(char_temp_ptr2);
                        continue;
                    }
                    bv_print_bytes("Bytes", temp_node, char_temp_ptr2);
                    free(char_temp_ptr2);
                    /*
                         * Here dont free temp_node bcoz IT IS ALREADY FREED IN bv_print_bytes function.
                         */
                    break;
                default:
                    break;
                }
            }
        }
    }
    free(username);
    free(password);
    free(temp);
    free(shell_command);
}

int bv_process_shell_command(const char *shell_command)
{
    char *temp = malloc(strlen(shell_command) + 10);
    size_t temp_size_t = 0;

    bv_trim(shell_command, temp);
    if (bv_starts_with_str(temp, "."))
    {
        if (!strcmpi(temp, BV_SHELL_COMMAND_EXIT_INPUT_STRING) || !strcmpi(temp, BV_SHELL_COMMAND_EXIT3_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_EXIT_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_EXIT2_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_EXIT_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_CLEAR_SCREEN_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_CLEAR_SCREEN_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_CLEAR_SCREEN2_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_CLEAR_SCREEN_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_SWITCH_USER_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_SWITCH_USER_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_HELP_DOCS_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_HELP_DOCS_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_ADD_USER_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_ADD_USER_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_LOG_IN_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_LOG_IN_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_LOG_OUT_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_LOG_OUT_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_SESSION_START_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_SESSION_START_CODE;
        }
        else if (!strcmpi(temp, BV_SHELL_COMMAND_SESSION_STOP_INPUT_STRING))
        {
            return BV_SHELL_COMMAND_SESSION_STOP_CODE;
        }
        else if (strlen(temp) >= 2 && sscanf(temp + 1, "%llu", &temp_size_t) > 0)
        {
            return BV_SHELL_COMMAND_SHOWBYTE_SHORTCUT_BY_ID_CODE;
        }
        else
        {
            return BV_SHELL_COMMAND_WRONG_COMMAND_CODE;
        }
    }
    else
    {
        return BV_SHELL_COMMAND_NORMAL_CODE;
    }
}

void bv_ca_encrypt_file(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    char *temp = malloc(MAX_PATH + 200);
    char *temp2 = malloc(MAX_PATH + 200);
    char *source_file_full_path = malloc(MAX_PATH + 300);
    char *dest_file_full_path = malloc(MAX_PATH + 200);
    char *dest_file_folder_full_path = malloc(MAX_PATH + 200);
    bv_user user_temp;
    char *errMSG;
    int rc;
    size_t ln;
    int scc = 0;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_SOURCE_FILE;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(source_file_full_path);
        free(dest_file_folder_full_path);
        free(dest_file_full_path);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_SOURCE_FILE:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter target file path: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, MAX_PATH + 50, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_SOURCE_FILE;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= MAX_PATH))
        {
            bv_error("Target file path should contain at least 1 character and maximum %llu characters", MAX_PATH);
            continue;
        }
        else
        {
            bv_get_absolute_path(temp, source_file_full_path);
            if (!bv_is_existing_file(source_file_full_path))
            {
                bv_error("This path does not exist or not a valid file");
                continue;
            }
            bv_get_path_parts(source_file_full_path, dest_file_folder_full_path, temp2);
            strcpy(dest_file_full_path, source_file_full_path);
            strcat(dest_file_full_path, BV_ENCRYPTED_FILE_EXTENSION);
            break;
        }
    }

    if (bv_is_existing_file(dest_file_full_path))
    {
    GOTO_FOR_FILE_OVERWRITTEN:
        while (1)
        {
            bv_apply_input_string_key_color();
            fprintf(stdout, "This File is already encrypted, overwrite? type y or n(default y): ");
            fseek(stdin, 0, SEEK_END);
            bv_apply_input_string_value_color();
            fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
            bv_apply_normal_color();
            ln = strlen(temp);
            (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
            ln = strlen(temp);
            scc = bv_process_special_command(temp);
            switch (scc)
            {
            case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
                free(username);
                free(password);
                free(temp);
                free(temp2);
                free(source_file_full_path);
                free(dest_file_folder_full_path);
                free(dest_file_full_path);
                return;
                break;
            case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
                goto GOTO_FOR_SOURCE_FILE;
                break;
            default:
                break;
            }
            if (ln == 0)
            {
                break;
            }
            else
            {
                if (!strcmpi("y", temp) || !strcmpi("yes", temp))
                {
                    break;
                }
                else
                {
                    bv_warn("Encryption is aborted");
                    goto GOTO_FOR_FILE_LOCATION_DIALOG;
                }
            }
        }
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "Encrypting file: ");
    bv_apply_normal_color();

    bv_apply_input_string_value_color();
    sprintf(temp, "%s", "0.00%");
    bv_encrypt_file_back_to_chars = (short)strlen(temp);
    fprintf(stdout, "%s", temp);
    bv_apply_normal_color();

    rc = bv_encrypt_file(source_file_full_path, dest_file_full_path, bv_logged_in_user->enc_key, 0x1, bv_encrypt_file_callback, &errMSG);
    fprintf(stdout, "\n");

    if (rc != BV_OP_SUCCESS)
    {
        bv_error("Encryption is aborted due to: %s", errMSG);
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(source_file_full_path);
        free(dest_file_folder_full_path);
        free(dest_file_full_path);
        return;
    }
    else
    {
        bv_log("Encryption is completed");
    }

GOTO_FOR_FILE_LOCATION_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Would you like to open encrypted file location, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_FILE_LOCATION_DIALOG;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                bv_open_path_in_explorer(dest_file_folder_full_path);
            }
            break;
        }
    }

    free(username);
    free(password);
    free(temp);
    free(temp2);
    free(source_file_full_path);
    free(dest_file_folder_full_path);
    free(dest_file_full_path);
}

void bv_ca_decrypt_file(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    char *temp = malloc(MAX_PATH + 200);
    char *temp2 = malloc(MAX_PATH + 200);
    char *source_file_full_path = malloc(MAX_PATH + 300);
    char *dest_file_full_path = malloc(MAX_PATH + 200);
    char *dest_file_folder_full_path = malloc(MAX_PATH + 200);
    bv_user user_temp;
    char *errMSG;
    int rc;
    size_t ln;
    int scc = 0;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_SOURCE_FILE;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(source_file_full_path);
        free(dest_file_folder_full_path);
        free(dest_file_full_path);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_SOURCE_FILE:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter encrypted file path: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, MAX_PATH + 50, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_SOURCE_FILE;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= MAX_PATH))
        {
            bv_error("Encrypted file path should contain at least 1 character and maximum %llu characters", MAX_PATH);
            continue;
        }
        else
        {
            bv_get_absolute_path(temp, source_file_full_path);
            if (!bv_is_existing_file(source_file_full_path))
            {
                bv_error("This path does not exist or not a valid file");
                continue;
            }
            if (!bv_ends_with_str(source_file_full_path, BV_ENCRYPTED_FILE_EXTENSION))
            {
                bv_error("This file is not a valid encrypted file");
                continue;
            }
            bv_get_path_parts(source_file_full_path, dest_file_folder_full_path, temp2);
            strcpy(dest_file_full_path, source_file_full_path);
            dest_file_full_path[strlen(dest_file_full_path) - strlen(BV_ENCRYPTED_FILE_EXTENSION)] = '\0';
            break;
        }
    }

    if (bv_is_existing_file(dest_file_full_path))
    {
    GOTO_FOR_FILE_OVERWRITTEN:
        while (1)
        {
            bv_apply_input_string_key_color();
            fprintf(stdout, "This File is already decrypted, overwrite? type y or n(default y): ");
            fseek(stdin, 0, SEEK_END);
            bv_apply_input_string_value_color();
            fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
            bv_apply_normal_color();
            ln = strlen(temp);
            (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
            ln = strlen(temp);
            scc = bv_process_special_command(temp);
            switch (scc)
            {
            case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
                free(username);
                free(password);
                free(temp);
                free(temp2);
                free(source_file_full_path);
                free(dest_file_folder_full_path);
                free(dest_file_full_path);
                return;
                break;
            case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
                goto GOTO_FOR_SOURCE_FILE;
                break;
            default:
                break;
            }
            if (ln == 0)
            {
                break;
            }
            else
            {
                if (!strcmpi("y", temp) || !strcmpi("yes", temp))
                {
                    break;
                }
                else
                {
                    bv_warn("Decryption is aborted");
                    goto GOTO_FOR_FILE_LOCATION_DIALOG;
                }
            }
        }
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "Decrypting file: ");
    bv_apply_normal_color();

    bv_apply_input_string_value_color();
    sprintf(temp, "%s", "0.00%");
    bv_decrypt_file_back_to_chars = (short)strlen(temp);
    fprintf(stdout, "%s", temp);
    bv_apply_normal_color();

    rc = bv_decrypt_file(source_file_full_path, dest_file_full_path, bv_logged_in_user->enc_key, 0x1, bv_decrypt_file_callback, &errMSG);
    fprintf(stdout, "\n");

    if (rc != BV_OP_SUCCESS)
    {
        bv_error("Decryption is aborted due to: %s", errMSG);
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(source_file_full_path);
        free(dest_file_folder_full_path);
        free(dest_file_full_path);
        return;
    }
    else
    {
        bv_log("Decryption is completed");
    }

GOTO_FOR_FILE_LOCATION_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Would you like to open decrypted file location, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(source_file_full_path);
            free(dest_file_folder_full_path);
            free(dest_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_FILE_LOCATION_DIALOG;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                bv_open_path_in_explorer(dest_file_folder_full_path);
            }
            break;
        }
    }

    free(username);
    free(password);
    free(temp);
    free(temp2);
    free(source_file_full_path);
    free(dest_file_folder_full_path);
    free(dest_file_full_path);
}

void bv_ca_delete_me(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    bv_user user_temp;
    char *errMSG;
    char *temp = malloc(MAX_PATH + 300);
    int rc;
    size_t ln;
    int scc = 0;
    bv_bool user_response = bv_false;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_RE_ENTER_PASSWORD;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

    if (execution_mode == BV_EXECUTION_MODE_NORMAL)
    {
        goto GOTO_FOR_END;
    }

GOTO_FOR_RE_ENTER_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter current password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_RE_ENTER_PASSWORD;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            if (bv_check_password(password, temp))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

GOTO_FOR_END:
    bv_warn("Before de-activate this account, please make sure something:\n"
            "\t1. You have backed up all your data\n"
            "\t2. You have decrypted your all backed up files and encrypted files\n"
            "\tbecause after deleting account you can not decrypt these anymore.");

GOTO_FOR_CONFIRM_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Are you sure to delete account? type y or n: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_CONFIRM_DIALOG;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            user_response = bv_false;
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                user_response = bv_true;
                break;
            }
            else
            {
                user_response = bv_false;
                break;
            }
        }
    }

    if (!user_response)
    {
        free(username);
        free(password);
        free(temp);
        bv_log("Account deleting is aborted");
        return;
    }

    bv_log("Your account is deleting...");
    bv_get_root_folder_path(temp, MAX_PATH);
    strcat(temp, "\\");
    strcat(temp, BV_FOLDER_DATA);
    strcat(temp, "\\");
    strcat(temp, username);
    strcat(temp, BV_APPEND_STRING_USERS_DATA_DB);

    bv_close_db_connections();

    rc = remove(temp);
    if (rc == 0)
    {
        rc = bv_delete_user(username, &errMSG);
        if (rc == BV_OP_SUCCESS)
        {
            bv_log("So sad, account is deleted, bye");
            bv_log("Now app will exit, enter any key...");
            getch();
            free(username);
            free(password);
            free(temp);
            bv_clear_screen();
            bv_exit(0);
            return;
        }
        else
        {
            bv_error("Account could not be deleted completely due to: %s", errMSG);
            bv_open_db_connections();
        }
    }
    else
    {
        bv_error("Account deleting aborted due to: database is locked at this moment");
        bv_open_db_connections();
    }

    free(username);
    free(password);
    free(temp);
}

void bv_ca_print_help_docs(int argc, char **argv)
{
    bv_log("Help docs are coming...."); //It is for both mode: normal and shell mode
}

/**
 * It is independent to SHELL MODE AND NORMAL MODE
 * @param argc
 * @param argv
 */
void bv_ca_adduser(int argc, char **argv)
{
    char *username = malloc(20 + 1);  //max 20 chars
    char *password = malloc(20 + 1);  //max 20 chars
    char *hints = malloc(110);        //max 100 chars
    char *sec_ques = malloc(110);     //max 100 chars
    char *sec_ques_ans = malloc(110); //max 100 chars
    char *enc_key = malloc(50);
    char *temp = malloc(400);
    size_t ln;
    bv_user user_temp;
    char *errMSG;
    int rc;
    int scc;
    bv_bool user_response_sure = bv_false;

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(hints);
            free(sec_ques);
            free(sec_ques_ans);
            free(enc_key);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                break;
            }
            else
            {
                bv_error("Username is already exist");
                continue;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(hints);
            free(sec_ques);
            free(sec_ques_ans);
            free(enc_key);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain maximum 20 characters and minimum 5 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            break;
        }
    }

GOTO_FOR_REPEAT_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Repeat password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(hints);
            free(sec_ques);
            free(sec_ques_ans);
            free(enc_key);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_PASSWORD;
            break;
        default:
            break;
        }
        if (strcmp(temp, password))
        {
            bv_error("Password does not match with previous one");
            continue;
        }
        else
        {
            break;
        }
    }

GOTO_FOR_HINTS:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter hints: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(hints);
            free(sec_ques);
            free(sec_ques_ans);
            free(enc_key);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_PASSWORD;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Hints should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(hints, temp);
            break;
        }
    }

GOTO_FOR_SEC_QUES:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter security question: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(hints);
            free(sec_ques);
            free(sec_ques_ans);
            free(enc_key);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_HINTS;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Security question should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(sec_ques, temp);
            break;
        }
    }

GOTO_FOR_SEC_QUES_ANS:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter answer: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(hints);
            free(sec_ques);
            free(sec_ques_ans);
            free(enc_key);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_SEC_QUES;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Security question answer should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(sec_ques_ans, temp);
            break;
        }
    }

GOTO_FOR_CONFIRM_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Are you sure, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(hints);
            free(sec_ques);
            free(sec_ques_ans);
            free(enc_key);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_SEC_QUES_ANS;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            user_response_sure = bv_false;
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                user_response_sure = bv_true;
            }
            else
            {
                user_response_sure = bv_false;
            }
            break;
        }
    }

    if (user_response_sure)
    {
        bv_generate_random_key(enc_key, 33); //33 is the enc key size
        rc = bv_get_user_info(username, &user_temp, &errMSG);
        if (rc == BV_OP_FAILED)
        {
            user_temp.username = username;
            user_temp.password = password;
            user_temp.hints = hints;
            user_temp.enc_key = enc_key;
            user_temp.sec_ques = sec_ques;
            user_temp.sec_ques_ans = sec_ques_ans;
            rc = bv_insert_user(&user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS)
            {
                bv_log("New user is added");
                bv_print_user_info("UserInfo", &user_temp, 0, 0);
            }
            else
            {
                sprintf(temp, "User is not added due to %s", errMSG);
                bv_error(temp);
            }
        }
        else
        {
            bv_error("Username is already exist");
        }
    }
    else
    {
        bv_log("New user is not added");
    }

    free(username);
    free(password);
    free(hints);
    free(enc_key);
    free(sec_ques);
    free(sec_ques_ans);
    free(temp);
}

/**
 * Here if the command is '.' then it will exit.
 * But one limitation is that no input value can not be '.', because it will interpreted as
 * special command action EXIT.
 * ALL SPECIAL COMMANDS CAN NOT BE INPUT. THIS IS THE LIMITATION AT THIS MOMENT.
 * @param command
 */
int bv_process_special_command(const char *command)
{
    if (command == NULL)
    {
        return BV_SPECIAL_COMMAND_ACTION_NOTHING_CODE;
    }
    else if (!strcmp(command, BV_SPECIAL_COMMAND_ACTION_EXIT_INPUT_STRING))
    {
        return BV_SPECIAL_COMMAND_ACTION_EXIT_CODE;
    }
    else if (!strcmp(command, BV_SPECIAL_COMMAND_ACTION_PREV_INPUT_STRING))
    {
        return BV_SPECIAL_COMMAND_ACTION_PREV_CODE;
    }
    return BV_SPECIAL_COMMAND_ACTION_NOTHING_CODE;
}

void bv_ca_addbyte(int argc, char **argv)
{
    char *username = malloc(20 + 5); //max 20 chars
    char *password = malloc(20 + 5); //max 20 chars
    char *byte_name = malloc(200);
    char *last_modifying_text = malloc(200);
    unsigned long long parent_byte_id = 0;
    bv_user user_temp;
    char *errMSG;
    int rc;
    char *temp = malloc(300);
    size_t ln;
    int scc = 0;
    bv_byte *byte_ptr = null;
    time_t time_in_secs;
    bv_data *bv_data_ptr = null;
    bv_node *temp_node_ptr = null;
    char *errMessage = null;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_BYTE_NAME;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(byte_name);
            free(last_modifying_text);
            (byte_ptr) ? (free(byte_ptr)) : (0);
            (bv_data_ptr) ? (free(bv_data_ptr)) : (0);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(byte_name);
            free(last_modifying_text);
            (byte_ptr) ? (free(byte_ptr)) : (0);
            (bv_data_ptr) ? (free(bv_data_ptr)) : (0);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(byte_name);
            free(last_modifying_text);
            (byte_ptr) ? (free(byte_ptr)) : (0);
            (bv_data_ptr) ? (free(bv_data_ptr)) : (0);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(byte_name);
        free(last_modifying_text);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_BYTE_NAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter byte name: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(byte_name);
            free(last_modifying_text);
            (byte_ptr) ? (free(byte_ptr)) : (0);
            (bv_data_ptr) ? (free(bv_data_ptr)) : (0);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_NAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Byte name should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(byte_name, temp);
            break;
        }
    }

GOTO_FOR_PARENT_BYTE_ID:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter parent byte id: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(byte_name);
            free(last_modifying_text);
            (byte_ptr) ? (free(byte_ptr)) : (0);
            (bv_data_ptr) ? (free(bv_data_ptr)) : (0);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_NAME;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            parent_byte_id = 0;
        }
        else
        {
            if (!(ln >= 1 && ln <= 20))
            {
                bv_error("Parent byte id should contain at least 1 character and maximum 20 characters");
                continue;
            }
            else
            {
                rc = sscanf(temp, "%llu", &parent_byte_id);
                if (rc == 0)
                {
                    bv_error("Invalid byte id, please enter number");
                    continue;
                }
            }
        }
        if (parent_byte_id == 0)
        {
            break;
        }
        else
        {
            sprintf(temp, "-i %llu", parent_byte_id);
            rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
            if (rc == BV_OP_FAILED)
            {
                sprintf(temp, "Could not processed due to: %s", errMessage);
                bv_error(temp);
                free(username);
                free(password);
                free(temp);
                free(byte_name);
                free(last_modifying_text);
                (byte_ptr) ? (free(byte_ptr)) : (0);
                (bv_data_ptr) ? (free(bv_data_ptr)) : (0);
                return;
            }
            if (temp_node_ptr == NULL)
            {
                bv_error("The parent byte id does not exist");
                continue;
            }
            break;
        }
    }

    strcpy(last_modifying_text, "The byte is not yet modified");
    time_in_secs = time(null);
    byte_ptr = malloc(sizeof(bv_byte));
    byte_ptr->data_count = 0;
    byte_ptr->data = null;
    byte_ptr->name = byte_name;
    byte_ptr->last_modifying_text = last_modifying_text;
    byte_ptr->date_created = time_in_secs;
    byte_ptr->date_last_modified = time_in_secs;
    byte_ptr->parent_byte_id = parent_byte_id;
    rc = bv_insert_byte(username, byte_ptr, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "Byte is not added due to: %s", errMSG);
        bv_error(temp);
        goto GOTO_FOR_BYTE_NAME;
    }
    else
    {
        bv_log("New byte is added of id: %llu", byte_ptr->id);
    }

    free(username);
    free(password);
    free(temp);
    free(byte_name);
    free(last_modifying_text);
    (byte_ptr) ? (free(byte_ptr)) : (0);
    (bv_data_ptr) ? (free(bv_data_ptr)) : (0);
}

void bv_ca_editbyte(int argc, char **argv)
{
    char *username = malloc(20 + 5); //max 20 chars
    char *password = malloc(20 + 5); //max 20 chars
    char *new_byte_name = malloc(200);
    char *prev_byte_name = malloc(200);
    bv_bool byte_name_will_be_changed = bv_false;
    unsigned long long new_parent_byte_id = 0;
    unsigned long long prev_parent_byte_id = 0;
    size_t byte_id = 0;
    bv_bool parent_byte_id_will_be_changed = bv_false;
    bv_user user_temp;
    char *errMSG;
    int rc;
    char *temp = malloc(300);
    size_t ln;
    int scc = 0;
    bv_node *temp_node_ptr = null;
    bv_bool user_response_sure = bv_false;
    size_t temp_ln = 0;
    bv_byte *ptr_byte_temp = null;
    size_t temp_id = 0;
    bv_bool temp_bool = bv_false;
    char *temp_char_ptr = null;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_BYTE_ID;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(new_byte_name);
            free(prev_byte_name);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(new_byte_name);
            free(prev_byte_name);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(new_byte_name);
            free(prev_byte_name);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(new_byte_name);
        free(prev_byte_name);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_BYTE_ID:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter byte id: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(new_byte_name);
            free(prev_byte_name);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_ID;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Byte id should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            rc = sscanf(temp, "%llu", &byte_id);
            if (rc == 0)
            {
                bv_error("Invalid byte id, please enter number");
                continue;
            }
        }
        sprintf(temp, "-i %llu", byte_id);
        rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMSG);
        if (rc == BV_OP_FAILED)
        {
            sprintf(temp, "Could not processed due to: %s", errMSG);
            bv_error(temp);
            free(username);
            free(password);
            free(temp);
            free(new_byte_name);
            free(prev_byte_name);
            return;
        }
        if (temp_node_ptr == NULL)
        {
            bv_error("The byte id does not exist");
            continue;
        }
        strcpy(prev_byte_name, ((bv_byte *)temp_node_ptr->value_ptr)->name);
        prev_parent_byte_id = ((bv_byte *)temp_node_ptr->value_ptr)->parent_byte_id;

        bv_free_byte_obj(temp_node_ptr->value_ptr);
        free(temp_node_ptr->value_ptr);
        free(temp_node_ptr);
        break;
    }

GOTO_FOR_NEW_BYTE_NAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter new byte name: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(new_byte_name);
            free(prev_byte_name);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_ID;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            byte_name_will_be_changed = bv_false;
            break;
        }
        else
        {
            if (!(ln >= 1 && ln <= 100))
            {
                bv_error("Byte name should contain at least 1 character and maximum 100 characters");
                continue;
            }
            else
            {
                strcpy(new_byte_name, temp);
                byte_name_will_be_changed = bv_true;
                break;
            }
        }
    }

GOTO_FOR_NEW_PARENT_ID:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter new parent id: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(new_byte_name);
            free(prev_byte_name);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_NEW_BYTE_NAME;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            parent_byte_id_will_be_changed = bv_false;
            break;
        }
        else
        {
            if (!(ln >= 1 && ln <= 20))
            {
                bv_error("Parent byte id should contain at least 1 character and maximum 20 characters");
                continue;
            }
            else
            {
                rc = sscanf(temp, "%llu", &new_parent_byte_id);
                if (rc == 0)
                {
                    bv_error("Invalid byte id, please enter number");
                    continue;
                }
            }
        }
        if (new_parent_byte_id == 0)
        {
            parent_byte_id_will_be_changed = bv_true;
            break;
        }
        else
        {
            sprintf(temp, "-i %llu", new_parent_byte_id);
            rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                sprintf(temp, "Could not processed due to: %s", errMSG);
                bv_error(temp);
                free(username);
                free(password);
                free(temp);
                free(new_byte_name);
                free(prev_byte_name);
                return;
            }
            if (temp_node_ptr == NULL)
            {
                bv_error("The parent byte id does not exist");
                continue;
            }
            bv_free_byte_obj(temp_node_ptr->value_ptr);
            free(temp_node_ptr->value_ptr);
            free(temp_node_ptr);

            if (prev_parent_byte_id == new_parent_byte_id)
            {
                bv_error("This parent byte is already parent of the current byte");
                continue;
            }
            if (new_parent_byte_id == byte_id)
            {
                bv_error("Parent byte can't same to current byte");
                continue;
            }

            temp_node_ptr = null;
            temp_id = new_parent_byte_id;
            temp_bool = bv_false;
            while (1)
            {
                if (temp_id == 0)
                {
                    temp_bool = bv_false;
                    break;
                }
                if (temp_id == byte_id)
                {
                    temp_bool = bv_true;
                    break;
                }
                sprintf(temp, "-i %llu", temp_id);
                rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMSG);
                if (rc != BV_OP_SUCCESS || temp_node_ptr == null)
                {
                    bv_error("Unexpected error occurred due to: %s", errMSG);
                    free(username);
                    free(password);
                    free(temp);
                    free(new_byte_name);
                    free(prev_byte_name);
                    return;
                }
                temp_id = ((bv_byte *)temp_node_ptr->value_ptr)->parent_byte_id;
                bv_free_byte_obj(temp_node_ptr->value_ptr);
                free(temp_node_ptr->value_ptr);
                free(temp_node_ptr);
                temp_node_ptr = null;
            }

            if (temp_bool)
            {
                bv_error("Child byte can't be the parent byte");
                continue;
            }
            else
            {
                parent_byte_id_will_be_changed = bv_true;
                break;
            }
        }
    }

    if (!byte_name_will_be_changed && !parent_byte_id_will_be_changed)
    {
        user_response_sure = bv_false;
        goto GOTO_FOR_END;
    }

GOTO_FOR_CONFIRM_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Are you sure, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(new_byte_name);
            free(prev_byte_name);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_NEW_PARENT_ID;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            user_response_sure = bv_false;
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                user_response_sure = bv_true;
            }
            else
            {
                user_response_sure = bv_false;
            }
            break;
        }
    }

GOTO_FOR_END:
    if (user_response_sure)
    {
        if (byte_name_will_be_changed)
        {
            rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_NAME, new_byte_name, &errMSG);
            if (rc != BV_OP_SUCCESS)
            {
                bv_error("The byte name is not changed due to: %s", errMSG);
            }
            else
            {
                bv_log("The byte name is changed");
                sprintf(temp, "%llu", time(NULL));
                rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED, temp, &errMSG);
                if (rc == BV_OP_SUCCESS)
                {
                    sprintf(temp, "Byte name is changed from \"");
                    temp_ln = 100 - 30 - 5;
                    if (strlen(prev_byte_name) <= temp_ln)
                    {
                        strncat(temp, prev_byte_name, temp_ln);
                    }
                    else
                    {
                        strncat(temp, prev_byte_name, temp_ln - 3);
                        strcat(temp, "...");
                    }
                    strcat(temp, "\"");
                    rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT, temp, &errMSG);
                    if (rc != BV_OP_SUCCESS)
                    {
                        bv_warn("Last modified info is not updated due to: %s", errMSG);
                    }
                }
                else
                {
                    bv_warn("Last modified info is not updated due to: %s", errMSG);
                }
            }
        }
        if (parent_byte_id_will_be_changed)
        {
            sprintf(temp, "%llu", new_parent_byte_id);
            rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_PARENT_BYTE_ID, temp, &errMSG);
            if (rc != BV_OP_SUCCESS)
            {
                bv_error("The parent id is not changed due to: %s", errMSG);
            }
            else
            {
                bv_log("The parent id is changed");
                sprintf(temp, "%llu", time(NULL));
                rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED, temp, &errMSG);
                if (rc == BV_OP_SUCCESS)
                {
                    sprintf(temp, "Byte parent id is changed from \"%llu\"", prev_parent_byte_id);
                    rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT, temp, &errMSG);
                    if (rc != BV_OP_SUCCESS)
                    {
                        bv_warn("Last modified info is not updated due to: %s", errMSG);
                    }
                }
                else
                {
                    bv_warn("Last modified info is not updated due to: %s", errMSG);
                }
            }
        }
    }
    else
    {
        bv_log("The byte is not modified");
    }

    free(username);
    free(password);
    free(temp);
    free(new_byte_name);
    free(prev_byte_name);
}

void bv_ca_showbyte(int argc, char **argv)
{
    char *username = malloc(20 + 5); //max 20 chars
    char *password = malloc(20 + 5); //max 20 chars
    bv_user user_temp;
    char *errMSG;
    int rc;
    char *temp = malloc(300);
    size_t ln;
    size_t i = 0;
    int scc = 0;
    char *byte_query_ptr = malloc(210);
    bv_node *temp_node = null;
    int data_includes = 1;
    bv_byte *temp_byte_ptr = null;
    bv_bool search_by_id = bv_false;
    bv_node *temp_node2 = null;
    bv_node *temp_node3 = null;
    size_t j = 0;
    bv_node **nodes_temp = null;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_BYTE_QUERY;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(byte_query_ptr);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(byte_query_ptr);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(byte_query_ptr);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(byte_query_ptr);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_BYTE_QUERY:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter query for byte: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 210, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(byte_query_ptr);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_QUERY;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 200))
        {
            bv_error("Query should contain at least 1 character and maximum 200 characters");
            continue;
        }
        else
        {
            strcpy(byte_query_ptr, temp);
            bv_trim(byte_query_ptr, temp);
            strcpy(byte_query_ptr, temp);
            if (!(bv_starts_with_str(byte_query_ptr, "-i ") || bv_starts_with_str(byte_query_ptr, "-n ") || !strcmpi(byte_query_ptr, "all")))
            {
                bv_error("Invalid query, enter -i id or all or -n name");
                continue;
            }
            else
            {
                break;
            }
        }
    }

    if (argc >= 3 && !strcmpi(argv[2], "/i"))
    {
        data_includes = 0;
    }
    else
    {
        data_includes = 1;
    }
    rc = bv_query_bytes(username, byte_query_ptr, -1, null, data_includes, &temp_node, &errMSG);
    if (bv_starts_with_str(byte_query_ptr, "-i"))
    {
        search_by_id = bv_true;
    }
    else
    {
        search_by_id = bv_false;
    }
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(byte_query_ptr);
        return;
    }
    if (temp_node == NULL)
    {
        bv_log("Total byte result: %llu", (size_t)0);
        free(username);
        free(password);
        free(temp);
        free(byte_query_ptr);
        return;
    }
    else
    {
        if (argc >= 3 && !strcmpi(argv[2], "/i"))
        {
            sprintf(temp, "-i");
        }
        else if (argc >= 3 && !strcmpi(argv[2], "/d"))
        {
            sprintf(temp, "-d");
        }
        else
        {
            sprintf(temp, "-d");
        }
        if (search_by_id)
        {
            bv_log("Total byte result: %llu", bv_ll_count(temp_node));
            bv_print_bytes("Bytes", temp_node, temp);
            fprintf(stdout, "\n");
            free(username);
            free(password);
            free(temp);
            free(byte_query_ptr);
            return;
        }
        else
        {
            nodes_temp = malloc(sizeof(bv_node *) * bv_ll_count(temp_node));
            j = 0;
            while (temp_node != null)
            {
                temp_byte_ptr = temp_node->value_ptr;
                if (temp_byte_ptr->parent_byte_id == 0)
                {
                    nodes_temp[j] = temp_node;
                    j++;
                    temp_node = temp_node->next;
                }
                else
                {
                    temp_node2 = temp_node;
                    temp_node = temp_node->next;
                    bv_free_byte_obj(temp_byte_ptr);
                    free(temp_byte_ptr);
                    free(temp_node2);
                }
            }
            if (j > 0)
            {
                temp_node = nodes_temp[0];
                for (i = 1; i < j; i++)
                {
                    temp_node->next = nodes_temp[i];
                    temp_node = temp_node->next;
                }
                temp_node->next = null;

                temp_node = nodes_temp[0]; //must
                bv_log("Total byte result: %llu", bv_ll_count(temp_node));
                bv_print_bytes("Bytes", temp_node, temp); //free the temp_node in bv_print_bytes() function
                fprintf(stdout, "\n");
            }
            else
            {
                bv_log("Total byte result: %llu", 0llu);
            }
            free(nodes_temp);
            free(username);
            free(password);
            free(temp);
            free(byte_query_ptr);
            return;
        }
    }

    free(username);
    free(password);
    free(temp);
    free(byte_query_ptr);
}

void bv_ca_deletebyte(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    char *temp = malloc(500);
    char *temp2 = malloc(200);
    char *byte_query_ptr = malloc(120);
    bv_user user_temp;
    char *errMSG;
    int rc;
    size_t ln;
    int scc = 0;
    size_t *byte_ids = null;
    size_t bytes_count = 0;
    bv_node *node_temp = null;
    bv_node *node_temp2 = null;
    char *back_up_file_path = malloc(MAX_PATH + 150);
    bv_bool is_recursive = bv_false;
    char *backup_file_folder_full_path = malloc(MAX_PATH + 100);
    char *temp_file_full_path = malloc(MAX_PATH + BV_TEMP_FILE_NAME_LENGTH + 100);
    FILE *temp_file = null;
    bv_bool delete_confirm = bv_false;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_BYTE_QUERY;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(byte_query_ptr);
        free(back_up_file_path);
        free(backup_file_folder_full_path);
        free(temp_file_full_path);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_BYTE_QUERY:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter byte query to delete: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_QUERY;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Query should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(byte_query_ptr, temp);
            bv_trim(byte_query_ptr, temp);
            strcpy(byte_query_ptr, temp);
            if (!(bv_starts_with_str(byte_query_ptr, "-i ") || !strcmpi(byte_query_ptr, "all")))
            {
                bv_error("Invalid query, enter -i id or all");
                continue;
            }
            else
            {
                break;
            }
        }
    }

    if (!strcmpi(byte_query_ptr, "all"))
    {
        node_temp = null;
        rc = bv_query_bytes(username, "all", -1, null, 0, &node_temp, &errMSG);
        if (rc != BV_OP_SUCCESS)
        {
            bv_error("Could not proceed due to: %s", errMSG);
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }
        if (node_temp == null)
        {
            bv_error("There is no byte to delete");
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }

        bytes_count = 0;
        while (node_temp != null)
        {
            if (((bv_byte *)node_temp->value_ptr)->parent_byte_id == 0)
            {
                if (bytes_count == 0)
                {
                    byte_ids = malloc(sizeof(size_t));
                    byte_ids[0] = ((bv_byte *)node_temp->value_ptr)->id;
                    bytes_count++;
                }
                else
                {
                    byte_ids = realloc(byte_ids, sizeof(size_t) * (bytes_count + 1));
                    byte_ids[bytes_count] = ((bv_byte *)node_temp->value_ptr)->id;
                    bytes_count++;
                }
            }
            node_temp2 = node_temp;
            node_temp = node_temp->next;
            bv_free_byte_obj(node_temp2->value_ptr);
            free(node_temp2->value_ptr);
            free(node_temp2);
        }
    }
    else
    {
        node_temp = null;
        rc = bv_query_bytes(username, byte_query_ptr, -1, null, 0, &node_temp, &errMSG);
        if (rc != BV_OP_SUCCESS)
        {
            bv_error("Could not proceed due to: %s", errMSG);
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }
        if (node_temp == null)
        {
            bv_error("This byte id does not exist or not a valid byte id");
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }
        bytes_count = 1;
        byte_ids = malloc(sizeof(size_t));
        byte_ids[0] = ((bv_byte *)node_temp->value_ptr)->id;

        bv_free_byte_obj(node_temp->value_ptr);
        free(node_temp->value_ptr);
        free(node_temp);
    }

GOTO_FOR_DELETE_CONFIRM_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Are you sure, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_DELETE_CONFIRM_DIALOG;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            delete_confirm = bv_false; //default value
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                delete_confirm = bv_true;
            }
            else
            {
                delete_confirm = bv_false;
            }
            break;
        }
    }

    if (!delete_confirm)
    {
        bv_log("Byte deleting is aborted");
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(byte_query_ptr);
        free(back_up_file_path);
        free(backup_file_folder_full_path);
        free(temp_file_full_path);
        return;
    }

    if (bv_starts_with_str(byte_query_ptr, "-i "))
    {
    GOTO_FOR_RECURSIVE_DIALOG:
        while (1)
        {
            bv_apply_input_string_key_color();
            fprintf(stdout, "Is delete recursively, type y or n ?: ");
            fseek(stdin, 0, SEEK_END);
            bv_apply_input_string_value_color();
            fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
            bv_apply_normal_color();
            ln = strlen(temp);
            (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
            ln = strlen(temp);
            scc = bv_process_special_command(temp);
            switch (scc)
            {
            case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
                free(username);
                free(password);
                free(temp);
                free(temp2);
                free(byte_query_ptr);
                free(back_up_file_path);
                free(backup_file_folder_full_path);
                free(temp_file_full_path);
                return;
                break;
            case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
                goto GOTO_FOR_RECURSIVE_DIALOG;
                break;
            default:
                break;
            }
            if (ln == 0)
            {
                is_recursive = bv_false; //default value
                break;
            }
            else
            {
                if (!strcmpi("y", temp) || !strcmpi("yes", temp))
                {
                    is_recursive = bv_true;
                }
                else
                {
                    is_recursive = bv_false;
                }
                break;
            }
        }
    }
    else if (!strcmpi(byte_query_ptr, "all"))
    {
        is_recursive = bv_true;
    }
    else
    {
        is_recursive = bv_false;
    }

    bv_get_backups_folder_path(back_up_file_path, MAX_PATH);
    if (!bv_is_existing_folder(back_up_file_path))
    {
        CreateDirectory(back_up_file_path, NULL);
    }
    strcat(back_up_file_path, "\\");
    strcat(back_up_file_path, username);
    if (!bv_is_existing_folder(back_up_file_path))
    {
        CreateDirectory(back_up_file_path, NULL);
    }
    bv_get_unique_backup_folder_name(temp2);
    strcat(back_up_file_path, "\\");
    strcat(back_up_file_path, temp2);
    if (!bv_is_existing_folder(back_up_file_path))
    {
        CreateDirectory(back_up_file_path, NULL);
    }
    strcpy(backup_file_folder_full_path, back_up_file_path);
    strcat(back_up_file_path, "\\");
    strcat(back_up_file_path, BV_FILE_BACKUP);
    strcat(back_up_file_path, BV_ENCRYPTED_FILE_EXTENSION);

    bv_get_unique_temp_file_path(temp_file_full_path, BV_TEMP_FILE_NAME_LENGTH);
    temp_file = fopen(temp_file_full_path, "w"); //It must open in 'w' mode, bcoz this will be used as text stream
    if (temp_file == null)
    {
        bv_error("Could not proceed due to: backup file is not opened");
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(byte_query_ptr);
        free(back_up_file_path);
        free(backup_file_folder_full_path);
        remove(temp_file_full_path); //it is must top of free the 'temp_file_full_path' buff
        free(temp_file_full_path);
        return;
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "Backing up: ");
    bv_apply_normal_color();

    bv_apply_input_string_value_color();
    sprintf(temp, "%s", "0.00%");
    bv_backup_back_to_chars = (short)strlen(temp);
    fprintf(stdout, "%s", temp);
    bv_apply_normal_color();

    rc = bv_backup_byte(temp_file, username, byte_ids, bytes_count, is_recursive, bv_backup_callback, 0x1, &errMSG);
    fprintf(stdout, "\n");
    fflush(temp_file);
    fclose(temp_file);

    if (rc != BV_OP_SUCCESS)
    {
        bv_warn("Some bytes are not backed up");
    }
    else
    {
        bv_log("Specified byte(s) are backed up");
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "Encrypting file: ");
    bv_apply_normal_color();

    bv_apply_input_string_value_color();
    sprintf(temp, "%s", "0.00%");
    bv_encrypt_file_back_to_chars = (short)strlen(temp);
    fprintf(stdout, "%s", temp);
    bv_apply_normal_color();

    rc = bv_encrypt_file(temp_file_full_path, back_up_file_path, bv_logged_in_user->enc_key, 0x1, bv_encrypt_file_callback, &errMSG);
    fprintf(stdout, "\n");
    remove(temp_file_full_path);

    if (rc != BV_OP_SUCCESS)
    {
        bv_warn("Backed up file could not be encrypted");
    }
    else
    {
        bv_log("Backed file is encrypted, to decrypt just type decfile");
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "deleting byte: ");
    bv_apply_normal_color();

    bv_apply_input_string_value_color();
    sprintf(temp, "%s", "0.00%");
    bv_delete_byte_back_to_chars = (short)strlen(temp);
    fprintf(stdout, "%s", temp);
    bv_apply_normal_color();

    rc = bv_delete_byte(username, byte_ids, bytes_count, is_recursive, bv_delete_byte_callback, 0x1, &errMSG);
    fprintf(stdout, "\n");
    free(byte_ids);

    if (rc != BV_OP_SUCCESS)
    {
        bv_warn("Some bytes are not deleted");
    }
    else
    {
        bv_log("Specified byte(s) are deleted");
    }

GOTO_FOR_FILE_LOCATION_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Would you like to open backup-file location, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            remove(temp_file_full_path); //it is must top of free the 'temp_file_full_path' buff
            free(temp_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_FILE_LOCATION_DIALOG;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                bv_open_path_in_explorer(backup_file_folder_full_path);
            }
            break;
        }
    }

    free(username);
    free(password);
    free(temp);
    free(temp2);
    free(byte_query_ptr);
    free(back_up_file_path);
    free(backup_file_folder_full_path);
    remove(temp_file_full_path); //it is must top of free the 'temp_file_full_path' buff
    free(temp_file_full_path);
}

void bv_ca_addbytedata(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    char *key = malloc(200);
    char *value = malloc(BV_BYTE_DATA_VALUE_LENGTH + 100);
    bv_user user_temp;
    char *errMSG;
    int rc;
    char *temp = malloc(BV_BYTE_DATA_VALUE_LENGTH + 200);
    size_t ln;
    int scc = 0;
    bv_node *temp_node_ptr = null;
    char *errMessage = null;
    size_t byte_id = 0;
    bv_data bv_data_temp;
    size_t i = 0;
    bv_byte *byte_tmp_ptr = null;
    bv_bool key_found = bv_false;
    size_t temp_ln = 0;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_BYTE_ID;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(key);
        free(value);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_BYTE_ID:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter byte id: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_ID;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Byte id should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            rc = sscanf(temp, "%llu", &byte_id);
            if (rc == 0)
            {
                bv_error("Invalid byte id, please enter number");
                continue;
            }
        }
        sprintf(temp, "-i %llu", byte_id);
        rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMessage);
        if (rc == BV_OP_FAILED)
        {
            sprintf(temp, "Could not processed due to: %s", errMessage);
            bv_error(temp);
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            return;
        }
        if (temp_node_ptr == NULL)
        {
            bv_error("The byte id does not exist");
            continue;
        }
        byte_tmp_ptr = temp_node_ptr->value_ptr;
        if (byte_tmp_ptr->data_count > 0)
        {
            fprintf(stdout, "Keys(");
            fprintf(stdout, "%s", byte_tmp_ptr->data[0].key);
            for (i = 1; i < byte_tmp_ptr->data_count; i++)
            {
                fprintf(stdout, ", %s", byte_tmp_ptr->data[i].key);
            }
            fprintf(stdout, ")\n");
        }
        else
        {
            fprintf(stdout, "keys()\n");
        }
        bv_free_byte_obj(byte_tmp_ptr);
        free(byte_tmp_ptr);
        free(temp_node_ptr);
        break;
    }

GOTO_FOR_KEY:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter data key: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_KEY;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Data key should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(key, temp);
            sprintf(temp, "-i %llu", byte_id);
            rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMessage);
            if (rc == BV_OP_FAILED || temp_node_ptr == null)
            {
                sprintf(temp, "Could not processed due to: %s", errMessage);
                bv_error(temp);
                free(username);
                free(password);
                free(temp);
                free(key);
                free(value);
                return;
            }
            key_found = bv_false;
            byte_tmp_ptr = temp_node_ptr->value_ptr;
            for (i = 0; i < byte_tmp_ptr->data_count; i++)
            {
                if (!strcmp(byte_tmp_ptr->data[i].key, key))
                {
                    key_found = bv_true;
                    break;
                }
            }
            bv_free_byte_obj(byte_tmp_ptr);
            free(byte_tmp_ptr);
            free(temp_node_ptr);
            if (key_found)
            {
                bv_error("The key is already exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_VALUE:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter data value: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, BV_BYTE_DATA_VALUE_LENGTH + 10, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_KEY;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= BV_BYTE_DATA_VALUE_LENGTH))
        {
            bv_error("Data value should contain at least 1 character and maximum %llu characters", BV_BYTE_DATA_VALUE_LENGTH);
            continue;
        }
        else
        {
            strcpy(value, temp);
            break;
        }
    }

    bv_data_temp.key = key;
    bv_data_temp.value = value;
    rc = bv_insert_byte_data(username, byte_id, &bv_data_temp, 1, &errMSG);
    if (rc == BV_OP_FAILED)
    {
        sprintf(temp, "Byte data is not added due to: %s", errMSG);
        bv_error(temp);
        goto GOTO_FOR_KEY;
    }
    else
    {
        bv_log("New byte data is added");
        sprintf(temp, "%llu", time(NULL));
        rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED, temp, &errMSG);
        if (rc == BV_OP_SUCCESS)
        {
            sprintf(temp, "New byte data is added named \"");
            temp_ln = 100 - 35 - 5;
            if (strlen(key) <= temp_ln)
            {
                strncat(temp, key, temp_ln);
            }
            else
            {
                strncat(temp, key, temp_ln - 3);
                strcat(temp, "...");
            }
            strcat(temp, "\"");
            rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT, temp, &errMSG);
            if (rc != BV_OP_SUCCESS)
            {
                bv_warn("Last modified info is not updated due to: %s", errMSG);
            }
        }
        else
        {
            bv_warn("Last modified info is not updated due to: %s", errMSG);
        }
        goto GOTO_FOR_KEY;
    }

    free(username);
    free(password);
    free(temp);
    free(key);
    free(value);
}

void bv_ca_show_me(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    bv_user user_temp;
    char *errMSG;
    char *temp = malloc(300);
    int rc;
    size_t ln;
    int scc = 0;
    bv_node *temp_node_ptr = null;
    size_t total_bytes = 0;
    size_t total_bytes_data = 0;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_RE_ENTER_PASSWORD;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

    if (execution_mode == BV_EXECUTION_MODE_NORMAL)
    {
        goto GOTO_FOR_END;
    }

GOTO_FOR_RE_ENTER_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter current password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_RE_ENTER_PASSWORD;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            if (bv_check_password(password, temp))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

GOTO_FOR_END:
    rc = bv_get_user_info(username, &user_temp, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        bv_error("Could not processed due to: %s", errMSG);
        free(username);
        free(password);
        free(temp);
        return;
    }

    rc = bv_query_bytes(username, "all", -1, null, 1, &temp_node_ptr, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        bv_error("Could not processed due to: %s", errMSG);
        free(username);
        free(password);
        free(temp);
        return;
    }
    if (temp_node_ptr == null)
    {
        total_bytes = 0;
        total_bytes_data = 0;
    }
    else
    {
        total_bytes = 0;
        total_bytes_data = 0;
        while (temp_node_ptr != null)
        {
            total_bytes++;
            total_bytes_data += ((bv_byte *)temp_node_ptr->value_ptr)->data_count;
            temp_node_ptr = temp_node_ptr->next;
        }
    }

    bv_print_user_info("UserInfo", &user_temp, total_bytes, total_bytes_data);

    free(username);
    free(password);
    free(temp);
}

void bv_ca_editbytedata(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    char *key = malloc(200);
    char *key_new_name = malloc(200);
    bv_bool key_value_will_be_changed = bv_false;
    bv_bool key_name_will_be_changed = bv_false;
    char *value = malloc(BV_BYTE_DATA_VALUE_LENGTH + 100);
    bv_user user_temp;
    char *errMSG = null;
    int rc;
    char *temp = malloc(BV_BYTE_DATA_VALUE_LENGTH + 200);
    size_t ln;
    int scc = 0;
    bv_node *temp_node_ptr = null;
    char *errMessage = null;
    size_t byte_id = 0;
    bv_data bv_data_temp;
    size_t i = 0;
    bv_byte *byte_tmp_ptr = null;
    bv_bool key_found = bv_false;
    char *key_prev_value_partial = malloc(100);
    size_t temp_ln = 0;
    char *key_updated_name = null;
    bv_bool user_response_sure = bv_false;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_BYTE_ID;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            free(value);
            free(key_new_name);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            free(key_prev_value_partial);
            free(key_new_name);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            free(key_prev_value_partial);
            free(key_new_name);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(key);
        free(value);
        free(key_prev_value_partial);
        free(key_new_name);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_BYTE_ID:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter byte id: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            free(key_prev_value_partial);
            free(key_new_name);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_ID;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Byte id should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            rc = sscanf(temp, "%llu", &byte_id);
            if (rc == 0)
            {
                bv_error("Invalid byte id, please enter number");
                continue;
            }
        }
        sprintf(temp, "-i %llu", byte_id);
        rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMessage);
        if (rc == BV_OP_FAILED)
        {
            sprintf(temp, "Could not processed due to: %s", errMessage);
            bv_error(temp);
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            free(key_prev_value_partial);
            free(key_new_name);
            return;
        }
        if (temp_node_ptr == NULL)
        {
            bv_error("The byte id does not exist");
            continue;
        }

        byte_tmp_ptr = temp_node_ptr->value_ptr;
        if (byte_tmp_ptr->data_count > 0)
        {
            fprintf(stdout, "Keys(");
            fprintf(stdout, "%s", byte_tmp_ptr->data[0].key);
            for (i = 1; i < byte_tmp_ptr->data_count; i++)
            {
                fprintf(stdout, ", %s", byte_tmp_ptr->data[i].key);
            }
            fprintf(stdout, ")\n");
        }
        else
        {
            fprintf(stdout, "keys()\n");
        }
        bv_free_byte_obj(byte_tmp_ptr);
        free(byte_tmp_ptr);
        free(temp_node_ptr);
        break;
    }

GOTO_FOR_KEY:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter data key: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            free(key_prev_value_partial);
            free(key_new_name);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_KEY;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Data key should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(key, temp);
            sprintf(temp, "-i %llu", byte_id);
            rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMessage);
            if (rc == BV_OP_FAILED || temp_node_ptr == null)
            {
                sprintf(temp, "Could not processed due to: %s", errMessage);
                bv_error(temp);
                free(username);
                free(password);
                free(temp);
                free(key);
                free(value);
                free(key_prev_value_partial);
                free(key_new_name);
                return;
            }
            key_found = bv_false;
            byte_tmp_ptr = temp_node_ptr->value_ptr;
            for (i = 0; i < byte_tmp_ptr->data_count; i++)
            {
                if (!strcmp(byte_tmp_ptr->data[i].key, key))
                {
                    key_found = bv_true;
                    strcpy(key_prev_value_partial, "");
                    strncat(key_prev_value_partial, byte_tmp_ptr->data[i].value, 50);
                    break;
                }
            }
            bv_free_byte_obj(byte_tmp_ptr);
            free(byte_tmp_ptr);
            free(temp_node_ptr);
            if (!key_found)
            {
                bv_error("The key does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_KEY_NEW_NAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter data key new name: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            free(key_prev_value_partial);
            free(key_new_name);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_KEY;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            key_name_will_be_changed = bv_false;
            break;
        }
        else
        {
            if (!(ln >= 1 && ln <= 100))
            {
                bv_error("Data key name should contain at least 1 character and maximum 100 characters");
                continue;
            }
            else
            {
                strcpy(key_new_name, temp);

                if (!strcmp(key, key_new_name))
                {
                    bv_error("The new key name is same to previous name");
                    continue;
                }

                sprintf(temp, "-i %llu", byte_id);
                rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMessage);
                if (rc == BV_OP_FAILED || temp_node_ptr == null)
                {
                    sprintf(temp, "Could not processed due to: %s", errMessage);
                    bv_error(temp);
                    free(username);
                    free(password);
                    free(temp);
                    free(key);
                    free(value);
                    free(key_prev_value_partial);
                    free(key_new_name);
                    return;
                }
                key_found = bv_false;
                byte_tmp_ptr = temp_node_ptr->value_ptr;
                for (i = 0; i < byte_tmp_ptr->data_count; i++)
                {
                    if (!strcmp(byte_tmp_ptr->data[i].key, key_new_name))
                    {
                        key_found = bv_true;
                        break;
                    }
                }
                bv_free_byte_obj(byte_tmp_ptr);
                free(byte_tmp_ptr);
                free(temp_node_ptr);
                if (key_found)
                {
                    bv_error("The key new name is already exist");
                    continue;
                }
                else
                {
                    key_name_will_be_changed = bv_true;
                    break;
                }
            }
        }
    }

GOTO_FOR_VALUE:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter data value: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, BV_BYTE_DATA_VALUE_LENGTH + 10, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            free(key_prev_value_partial);
            free(key_new_name);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_KEY_NEW_NAME;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            key_value_will_be_changed = bv_false;
            break;
        }
        else
        {
            if (!(ln >= 1 && ln <= BV_BYTE_DATA_VALUE_LENGTH))
            {
                bv_error("Data value should contain at least 1 character and maximum %llu characters", BV_BYTE_DATA_VALUE_LENGTH);
                continue;
            }
            else
            {
                strcpy(value, temp);
                key_value_will_be_changed = bv_true;
                break;
            }
        }
    }

    if (key_name_will_be_changed || key_value_will_be_changed)
    {
    GOTO_FOR_CONFIRM_DIALOG:
        while (1)
        {
            bv_apply_input_string_key_color();
            fprintf(stdout, "Are you sure, type y or n ?: ");
            fseek(stdin, 0, SEEK_END);
            bv_apply_input_string_value_color();
            fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
            bv_apply_normal_color();
            ln = strlen(temp);
            (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
            ln = strlen(temp);
            scc = bv_process_special_command(temp);
            switch (scc)
            {
            case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
                free(username);
                free(password);
                free(temp);
                free(key);
                free(value);
                free(key_prev_value_partial);
                free(key_new_name);
                return;
                break;
            case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
                goto GOTO_FOR_VALUE;
                break;
            default:
                break;
            }
            if (ln == 0)
            {
                user_response_sure = bv_false;
                break;
            }
            else
            {
                if (!strcmpi("y", temp) || !strcmpi("yes", temp))
                {
                    user_response_sure = bv_true;
                }
                else
                {
                    user_response_sure = bv_false;
                }
                break;
            }
        }

        if (!user_response_sure)
        {
            bv_log("Key name is not modified");
            bv_log("Byte data value is not modified");
            free(username);
            free(password);
            free(temp);
            free(key);
            free(value);
            free(key_prev_value_partial);
            free(key_new_name);
            return;
        }
    }

    key_updated_name = key;

    if (key_name_will_be_changed)
    {
        rc = bv_change_byte_data_key_name(username, byte_id, key, key_new_name, &errMessage);
        if (rc != BV_OP_SUCCESS)
        {
            bv_error("Key name is not changed due to: %s", errMessage);
        }
        else
        {
            key_updated_name = key_new_name;

            bv_log("Key name is changed");
            sprintf(temp, "%llu", time(NULL));
            rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED, temp, &errMSG);
            if (rc == BV_OP_SUCCESS)
            {
                temp_ln = (100 - 40 - 5 - 20 - 5) / 2;
                sprintf(temp, "Byte data key name is changed named \"");
                if (strlen(key_new_name) <= temp_ln)
                {
                    strncat(temp, key_new_name, temp_ln);
                }
                else
                {
                    strncat(temp, key_new_name, temp_ln - 3);
                    strcat(temp, "...");
                }
                strcat(temp, "\"");
                strcat(temp, " from prev name \"");
                if (strlen(key) <= temp_ln)
                {
                    strncat(temp, key, temp_ln);
                }
                else
                {
                    strncat(temp, key, temp_ln - 3);
                    strcat(temp, "...");
                }
                strcat(temp, "\"");
                rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT, temp, &errMSG);
                if (rc != BV_OP_SUCCESS)
                {
                    bv_warn("Last modified info is not updated due to: %s", errMSG);
                }
            }
            else
            {
                bv_warn("Last modified info is not updated due to: %s", errMSG);
            }
        }
    }
    else
    {
        bv_log("Key name is not modified");
    }

    if (key_value_will_be_changed)
    {
        bv_data_temp.key = key_updated_name;
        bv_data_temp.value = value;
        rc = bv_insert_byte_data(username, byte_id, &bv_data_temp, 1, &errMSG);
        if (rc != BV_OP_SUCCESS)
        {
            sprintf(temp, "Byte data value is not modified due to: %s", errMSG);
            bv_error(temp);
        }
        else
        {
            bv_log("Byte data value is modified");
            sprintf(temp, "%llu", time(NULL));
            rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED, temp, &errMSG);
            if (rc == BV_OP_SUCCESS)
            {
                temp_ln = (100 - 30 - 5 - 20 - 5) / 2;
                sprintf(temp, "Byte data value is modified named \"");
                if (strlen(key_updated_name) <= temp_ln)
                {
                    strncat(temp, key_updated_name, temp_ln);
                }
                else
                {
                    strncat(temp, key_updated_name, temp_ln - 3);
                    strcat(temp, "...");
                }
                strcat(temp, "\"");
                strcat(temp, " from prev value \"");
                if (strlen(key_prev_value_partial) <= temp_ln)
                {
                    strncat(temp, key_prev_value_partial, temp_ln);
                }
                else
                {
                    strncat(temp, key_prev_value_partial, temp_ln - 3);
                    strcat(temp, "...");
                }
                strcat(temp, "\"");
                rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT, temp, &errMSG);
                if (rc != BV_OP_SUCCESS)
                {
                    bv_warn("Last modified info is not updated due to: %s", errMSG);
                }
            }
            else
            {
                bv_warn("Last modified info is not updated due to: %s", errMSG);
            }
        }
    }
    else
    {
        bv_log("Byte data value is not modified");
    }

    free(username);
    free(password);
    free(temp);
    free(key);
    free(value);
    free(key_prev_value_partial);
    free(key_new_name);
}

void bv_ca_deletebytedata(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    char *key = malloc(200);
    bv_user user_temp;
    char *errMSG;
    int rc;
    char *temp = malloc(400);
    size_t ln;
    int scc = 0;
    bv_node *temp_node_ptr = null;
    size_t byte_id = 0;
    size_t i = 0;
    bv_byte *byte_tmp_ptr = null;
    bv_bool key_found = bv_false;
    bv_bool user_response = bv_false;
    size_t temp_ln = 0;
    char *key_prev_value_partial = malloc(100);

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_BYTE_ID;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        free(username);
        free(password);
        free(temp);
        free(key_prev_value_partial);
        free(key);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_BYTE_ID:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter byte id: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_ID;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Byte id should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            rc = sscanf(temp, "%llu", &byte_id);
            if (rc == 0)
            {
                bv_error("Invalid byte id, please enter number");
                continue;
            }
        }
        sprintf(temp, "-i %llu", byte_id);
        rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMSG);
        if (rc == BV_OP_FAILED)
        {
            sprintf(temp, "Could not processed due to: %s", errMSG);
            bv_error(temp);
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
        }
        if (temp_node_ptr == NULL)
        {
            bv_error("The byte id does not exist");
            continue;
        }

        byte_tmp_ptr = temp_node_ptr->value_ptr;
        if (byte_tmp_ptr->data_count > 0)
        {
            fprintf(stdout, "Keys(");
            fprintf(stdout, "%s", byte_tmp_ptr->data[0].key);
            for (i = 1; i < byte_tmp_ptr->data_count; i++)
            {
                fprintf(stdout, ", %s", byte_tmp_ptr->data[i].key);
            }
            fprintf(stdout, ")\n");
        }
        else
        {
            fprintf(stdout, "keys()\n");
        }
        bv_free_byte_obj(byte_tmp_ptr);
        free(byte_tmp_ptr);
        free(temp_node_ptr);
        break;
    }

GOTO_FOR_KEY:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter data key: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_KEY;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Data key should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(key, temp);
            sprintf(temp, "-i %llu", byte_id);
            rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMSG);
            if (rc == BV_OP_FAILED || temp_node_ptr == null)
            {
                sprintf(temp, "Could not processed due to: %s", errMSG);
                bv_error(temp);
                free(username);
                free(password);
                free(temp);
                free(key_prev_value_partial);
                free(key);
                return;
            }
            key_found = bv_false;
            byte_tmp_ptr = temp_node_ptr->value_ptr;
            for (i = 0; i < byte_tmp_ptr->data_count; i++)
            {
                if (!strcmp(byte_tmp_ptr->data[i].key, key))
                {
                    key_found = bv_true;
                    strcpy(key_prev_value_partial, "");
                    strncat(key_prev_value_partial, byte_tmp_ptr->data[i].value, 50);
                    break;
                }
            }
            bv_free_byte_obj(byte_tmp_ptr);
            free(byte_tmp_ptr);
            free(temp_node_ptr);
            if (!key_found)
            {
                bv_error("The key does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_CONFIRM_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Are you sure to delete? type y or n: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_KEY;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            user_response = bv_false;
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                user_response = bv_true;
                break;
            }
            else
            {
                user_response = bv_false;
                break;
            }
        }
    }

    if (user_response)
    {
        rc = bv_delete_bytedata(username, byte_id, key, &errMSG);
        if (rc == BV_OP_FAILED)
        {
            sprintf(temp, "Byte data is not deleted due to: %s", errMSG);
            bv_error(temp);
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
        }
        else
        {
            bv_log("New byte data is deleted");
            sprintf(temp, "%llu", time(NULL));
            rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED, temp, &errMSG);
            if (rc == BV_OP_SUCCESS)
            {
                temp_ln = (100 - 30 - 5 - 20 - 5) / 2;
                sprintf(temp, "Byte data is deleted named \"");
                if (strlen(key) <= temp_ln)
                {
                    strncat(temp, key, temp_ln);
                }
                else
                {
                    strncat(temp, key, temp_ln - 3);
                    strcat(temp, "...");
                }
                strcat(temp, "\"");
                strcat(temp, " had value \"");
                if (strlen(key_prev_value_partial) <= temp_ln)
                {
                    strncat(temp, key_prev_value_partial, temp_ln);
                }
                else
                {
                    strncat(temp, key_prev_value_partial, temp_ln - 3);
                    strcat(temp, "...");
                }
                strcat(temp, "\"");
                rc = bv_edit_byte_info(username, byte_id, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT, temp, &errMSG);
                if (rc != BV_OP_SUCCESS)
                {
                    bv_warn("Last modified info is not updated due to: %s", errMSG);
                }
            }
            else
            {
                bv_warn("Last modified info is not updated due to: %s", errMSG);
            }
            free(username);
            free(password);
            free(temp);
            free(key_prev_value_partial);
            free(key);
            return;
        }
    }
    else
    {
        bv_log("Byte data is not deleted");
    }

    free(username);
    free(password);
    free(temp);
    free(key_prev_value_partial);
    free(key);
}

void bv_ca_backup(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    char *temp = malloc(500);
    char *temp2 = malloc(200);
    char *byte_query_ptr = malloc(120);
    bv_user user_temp;
    char *errMSG;
    int rc;
    size_t ln;
    int scc = 0;
    size_t *byte_ids = null;
    size_t bytes_count = 0;
    bv_node *node_temp = null;
    bv_node *node_temp2 = null;
    char *back_up_file_path = malloc(MAX_PATH + 150);
    bv_bool is_recursive = bv_false;
    char *backup_file_folder_full_path = malloc(MAX_PATH + 100);
    char *temp_file_full_path = malloc(MAX_PATH + BV_TEMP_FILE_NAME_LENGTH + 100);
    FILE *temp_file = null;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_BYTE_QUERY;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(byte_query_ptr);
        free(back_up_file_path);
        free(backup_file_folder_full_path);
        free(temp_file_full_path);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_BYTE_QUERY:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter byte query to backup: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_BYTE_QUERY;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Query should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            strcpy(byte_query_ptr, temp);
            bv_trim(byte_query_ptr, temp);
            strcpy(byte_query_ptr, temp);
            if (!(bv_starts_with_str(byte_query_ptr, "-i ") || !strcmpi(byte_query_ptr, "all")))
            {
                bv_error("Invalid query, enter -i id or all");
                continue;
            }
            else
            {
                break;
            }
        }
    }

    if (!strcmpi(byte_query_ptr, "all"))
    {
        node_temp = null;
        rc = bv_query_bytes(username, "all", -1, null, 0, &node_temp, &errMSG);
        if (rc != BV_OP_SUCCESS)
        {
            bv_error("Could not proceed due to: %s", errMSG);
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }
        if (node_temp == null)
        {
            bv_error("There is no byte to backup");
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }

        bytes_count = 0;
        while (node_temp != null)
        {
            if (((bv_byte *)node_temp->value_ptr)->parent_byte_id == 0)
            {
                if (bytes_count == 0)
                {
                    byte_ids = malloc(sizeof(size_t));
                    byte_ids[0] = ((bv_byte *)node_temp->value_ptr)->id;
                    bytes_count++;
                }
                else
                {
                    byte_ids = realloc(byte_ids, sizeof(size_t) * (bytes_count + 1));
                    byte_ids[bytes_count] = ((bv_byte *)node_temp->value_ptr)->id;
                    bytes_count++;
                }
            }
            node_temp2 = node_temp;
            node_temp = node_temp->next;
            bv_free_byte_obj(node_temp2->value_ptr);
            free(node_temp2->value_ptr);
            free(node_temp2);
        }
    }
    else
    {
        node_temp = null;
        rc = bv_query_bytes(username, byte_query_ptr, -1, null, 0, &node_temp, &errMSG);
        if (rc != BV_OP_SUCCESS)
        {
            bv_error("Could not proceed due to: %s", errMSG);
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }
        if (node_temp == null)
        {
            bv_error("This byte id does not exist or not a valid byte id");
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            free(temp_file_full_path);
            return;
        }
        bytes_count = 1;
        byte_ids = malloc(sizeof(size_t));
        byte_ids[0] = ((bv_byte *)node_temp->value_ptr)->id;

        bv_free_byte_obj(node_temp->value_ptr);
        free(node_temp->value_ptr);
        free(node_temp);
    }

    if (bv_starts_with_str(byte_query_ptr, "-i "))
    {
    GOTO_FOR_RECURSIVE_DIALOG:
        while (1)
        {
            bv_apply_input_string_key_color();
            fprintf(stdout, "Is backup recursively, type y or n ?(default y): ");
            fseek(stdin, 0, SEEK_END);
            bv_apply_input_string_value_color();
            fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
            bv_apply_normal_color();
            ln = strlen(temp);
            (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
            ln = strlen(temp);
            scc = bv_process_special_command(temp);
            switch (scc)
            {
            case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
                free(username);
                free(password);
                free(temp);
                free(temp2);
                free(byte_query_ptr);
                free(back_up_file_path);
                free(backup_file_folder_full_path);
                free(temp_file_full_path);
                return;
                break;
            case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
                goto GOTO_FOR_RECURSIVE_DIALOG;
                break;
            default:
                break;
            }
            if (ln == 0)
            {
                is_recursive = bv_true; //default value
                break;
            }
            else
            {
                if (!strcmpi("y", temp) || !strcmpi("yes", temp))
                {
                    is_recursive = bv_true;
                }
                else
                {
                    is_recursive = bv_false;
                }
                break;
            }
        }
    }
    else if (!strcmpi(byte_query_ptr, "all"))
    {
        is_recursive = bv_true;
    }
    else
    {
        is_recursive = bv_false;
    }

    bv_get_backups_folder_path(back_up_file_path, MAX_PATH);
    if (!bv_is_existing_folder(back_up_file_path))
    {
        CreateDirectory(back_up_file_path, NULL);
    }
    strcat(back_up_file_path, "\\");
    strcat(back_up_file_path, username);
    if (!bv_is_existing_folder(back_up_file_path))
    {
        CreateDirectory(back_up_file_path, NULL);
    }
    bv_get_unique_backup_folder_name(temp2);
    strcat(back_up_file_path, "\\");
    strcat(back_up_file_path, temp2);
    if (!bv_is_existing_folder(back_up_file_path))
    {
        CreateDirectory(back_up_file_path, NULL);
    }
    strcpy(backup_file_folder_full_path, back_up_file_path);
    strcat(back_up_file_path, "\\");
    strcat(back_up_file_path, BV_FILE_BACKUP);
    strcat(back_up_file_path, BV_ENCRYPTED_FILE_EXTENSION);

    bv_get_unique_temp_file_path(temp_file_full_path, BV_TEMP_FILE_NAME_LENGTH);
    temp_file = fopen(temp_file_full_path, "w"); //It must open in 'w' mode, bcoz this will be used as text stream
    if (temp_file == null)
    {
        bv_error("Could not proceed due to: backup file is not opened");
        free(username);
        free(password);
        free(temp);
        free(temp2);
        free(byte_query_ptr);
        free(back_up_file_path);
        free(backup_file_folder_full_path);
        remove(temp_file_full_path); //it is must top of free the 'temp_file_full_path' buff
        free(temp_file_full_path);
        return;
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "Backing up: ");
    bv_apply_normal_color();

    bv_apply_input_string_value_color();
    sprintf(temp, "%s", "0.00%");
    bv_backup_back_to_chars = (short)strlen(temp);
    fprintf(stdout, "%s", temp);
    bv_apply_normal_color();

    rc = bv_backup_byte(temp_file, username, byte_ids, bytes_count, is_recursive, bv_backup_callback, 0x1, &errMSG);
    fprintf(stdout, "\n");
    fclose(temp_file);
    free(byte_ids);

    if (rc != BV_OP_SUCCESS)
    {
        bv_warn("Some bytes are not backed up");
    }
    else
    {
        bv_log("Specified byte(s) are backed up");
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "Encrypting file: ");
    bv_apply_normal_color();

    bv_apply_input_string_value_color();
    sprintf(temp, "%s", "0.00%");
    bv_encrypt_file_back_to_chars = (short)strlen(temp);
    fprintf(stdout, "%s", temp);
    bv_apply_normal_color();

    rc = bv_encrypt_file(temp_file_full_path, back_up_file_path, bv_logged_in_user->enc_key, 0x1, bv_encrypt_file_callback, &errMSG);
    fprintf(stdout, "\n");
    remove(temp_file_full_path);

    if (rc != BV_OP_SUCCESS)
    {
        bv_warn("Backed up file could not be encrypted");
    }
    else
    {
        bv_log("Backed file is encrypted, to decrypt just type %s", BV_COMMAND_DECFILE_1);
    }

GOTO_FOR_FILE_LOCATION_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Would you like to open backup-file location, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(temp2);
            free(byte_query_ptr);
            free(back_up_file_path);
            free(backup_file_folder_full_path);
            remove(temp_file_full_path); //it is must top of free the 'temp_file_full_path' buff
            free(temp_file_full_path);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_FILE_LOCATION_DIALOG;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                bv_open_path_in_explorer(backup_file_folder_full_path);
            }
            break;
        }
    }

    free(username);
    free(password);
    free(temp);
    free(temp2);
    free(byte_query_ptr);
    free(back_up_file_path);
    free(backup_file_folder_full_path);
    remove(temp_file_full_path); //it is must top of free the 'temp_file_full_path' buff
    free(temp_file_full_path);
}

static void bv_backup_callback(int code, size_t total_backed_up, size_t total_bytes)
{
    if (code == 0x1)
    {
        short i = 0;
        char temp[10];
        double percent = 0.00;

        for (i = 0; i < bv_backup_back_to_chars; i++)
        {
            fprintf(stdout, "\b");
        }
        if (total_bytes == 0)
        {
            percent = 100.00;
        }
        else
        {
            percent = ((double)total_backed_up / (double)total_bytes) * (double)100;
        }
        sprintf(temp, "%.2lf%s", percent, "%");
        bv_backup_back_to_chars = (short)strlen(temp);
        fprintf(stdout, "%s", temp);
    }
}

static void bv_encrypt_file_callback(int code, size_t bytes_encrypted, size_t total_bytes)
{
    if (code == 0x1)
    {
        short i = 0;
        char temp[10];
        double percent = 0.00;

        for (i = 0; i < bv_encrypt_file_back_to_chars; i++)
        {
            fprintf(stdout, "\b");
        }
        if (total_bytes == 0)
        {
            percent = 100.00;
        }
        else
        {
            percent = ((double)bytes_encrypted / (double)total_bytes) * (double)100;
        }
        sprintf(temp, "%.2lf%s", percent, "%");
        bv_encrypt_file_back_to_chars = (short)strlen(temp);
        fprintf(stdout, "%s", temp);
    }
}

static void bv_decrypt_file_callback(int code, size_t bytes_decrypted, size_t total_bytes)
{
    if (code == 0x1)
    {
        short i = 0;
        char temp[10];
        double percent = 0.00;

        for (i = 0; i < bv_decrypt_file_back_to_chars; i++)
        {
            fprintf(stdout, "\b");
        }
        if (total_bytes == 0)
        {
            percent = 100.0;
        }
        else
        {
            percent = ((double)bytes_decrypted / (double)total_bytes) * (double)100;
        }
        sprintf(temp, "%.2lf%s", percent, "%");
        bv_decrypt_file_back_to_chars = (short)strlen(temp);
        fprintf(stdout, "%s", temp);
    }
}

static void bv_delete_byte_callback(int code, size_t bytes_deleted, size_t total_bytes)
{
    if (code == 0x1)
    {
        short i = 0;
        char temp[10];
        double percent = 0.00;

        for (i = 0; i < bv_delete_byte_back_to_chars; i++)
        {
            fprintf(stdout, "\b");
        }
        if (total_bytes == 0)
        {
            percent = 100.0;
        }
        else
        {
            percent = ((double)bytes_deleted / (double)total_bytes) * (double)100;
        }
        sprintf(temp, "%.2lf%s", percent, "%");
        bv_delete_byte_back_to_chars = (short)strlen(temp);
        fprintf(stdout, "%s", temp);
    }
}

void bv_ca_change_user_info(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *password = malloc(20 + 5);
    char *temp = malloc(500);
    char *user_info_column_value = malloc(500);
    char *column_name = null;
    int flags = -1;
    size_t column_max_ln = 0;
    size_t column_min_ln = 0;
    char *column_field_name = null;
    char *column_prompt_string = null;
    bv_user user_temp;
    char *errMSG;
    int rc;
    size_t ln;
    int scc = 0;
    bv_bool user_response_sure = bv_false;
    bv_bool temp_bool = bv_false;

    if (argc >= 3)
    {
        if (!strcmpi(argv[2], BV_CONSTANT_COMMAND_CHANGE_PASSWORD))
        {
            column_name = "Password";
            column_field_name = BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_PASSWORD;
            column_max_ln = 20;
            column_min_ln = 5;
            flags = 0x1;
            column_prompt_string = "Enter new password: ";
        }
        else if (!strcmpi(argv[2], BV_CONSTANT_COMMAND_CHANGE_HINTS))
        {
            column_name = "Hints";
            column_field_name = BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_HINTS;
            flags = 0x2;
            column_max_ln = 100;
            column_min_ln = 1;
            column_prompt_string = "Enter new hints: ";
        }
        else if (!strcmpi(argv[2], BV_CONSTANT_COMMAND_CHANGE_SEC_QUES))
        {
            column_name = "Security question";
            column_field_name = BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES;
            flags = 0x3;
            column_max_ln = 100;
            column_min_ln = 1;
            column_prompt_string = "Enter new security question: ";
        }
        else if (!strcmpi(argv[2], BV_CONSTANT_COMMAND_CHANGE_SEC_QUES_ANS))
        {
            column_name = "Security question answer";
            column_field_name = BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES_ANS;
            flags = 0x4;
            column_max_ln = 100;
            column_min_ln = 1;
            column_prompt_string = "Enter new security question answer: ";
        }
        else
        {
            bv_error("Please specify /p for password or /h for hints or /s for security question or /a for security question answer");
            free(username);
            free(password);
            free(temp);
            free(user_info_column_value);
            return;
        }
    }
    else
    {
        bv_error("Please specify /p for password or /h for hints or /s for security question or /a for security question answer");
        free(username);
        free(password);
        free(temp);
        free(user_info_column_value);
        return;
    }

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_RE_ENTER_PASSWORD;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            free(user_info_column_value);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(user_info_column_value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(user_info_column_value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(user_info_column_value);
        free(temp);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

    if (execution_mode == BV_EXECUTION_MODE_NORMAL)
    {
        goto GOTO_FOR_COLUMN_VALUE;
    }

GOTO_FOR_RE_ENTER_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter current password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(user_info_column_value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_RE_ENTER_PASSWORD;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            if (bv_check_password(password, temp))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

GOTO_FOR_COLUMN_VALUE:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, column_prompt_string);
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        if (flags == 0x1)
        {
            bv_read_password_from_console(temp, 110, 1);
        }
        else
        {
            fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        }
        bv_apply_normal_color();
        ln = strlen(temp);
        if (flags == 0x2 || flags == 0x3 || flags == 0x4)
        {
            (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
            ln = strlen(temp);
        }
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(user_info_column_value);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_COLUMN_VALUE;
            break;
        default:
            break;
        }
        if (!(ln >= column_min_ln && ln <= column_max_ln))
        {
            bv_error("%s should contain at least %llu character(s) and maximum %llu characters", column_name, column_min_ln, column_max_ln);
            continue;
        }
        else
        {
            strcpy(user_info_column_value, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc != BV_OP_SUCCESS)
            {
                free(username);
                free(password);
                free(temp);
                free(user_info_column_value);
                bv_error("Unexpected error occurred due to: %s", errMSG);
                return;
            }
            switch (flags)
            {
            case 0x1:
                temp_bool = (bv_bool)(!strcmp(user_info_column_value, user_temp.password));
                break;
            case 0x2:
                temp_bool = (bv_bool)(!strcmp(user_info_column_value, user_temp.hints));
                break;
            case 0x3:
                temp_bool = (bv_bool)(!strcmp(user_info_column_value, user_temp.sec_ques));
                break;
            case 0x4:
                temp_bool = (bv_bool)(!strcmp(user_info_column_value, user_temp.sec_ques_ans));
                break;
            default:
                temp_bool = bv_true;
            }
            if (temp_bool)
            {
                bv_error("New value is same as the current value", errMSG);
                continue;
            }
            else
            {
                break;
            }
        }
    }

    if (flags == 0x1)
    {
    GOTO_FOR_REPEAT_NEW_PASSWORD:
        while (1)
        {
            bv_apply_input_string_key_color();
            fprintf(stdout, "Repeat new password: ");
            fseek(stdin, 0, SEEK_END);
            bv_apply_input_string_value_color();
            bv_read_password_from_console(temp, 110, 1);
            bv_apply_normal_color();
            scc = bv_process_special_command(temp);
            switch (scc)
            {
            case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
                free(username);
                free(password);
                free(temp);
                free(user_info_column_value);
                return;
                break;
            case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
                goto GOTO_FOR_COLUMN_VALUE;
                break;
            default:
                break;
            }
            if (strcmp(temp, user_info_column_value))
            {
                bv_error("Password does not match with previous one");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_CONFIRM_DIALOG:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Are you sure, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            free(user_info_column_value);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_COLUMN_VALUE;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            user_response_sure = bv_false;
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                user_response_sure = bv_true;
            }
            else
            {
                user_response_sure = bv_false;
            }
            break;
        }
    }

    if (user_response_sure)
    {
        rc = bv_edit_user_info(username, column_field_name, user_info_column_value, &errMSG);
        if (rc != BV_OP_SUCCESS)
        {
            bv_error("%s could not be modified due to: %s", column_name, errMSG);
            free(username);
            free(password);
            free(temp);
            free(user_info_column_value);
            return;
        }
        else
        {
            bv_log("%s is modified", column_name);
        }

        if (execution_mode == BV_EXECUTION_MODE_SHELL)
        {
            if (bv_logged_in_user != NULL)
            {
                free(bv_logged_in_user);
            }
            bv_logged_in_user = malloc(sizeof(bv_user));
            rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
            if (rc != BV_OP_SUCCESS)
            {
                sprintf(temp, "%s", errMSG);
                bv_error(temp);
                free(username);
                free(password);
                free(user_info_column_value);
                free(temp);
                if (bv_logged_in_user != NULL)
                {
                    free(bv_logged_in_user);
                }
                bv_logged_in_user = null;
                return;
            }
            bv_open_db_connections();
        }
    }
    else
    {
        bv_log("%s is not modified", column_name);
    }

    free(username);
    free(password);
    free(temp);
    free(user_info_column_value);
}

void bv_ca_forgot_password(int argc, char **argv)
{
    char *username = malloc(20 + 5);
    char *temp = malloc(500);
    char *new_password = malloc(20 + 5);
    bv_user user_temp;
    char *errMSG;
    int rc;
    size_t ln;
    int scc = 0;
    bv_bool user_response_positive = bv_false;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            goto GOTO_FOR_AFTER_USERNAME;
        }
        else
        {
            goto GOTO_FOR_USERNAME;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(new_password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_AFTER_USERNAME:
    rc = bv_get_user_info(username, &user_temp, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        bv_error("Error occurred due to: %s", errMSG);
        free(username);
        free(new_password);
        free(temp);
        return;
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "Your hints is: ");
    bv_apply_input_string_value_color();
    fprintf(stdout, "%s\n", user_temp.hints);
    bv_apply_normal_color();

GOTO_FOR_CONFIRM_DIALOG_1:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Did you remember your password, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(new_password);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_CONFIRM_DIALOG_1;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            user_response_positive = bv_false;
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                user_response_positive = bv_true;
            }
            else
            {
                user_response_positive = bv_false;
            }
            break;
        }
    }

    if (user_response_positive)
    {
        free(username);
        free(new_password);
        free(temp);
        bv_log("Password is recovered from hints");
        return;
    }

    bv_apply_input_string_key_color();
    fprintf(stdout, "Answer to your security question: ");
    bv_apply_input_string_value_color();
    fprintf(stdout, "%s\n", user_temp.sec_ques);
    bv_apply_normal_color();

GOTO_FOR_SEC_QUES_ANS:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Your answer: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(new_password);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_SEC_QUES_ANS;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 100))
        {
            bv_error("Security question answer should contain at least 1 character and maximum 100 characters");
            continue;
        }
        else
        {
            if (!strcmpi(temp, user_temp.sec_ques_ans))
            {
                break;
            }
            else
            {
                bv_error("Answer is not correct");
                continue;
            }
        }
    }

    bv_log("Security question answer is correct");

GOTO_FOR_NEW_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter new password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(new_password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_NEW_PASSWORD;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(new_password, temp);
            if (!strcmp(new_password, user_temp.password))
            {
                bv_error("New password is same as the current password");
                continue;
            }
            break;
        }
    }

GOTO_FOR_REPEAT_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Repeat password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(new_password);
            free(temp);
            return;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_NEW_PASSWORD;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            if (bv_check_password(new_password, temp))
            {
                break;
            }
            else
            {
                bv_error("Password does not match with previous one");
                continue;
            }
        }
    }

GOTO_FOR_CONFIRM_DIALOG_2:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Are you sure to reset password, type y or n ?: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(new_password);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_NEW_PASSWORD;
            break;
        default:
            break;
        }
        if (ln == 0)
        {
            user_response_positive = bv_false;
            break;
        }
        else
        {
            if (!strcmpi("y", temp) || !strcmpi("yes", temp))
            {
                user_response_positive = bv_true;
            }
            else
            {
                user_response_positive = bv_false;
            }
            break;
        }
    }

    if (user_response_positive)
    {
        rc = bv_edit_user_info(username, BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_PASSWORD, new_password, &errMSG);
        if (rc != BV_OP_SUCCESS)
        {
            free(username);
            free(new_password);
            free(temp);
            bv_error("Password is not changed due to: %s", errMSG);
            return;
        }
        else
        {
            bv_log("Password is recovered from security question");
            if (execution_mode == BV_EXECUTION_MODE_SHELL && bv_logged_in_user != null)
            {
                if (bv_logged_in_user != NULL)
                {
                    free(bv_logged_in_user);
                }
                bv_logged_in_user = malloc(sizeof(bv_user));
                rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
                if (rc != BV_OP_SUCCESS)
                {
                    sprintf(temp, "%s", errMSG);
                    bv_error(temp);
                    free(username);
                    free(new_password);
                    free(temp);
                    free(temp);
                    if (bv_logged_in_user != NULL)
                    {
                        free(bv_logged_in_user);
                    }
                    bv_logged_in_user = null;
                    return;
                }
                bv_open_db_connections();
            }
        }
    }
    else
    {
        bv_log("Password is not recovered");
    }

    free(username);
    free(new_password);
    free(temp);
}

void bv_ca_start_user_session()
{
    char cache_root_path[MAX_PATH];
    char session_file_path[MAX_PATH + 50];
    char temp[100];

    if (bv_logged_in_user == null)
    {
        bv_error("Please log in to start session");
        return;
    }
    bv_get_app_cache_folder_path(cache_root_path, MAX_PATH);
    sprintf(session_file_path, "%s\\%s", cache_root_path, BV_FILE_USER_SESSION);
    FILE *session_file = null;
    session_file = fopen(session_file_path, "w");
    if (session_file == null)
    {
        bv_error("Could not start session");
        return;
    }
    bv_encrypt_text(bv_logged_in_user->username, BV_PRIMARY_KEY, temp);
    fprintf(session_file, "%s", temp);
    fclose(session_file);
    bv_log("Session started as %s", bv_logged_in_user->username);
}

void bv_ca_print_top_parent_byte_ids(int argc, char **argv)
{
    char *username = malloc(20 + 5); //max 20 chars
    char *password = malloc(20 + 5); //max 20 chars
    bv_user user_temp;
    char *errMSG;
    int rc;
    char *temp = malloc(300);
    size_t ln;
    int scc = 0;
    bv_byte *byte_ptr = null;
    bv_node *temp_node_ptr = null;
    bv_node *temp_node_ptr2 = null;
    size_t i = 0;
    bv_node *head_node = null;

    if (execution_mode == BV_EXECUTION_MODE_SHELL)
    {
        if (bv_logged_in_user != null)
        {
            strcpy(username, bv_logged_in_user->username);
            strcpy(password, bv_logged_in_user->password);
            goto GOTO_FOR_PRINT_TPB_IDS;
        }
        else
        {
            bv_error("Please log in to access database, type .login or .adduser");
            free(username);
            free(password);
            free(temp);
            return;
        }
    }

GOTO_FOR_USERNAME:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter username: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        fgets(temp, 110, stdin); //fgets function reads and stores '\n' char, but gets function just only reads, but does not stores in buffer.
        bv_apply_normal_color();
        ln = strlen(temp);
        (ln >= 1 && temp[ln - 1] == '\n') ? (temp[ln - 1] = '\0') : (0);
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 1 && ln <= 20))
        {
            bv_error("Username should contain at least 1 character and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(username, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_FAILED)
            {
                bv_error("Username does not exist");
                continue;
            }
            else
            {
                break;
            }
        }
    }

GOTO_FOR_PASSWORD:
    while (1)
    {
        bv_apply_input_string_key_color();
        fprintf(stdout, "Enter password: ");
        fseek(stdin, 0, SEEK_END);
        bv_apply_input_string_value_color();
        bv_read_password_from_console(temp, 110, 1);
        bv_apply_normal_color();
        ln = strlen(temp);
        scc = bv_process_special_command(temp);
        switch (scc)
        {
        case BV_SPECIAL_COMMAND_ACTION_EXIT_CODE:
            free(username);
            free(password);
            free(temp);
            return;
            break;
        case BV_SPECIAL_COMMAND_ACTION_PREV_CODE:
            goto GOTO_FOR_USERNAME;
            break;
        default:
            break;
        }
        if (!(ln >= 5 && ln <= 20))
        {
            bv_error("Password should contain at least 5 characters and maximum 20 characters");
            continue;
        }
        else
        {
            strcpy(password, temp);
            rc = bv_get_user_info(username, &user_temp, &errMSG);
            if (rc == BV_OP_SUCCESS && bv_check_password(user_temp.password, password))
            {
                break;
            }
            else
            {
                bv_error("Password is not correct");
                continue;
            }
        }
    }

    if (bv_logged_in_user != NULL)
    {
        free(bv_logged_in_user);
    }
    bv_logged_in_user = malloc(sizeof(bv_user));
    rc = bv_get_user_info(username, bv_logged_in_user, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        if (bv_logged_in_user != NULL)
        {
            free(bv_logged_in_user);
        }
        bv_logged_in_user = null;
        return;
    }
    bv_open_db_connections();

GOTO_FOR_PRINT_TPB_IDS:
    rc = bv_query_bytes(username, "all", -1, null, 0, &temp_node_ptr, &errMSG);
    if (rc != BV_OP_SUCCESS)
    {
        sprintf(temp, "%s", errMSG);
        bv_error(temp);
        free(username);
        free(password);
        free(temp);
        return;
    }

    fprintf(stdout, "ByteVault\n");
    bv_print_char(BV_L_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, BV_SYM_COUNT);
    fprintf(stdout, "TopParentByteIds\n");

    head_node = null;
    while (temp_node_ptr != null)
    {
        byte_ptr = temp_node_ptr->value_ptr;
        temp_node_ptr2 = temp_node_ptr->next;
        if (byte_ptr->parent_byte_id == 0)
        {
            if (head_node == null)
            {
                head_node = temp_node_ptr;
                temp_node_ptr->next = null;
            }
            else
            {
                bv_ll_add_node(head_node, temp_node_ptr);
            }
        }
        else
        {
            bv_free_byte_obj(temp_node_ptr->value_ptr);
            free(temp_node_ptr->value_ptr);
            free(temp_node_ptr);
        }
        temp_node_ptr = temp_node_ptr2;
    }

    i = 0;
    while (head_node != null)
    {
        byte_ptr = head_node->value_ptr;
        bv_print_char(BV_SPACE_CHAR, 1);
        bv_print_char(BV_SPACE_CHAR, BV_SYM_COUNT);
        if (head_node->next == null)
        {
            bv_print_char(BV_L_CHAR, 1);
        }
        else
        {
            bv_print_char(BV_PERPND_T_CHAR, 1);
        }
        bv_print_char(BV_HIFEN_CHAR, BV_SYM_COUNT);
        bv_apply_byte_key_color();
        fprintf(stdout, "%llu: ", byte_ptr->id);
        bv_apply_byte_value_color();
        fprintf(stdout, "%s\n", byte_ptr->name);
        bv_apply_normal_color();

        temp_node_ptr2 = head_node->next;
        bv_free_byte_obj(head_node->value_ptr);
        free(head_node->value_ptr);
        free(head_node);
        head_node = temp_node_ptr2;
        i++;
    }

    if (i == 0)
    {
        bv_print_char(BV_SPACE_CHAR, 1);
        bv_print_char(BV_SPACE_CHAR, BV_SYM_COUNT);
        bv_print_char(BV_L_CHAR, 1);
        bv_print_char(BV_HIFEN_CHAR, BV_SYM_COUNT);
        bv_apply_byte_key_color();
        fprintf(stdout, "Empty\n");
        bv_apply_normal_color();
    }

    free(username);
    free(password);
    free(temp);
}

void bv_ca_stop_user_session()
{
    char cache_root_path[MAX_PATH];
    char session_file_path[MAX_PATH + 50];
    int rc;

    bv_get_app_cache_folder_path(cache_root_path, MAX_PATH);
    sprintf(session_file_path, "%s\\%s", cache_root_path, BV_FILE_USER_SESSION);
    if (bv_is_existing_file(session_file_path))
    {
        rc = remove(session_file_path);
        if (rc == 0)
        {
            bv_log("Last session is stopped");
        }
        else
        {
            bv_error("Could not stop last session");
        }
    }
    else
    {
        bv_error("There is no user session");
    }
}

int bv_get_username_from_session(char *dest)
{
    char cache_root_path[MAX_PATH];
    char session_file_path[MAX_PATH + 50];
    char temp[100];
    char temp2[100];
    size_t read = 0;

    bv_get_app_cache_folder_path(cache_root_path, MAX_PATH);
    sprintf(session_file_path, "%s\\%s", cache_root_path, BV_FILE_USER_SESSION);
    FILE *session_file = null;
    session_file = fopen(session_file_path, "r");
    if (session_file == null)
    {
        return BV_OP_FAILED;
    }
    read = fread(temp, sizeof(char), 99, session_file);
    fclose(session_file);
    temp[read] = '\0';
    bv_decrypt_text(temp, BV_PRIMARY_KEY, temp2);
    strcpy(dest, temp2);

    return BV_OP_SUCCESS;
}

bv_bool bv_check_password(const char *correct_pass, const char *testing_pass)
{
    if (correct_pass == null || testing_pass == null)
    {
        return bv_false;
    }
    if (!strcmp(correct_pass, testing_pass))
    {
        return bv_true;
    }
    else
    {
        return bv_false;
    }
}
