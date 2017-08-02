//
// Created by Ariyan Khan on 05-10-2016.
//

#include <windows.h>
#include <wincrypt.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <conio.h>
#include <stdarg.h>

#include "include/common.h"
#include "include/logger.h"
#include "include/encryption.h"
#include "include/user.h"
#include "include/viewer.h"
#include "include/sqlite/sqlite3.h"
#include "include/utils.h"
#include "include/database.h"
#include "include/commands.h"
#include "include/command_actions.h"
#include "include/constants.h"


int test();
int main(int argc, char** argv) {
    bv_callback_on_app_start();
    command_processor(argc, argv, BV_EXECUTION_MODE_NORMAL);
    /*
     * In execution mode BV_EXECUTION_MODE_NORMAL waits for input and
     * THEN CLEAR SCREEN.
     * But in BV_EXECUTION_MODE_SHELL don't wait just clear screen.
     */
    if(execution_mode == BV_EXECUTION_MODE_NORMAL) {
        fprintf(stdout, "Enter any key to exit: ");
        getch();
    }
    bv_clear_screen();
    test();
    return EXIT_SUCCESS;
}

short bv_backup_back_to_chars;
static void bv_backup_callback(int code, size_t total_backed_up, size_t total_bytes) {
    if(code == 0x1) {
        short i = 0;
        char temp[10];
        double percent = 0.00;

        for(i = 0; i<bv_backup_back_to_chars; i++) {
            fprintf(stdout, "\b");
        }
        percent = ((double)total_backed_up / (double)total_bytes) * (double)100;
        sprintf(temp, "%.2lf%s", percent, "%");
        bv_backup_back_to_chars = (short)strlen(temp);
        fprintf(stdout, "%s", temp);
    }
}



typedef struct LocalStorage {
    void (*print)(struct LocalStorage*);
} LocalStorage;

void ___ls_print___(struct LocalStorage*);

void ___ls_print___(struct LocalStorage* ptr) {
    printf("dfsfssdffds");
}

LocalStorage LS_Init() {
    LocalStorage ls;
    ls.print = &___ls_print___;
    return ls;
}


int test() {

//Rousan Ali Holchod!





//bv_log("%d", execution_mode);


  /*HANDLE file = CreateFile("Z:\\Projects\\C & CPP\\CLion\\ByteVault\\src\\test.txt", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //WINBOOL done = LockFile(file, 0, 0, LONG_MAX, LONG_MAX);

    //printf("%d", done);

    FILE* file2 = fopen("Z:\\Projects\\C & CPP\\CLion\\ByteVault\\src\\test.txt", "r");
    printf("%d", file2 == null);

    char temp[MAX_PATH];
    GetTempPath(MAX_PATH, temp);
    bv_generate_random_key(temp, 20);

    printf(" %s ", temp);

    getch();

*/



    /*HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
DWORD dword;;;
    GetConsoleFontSize(handle, dword);
    printf("\n%d\n", bv_check_password("lhl", "kkkk"));

    puts("");

    char temp2[100] = "";
    strncat(temp2, "sdf", 10000);
    puts(temp2);

    char *err;
    bv_user user2;
    bv_get_user_info("byter", &user2, &err);
    bv_free_user_obj(&user2);


    bv_log("fdgbgb : %llu", 55445);
    bv_warn("sgs : %d", 43534);

    size_t arr[] =  {234324, 234234, 14112, 1, 77777777};

    qsort(arr, 5, sizeof(size_t), bv_compare_function_ull);

    int n;
    for( n = 0 ; n <5; n++ )
    {
        printf("%llu ", arr[n]);
    }

    size_t i = 234324;

    printf("\nPos: %llu\n", (size_t)bsearch(&i, arr, 0, sizeof(size_t), bv_compare_function_ull));

    HMODULE hmodule = GetModuleHandleW(NULL);
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(hmodule, path, MAX_PATH);
    wprintf(path);

    GetCurrentDirectoryW(MAX_PATH, path);

    printf("\n");
    wprintf(path);



    printf("%d", bv_is_existing_folder("C:\\ByteVault\\ByteVault-1.0.0\\bin\\cache"));

    bv_byte byte;
    byte.name = "gfffgfd dfbfdb dfbd fv";
    byte.date_created = 34535;
    byte.date_last_modified = 3434;
    byte.data_count = 1;
    byte.data = malloc(sizeof(bv_data)*2);
    byte.data[0].key = "ergegrer";
    byte.data[0].value = "3546456";
    byte.data[1].key = "d443f";
    byte.data[1].value = "admin";
    byte.parent_byte_id = 0;

    char* errMsg = NULL;
    int rc;
    bv_node* node = NULL;
    bv_byte* byte_p;
    puts("");
  //  bv_query_bytes("byter", "-i 1", 100, NULL, 1, &node, &errMsg);
   // byte_p = node->value_ptr;
    //printf("%d", byte_p->data_count);
   // bv_insert_byte("byter", &byte, &errMsg);
    //printf("%d", byte.id);

    bv_edit_byte_info("byter", 2, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_NAME, "SSSSSS", &errMsg);

    char temp[100];
    //bv_insert_byte("byter", &byte, &errMsg);
    ///bv_encrypt_text("A", "45e475a2900a8f99517c9f4e779200e3", temp);
     //bv_edit_byte_info("byter", 5, &byte, &errMsg);
     //bv_query_bytes("test", "-i ", -1423423, 0, 1, &node, &errMsg);
    //printf(" %s ", temp);
    //byte_p = node->value_ptr;
    //printf(" j%d ", byte_p->data_count);
   // bv_print_bytes("Bytes", node, "-d");


   // printf("%d", bv_starts_with_str("  ", "    "));

  //  bv_trim("", temp);
   // puts(temp);




    bv_get_direct_child_bytes("ariyankhan", 2, 1, &node, &errMsg);
    printf("%llu", bv_ll_count(node));




    size_t g[4] = {1, 2999, 7, 8};
    bv_backup_byte(stdout, "byter", g, 4, 1, c, 1, &err);
    printf("\nAAAAAAAAAAA %llu AAAAAAAAAAA\n", xt);




    bv_user user;
    bv_user user1;
    //bv_get_user_info("byter", &user, &errMsg);
    //bv_print_user_info("UserInfo", &user, 0, 0);
    //puts(user.password);
    //bv_edit_user_info("byter", "password", "8768sgffgfdgd6", &errMsg);
   // bv_get_user_info("byter", &user, &errMsg);
   //bv_print_user_info("UserInfo", &user, 0, 0);
//puts(user.password);


// bv_query_bytes("byter", "-n facebook", -9, " order by id asc ", 1, &node, &errMsg);
//bv_print_bytes("Bytes", node);
//printf("%llu", bv_ll_count(node));

//printf("%d", bv_search_query_in_str("Sbi", byte.name));

// bv_query_bytes("test", "all", -1, null, 1, &node, &errMsg);
//  bv_print_bytes("Bytes", node);

/*bv_data data1 = {"Username", "byter"};
bv_data data2 = {"Password", "4324d2422234"};
bv_data data3 = {"Email", "ariyankhan46@gmail.com"};
bv_data* data = malloc(sizeof(bv_data)*3);
data[0] = data1;
data[1] = data2;
data[2] = data3;

bv_byte* bytes = malloc(sizeof(bv_byte) * 10);
int i = 0;
for(; i<10; i++) {
    bytes[i].name = "SBI Account[id=1]";
    bytes[i].data = data;
    bytes[i].data_count = 3;
}

bv_user bv_user1;
bv_user1.username = "Byter";
bv_user1.enc_key = "SDfdsf";
bv_user1.hints = "sdfdsf";
bv_user1.password = "sfdsfdsf";
bv_print_user_info(&bv_user1, 345);

bv_print_bytes("ujfujhgjh", bytes, 10);

*/
}


















