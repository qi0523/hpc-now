/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <malloc.h>
#elif __linux__
#include <malloc.h>
#include <sys/time.h>
#elif __APPLE__
#include <sys/time.h>
#endif

#include "now_macros.h"
#include "cluster_general_funcs.h"
#include "cluster_init.h"
#include "cluster_operations.h"
#include "general_funcs.h"
#include "general_print_info.h"
#include "components.h"
#include "prereq_check.h"
#include "time_process.h"
#include "usage_and_logs.h"
#include "dataman.h"
#include "transfer.h"
#include "monman.h"
#include "appman.h"
#include "jobman.h"

char url_code_root_var[LOCATION_LENGTH]="";
char url_tf_root_var[LOCATION_LENGTH]="";
char url_shell_scripts_var[LOCATION_LENGTH]="";
char url_now_crypto_var[LOCATION_LENGTH]="";
char url_initutils_root_var[LOCATION_LENGTH]="";
int tf_loc_flag_var=0;
int code_loc_flag_var=0;
int now_crypto_loc_flag_var=0;

char terraform_version_var[16]="";
char ali_tf_plugin_version_var[16]="";
char qcloud_tf_plugin_version_var[16]="";
char aws_tf_plugin_version_var[16]="";
char hw_tf_plugin_version_var[16]="";
char bd_tf_plugin_version_var[16]="";
char azrm_tf_plugin_version_var[16]="";
char azad_tf_plugin_version_var[16]="";
char az_environment[16]="";
char gcp_tf_plugin_version_var[16]="";

char md5_tf_exec_var[64]="";
char md5_tf_zip_var[64]="";
char md5_now_crypto_var[64]="";
char md5_ali_tf_var[64]="";
char md5_ali_tf_zip_var[64]="";
char md5_qcloud_tf_var[64]="";
char md5_qcloud_tf_zip_var[64]="";
char md5_aws_tf_var[64]="";
char md5_aws_tf_zip_var[64]="";
char md5_hw_tf_var[64]="";
char md5_hw_tf_zip_var[64]="";
char md5_bd_tf_var[64]="";
char md5_bd_tf_zip_var[64]="";
char md5_azrm_tf_var[64]="";
char md5_azrm_tf_zip_var[64]="";
char md5_azad_tf_var[64]="";
char md5_azad_tf_zip_var[64]="";
char md5_gcp_tf_var[64]="";
char md5_gcp_tf_zip_var[64]="";

/*
 * GEN: GENERAL COMMANDS
 * opr: ONLY Operator can execute
 * ADMIN: ONLY Operator and Admin can execute
 * 
 * CNAME: cluster_name is a must
 * UNAME: user_name is must
 */
char commands[COMMAND_NUM][COMMAND_STRING_LENGTH_MAX]={
    "envcheck,gen,NULL",
    "new-cluster,gen,NULL",
    "ls-clusters,gen,NULL",
    "switch,gen,NULL",
    "glance,gen,NULL",
    "refresh,gen,CNAME",
    "export,gen,CNAME",
    "import,gen,NULL",
    "exit-current,gen,NULL",
    "remove,gen,CNAME",
    "help,gen,NULL",
    "usage,gen,NULL",
    "history,gen,NULL",
    "syserr,gen,NULL",
    "ssh,gen,UNAME",
    "configloc,gen,NULL",
    "showloc,gen,NULL",
    "resetloc,gen,NULL",
    "showmd5,gen,NULL",
    "rotate-key,opr,CNAME",
    "get-conf,opr,CNAME",
    "edit-conf,opr,CNAME",
    "rm-conf,opr,CNAME",
    "init,opr,CNAME",
    "rebuild,opr,CNAME",
    "vault,gen,CNAME",
    "graph,gen,CNAME",
    "viewlog,opr,CNAME",
    "delc,opr,CNAME",
    "addc,opr,CNAME",
    "shutdownc,opr,CNAME",
    "turnonc,opr,CNAME",
    "reconfc,opr,CNAME",
    "reconfm,opr,CNAME",
    "nfsup,opr,CNAME",
    "sleep,opr,CNAME",
    "wakeup,opr,CNAME",
    "destroy,opr,CNAME",
    "payment,opr,CNAME",
    "userman,gen,CNAME",
    "dataman,gen,UNAME",
    "appman,gen,UNAME",
    "jobman,gen,UNAME",
    "monman,admin,CNAME",
    "about,gen,NULL",
    "version,gen,NULL",
    "license,gen,NULL",
    "repair,gen,NULL"
};

char dataman_commands[DATAMAN_COMMAND_NUM][COMMAND_STRING_LENGTH_MAX]={
    "put",
    "rput",
    "get",
    "rget",
    "copy",
    "list",
    "delete",
    "move",
    "cp",
    "mv",
    "ls",
    "rm",
    "mkdir",
    "cat",
    "more",
    "less",
    "tail"
};

char appman_commands[6][COMMAND_STRING_LENGTH_MAX]={
    "store",
    "avail",
    "build",
    "install",
    "check",
    "remove"
};

char jobman_commands[3][COMMAND_STRING_LENGTH_MAX]={
    "list",
    "submit",
    "cancel"
};

/*
1 NOT_A_VALID_COMMAND
3 USER_DENIED
5 LACK_PARAMS
6 CLOUD_FUNCTION_UNSUPPORTED
7 MISSING_CLOUD_FLAG_FILE
8 CLOUD_FLAG_NOT_APPLICABLE
9 PARAM_FORMAT_ERROR

11 Prereq - Components Download and install failed
13 Prereq - Other failed
15 Prereq - Envcheck Failed
17 Prereq - Config Location Failed
19 Prereq - Vers and md5 Error
21 CLUSTER_NAME_CHECK_FAILED
23 INVALID_KEYPAIR
25 Not Operating Clusters
26 INVALID_USER_NAME
27 EMPTY_CLUSTER_OR_IN_PROGRESS
28 DATAMAN_FAILED
29 OPERATION_IN_PROGRESS
30 EXPORT_FAILED
31 REFRESHING FAILED
32 IMPORT_FAILED
33 EMPTY REGISTRY
34 REBUILD_FAILED
35 Failed to exit current
36 CLUSTER_ROLE_DOESN'T_MATCH
37 NO_NEED_TO_SWITCH
38 NO_NEED_TO_WAKEUP
39 NOT_IN_THE_REGISTRY
40 MONMAN_FAILED
41 DESTROY_ERROR
42 PAYMENT_SWITCH_FAILED
43 CLUSTER_ASLEEP
44 APPMAN_FAILED
45 GRAPH_FAILED
46 JOBMAN_FAILED
47 GRAPH_NOT_UPDATED
49 CLUSTER_EMPTY
51 CLUSTER_NOT_EMPTY
53 PROCESS_LOCKED
55 NO_CONF_FILE
57 ALREADY_INITED
59 UNKNOWN_CLOUD
61 WORKDIR_NOT_EXIST
63 AWS_REGION_VALID_FAILED
65 GET_FILE_FAILED
67 ZONE_ID_ERROR
69 AWS_INVALID_KEYPAIR
71 TF_INIT_FAILED
73 TF_APPLY_FAILED_ROLLED_BACK
75 TF_ROLLBACK_FAILED
77 USERMAN PREREQ_CHECK_FAILED

117 USER_CHECK_ERROR
119 KEY_FOLDER_ERROR
121 INTERNET_CHECK_FAILED
123 FATAL_ABNORMAL
125 FATAL_INTERNAL_ERROR
127 File I/O Error

SPECIAL RETURN VALUES: when the command_input is wrong.

200~255: command_check_prompt_index

*/

int main(int argc, char* argv[]){
    char* crypto_keyfile=CRYPTO_KEY_FILE;
    char command_name_prompt[128]="";
    char cloud_flag[16]="";
    int command_flag=0;
    int level_flag=0;
    int run_flag=0;
    int usrmgr_check_flag=0;
    char workdir[DIR_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX]="";
    char new_cluster_name[128]="";
    char cloud_ak[AKSK_LENGTH]="";
    char cloud_sk[AKSK_LENGTH]="";
    char stream_name[128]="";
    char log_type[128]="";
    char user_name[128]="";
    char pass_word[128]="";
    char user_name_list[1024]="";
    char vault_bucket_flag[8]="";
    char vault_root_flag[8]="";
    char export_dest[FILENAME_LENGTH]="";
    char import_source[FILENAME_LENGTH]="";

    char data_cmd[128]="";
    char source_path[FILENAME_LENGTH]="";
    char destination_path[FILENAME_LENGTH]="";
    char target_path[FILENAME_LENGTH]="";
    char recursive_flag[16]="";
    char force_flag_string[16]="";
    char node_num_string[128]="";
    char user_cmd[128]="";
    char app_cmd[128]="";
    char app_name[128]="";
    char job_cmd[128]="";
    char job_id[32]="";
    char* usage_log=USAGE_LOG_FILE;
    char* operation_log=OPERATION_LOG_FILE;
    char* syserror_log=SYSTEM_CMD_ERROR_LOG;
    char string_temp[256]="";
    char string_temp2[256]="";
    char string_temp3[256]="";
    char string_temp4[4]="";
    char doubleconfirm[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char cluster_role[8]="";
    int cluster_state_flag=0;
    jobinfo job_info;
    print_header();

#ifdef _WIN32
    if(check_current_user()!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] You *MUST* run hpcopr as the OS user 'hpc-now'." RESET_DISPLAY "\n");
        printf("|          Please follow the steps below:\n");
        printf("|          1. run the command " HIGH_GREEN_BOLD "runas /savecred /user:mymachine\\hpc-now cmd" RESET_DISPLAY "\n");
        printf("|          2. run the " HIGH_GREEN_BOLD "hpcopr" RESET_DISPLAY " commands in the *new* CMD window\n");
        print_tail();
        return 117;
    }
#else
    if(check_current_user()!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] You *MUST* run hpcopr as the OS user 'hpc-now'." RESET_DISPLAY "\n");
        printf("|     <> Please follow the instructions below:\n");
        printf("|     -> SUDO-MODE (pretty simple and fast for *sudoers*): \n");
        printf("|          run the hpcopr as " HIGH_GREEN_BOLD "sudo -u hpc-now hpcopr ..." RESET_DISPLAY "\n");
        printf("|     -> USER-MODE (for both *non-sudoers* and *sudoers*): \n");
        printf("|          1. " HIGH_GREEN_BOLD "su hpc-now" RESET_DISPLAY " (The password of 'hpc-now' is needed)\n");
        printf("|          2. run the " HIGH_GREEN_BOLD "hpcopr" RESET_DISPLAY " commands, i.e. " HIGH_GREEN_BOLD "hpcopr glance --all" RESET_DISPLAY "\n");
        print_tail();
        return 117;
    }
#endif

#ifdef _WIN32
    if(folder_exist_or_not("c:\\hpc-now")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The key directory C:\\hpc-now\\ is missing. The services cannot start.\n");
        printf("|          Please switch to Administrator and re-install the services to fix.\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
        print_tail();
        return 119;
    }
#elif __APPLE__
    if(folder_exist_or_not("/Applications/.hpc-now/")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The service is corrupted due to missing critical folder. Please exit\n");
        printf("|          and run the installer with 'sudo' to reinstall it. Sample command:\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH uninstall\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH install\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
        print_tail();
        return 119;
    }
#elif __linux__
    if(folder_exist_or_not("/usr/.hpc-now/")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The service is corrupted due to missing critical folder. Please exit\n");
        printf("|          and run the installer with 'sudo' to reinstall it. Sample command:\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH uninstall\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH install\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
        print_tail();
        return 119;
    }
#endif
    if(folder_exist_or_not(GENERAL_CONF_DIR)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,GENERAL_CONF_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }

    command_flag=command_parser(argc,argv,command_name_prompt,workdir,cluster_name,user_name,cluster_role);
    if(command_flag==-1){
        print_help("all");
        return 0;
    }
    else if(command_flag>199){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid Command. Did you mean " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ?\n" RESET_DISPLAY,command_name_prompt);
        print_tail();
        return command_flag;
    }
    else if(command_flag==-3){
        check_and_cleanup("");
        return 5;
    }
    else if(command_flag==-5){
        check_and_cleanup(workdir);
        return 5;
    }
    else if(command_flag==-7){
        write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
        check_and_cleanup(workdir);
        return 49;
    }
    else if(command_flag==1){
        check_and_cleanup(workdir);
        return 5;
    }
    if(strcmp(argv[1],"help")==0){
        if(cmd_keyword_check(argc,argv,"--cmd",string_temp)!=0){
            print_help("all");
        }
        else{
            if(command_name_check(string_temp,command_name_prompt,string_temp2,string_temp3)>199){
                printf(FATAL_RED_BOLD "[ FATAL: ] The specified command name is incorrect. Did you mean " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ?\n" RESET_DISPLAY,command_name_prompt);
                write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                check_and_cleanup("");
                return 9;
            }
            print_help(string_temp);
        }
        return 0;
    } 

    if(strcmp(argv[1],"about")==0){
        print_about();
        return 0;
    }
    
    if(strcmp(argv[1],"version")==0){
        print_version();
        return 0;
    }

    if(check_internet()!=0){
        write_operation_log("NULL",operation_log,argc,argv,"INTERNET_CHECK_FAILED",121);
        check_and_cleanup("");
        return 121;
    }

    if(strcmp(argv[1],"license")==0){
        if(cmd_flag_check(argc,argv,"--print")==0){
            read_license("print");
        }
        else{
            read_license("read");
        }
        print_tail();
        return 0;
    }

    if(strcmp(argv[1],"repair")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Entering repair mode. All the locations will be reset to default,\n");
        printf("|          and all the core components will be replaced by the default ones.\n");
        printf("|          Would you like to continue? Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm.\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,CONFIRM_STRING)!=0){
            printf("\n[ -INFO- ] Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to continue. You chose to deny this operation.\n");
            printf("|          Nothing changed. Exit now.\n");
            write_operation_log("NULL",operation_log,argc,argv,"USER_DENIED",3);
            print_tail();
            return 3;
        }
        run_flag=check_and_install_prerequisitions(1);
        if(run_flag==3){
            write_operation_log("NULL",operation_log,argc,argv,"COMPONENTS_DOWNLOAD_AND_INSTALL_FAILED",11);
            check_and_cleanup("");
            return 11;
        }
        else if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"REPAIR_FAILED",run_flag);
            check_and_cleanup("");
            return 13;
        }
        else{
            write_operation_log("NULL",operation_log,argc,argv,"REPAIR_SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
    }

    if(strcmp(argv[1],"envcheck")==0){
        if(cmd_flag_check(argc,argv,"--gcp")==0){
            run_flag=check_and_install_prerequisitions(2); // Check GCP Connectivity.
        }
        else{
            run_flag=check_and_install_prerequisitions(0);
        }
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"ENVCHECK_FAILED",run_flag);
            check_and_cleanup("");
            return 15;
        }
        write_operation_log("NULL",operation_log,argc,argv,"ENVCHECK_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"configloc")==0){
        run_flag=configure_locations();
        if(run_flag==1){
            write_operation_log("NULL",operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup("");
            return 3;
        }
        else if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"resetloc")==0){
        run_flag=reset_locations();
        if(run_flag==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The locations have been reset to the default.\n");
            show_locations();
            write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal error, failed to reset the locations." RESET_DISPLAY "\n");
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
    }

    if(strcmp(argv[1],"showloc")==0){
        run_flag=show_locations();
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"showmd5")==0){
        run_flag=show_vers_md5vars();
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    run_flag=check_and_install_prerequisitions(0);
    if(run_flag==3){
        write_operation_log("NULL",operation_log,argc,argv,"COMPONENTS_DOWNLOAD_AND_INSTALL_FAILED",-11);
        check_and_cleanup("");
        return 11;
    }
    else if(run_flag==-1){
        write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
        check_and_cleanup("");
        return 127;
    }
    else if(run_flag==-3){
        write_operation_log("NULL",operation_log,argc,argv,"RESET_LOCATION_FAILED",13);
        check_and_cleanup("");
        return 13;
    }
    else if(run_flag==1){
        write_operation_log("NULL",operation_log,argc,argv,"USER_DENIED",3);
        check_and_cleanup("");
        return 3;
    }
    else if(run_flag==5){
        write_operation_log("NULL",operation_log,argc,argv,"CONFIG_LOCATION_FAILED",17);
        check_and_cleanup("");
        return 17;
    }
    else if(run_flag==7){
        write_operation_log("NULL",operation_log,argc,argv,"VERSION_MD5SUM_ERROR",19);
        check_and_cleanup("");
        return 19;
    }
    
    cluster_state_flag=cluster_asleep_or_not(workdir);
    if(strcmp(argv[1],"new-cluster")==0){
        cmd_keyword_check(argc,argv,"--cname",new_cluster_name);
        cmd_keyword_check(argc,argv,"--ak",cloud_ak);
        cmd_keyword_check(argc,argv,"--sk",cloud_sk);
        cmd_keyword_check(argc,argv,"--az-sid",string_temp);
        cmd_keyword_check(argc,argv,"--az-tid",string_temp2);
        if(cmd_flag_check(argc,argv,"--gcp")==0){
            run_flag=create_new_cluster(crypto_keyfile,new_cluster_name,"",cloud_sk,"","","","gcp");
        }
        else{
            if(cmd_flag_check(argc,argv,"--echo")==0){
                run_flag=create_new_cluster(crypto_keyfile,new_cluster_name,cloud_ak,cloud_sk,string_temp,string_temp2,"echo","");
            }
            else{
                run_flag=create_new_cluster(crypto_keyfile,new_cluster_name,cloud_ak,cloud_sk,string_temp,string_temp2,"","");
            }
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            write_operation_log("NULL",operation_log,argc,argv,"CLUSTER_NAME_CHECK_FAILED",21);
            check_and_cleanup(workdir);
            return 21;
        }
        else if(run_flag==3){
            write_operation_log("NULL",operation_log,argc,argv,"INVALID_GCP_KEY_FILE",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==5){
            write_operation_log("NULL",operation_log,argc,argv,"INVALID_KEYPAIR",23);
            check_and_cleanup(workdir);
            return 23;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"ls-clusters")==0){
        run_flag=list_all_cluster_names(0);
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            write_operation_log("NULL",operation_log,argc,argv,"EMPTY_REGISTRY",33);
            check_and_cleanup(workdir);
            return 33;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"glance")==0){
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=glance_clusters("all",crypto_keyfile);
        }
        else{
            if(cmd_keyword_check(argc,argv,"-c",cluster_name)!=0&&show_current_cluster(workdir,cluster_name,0)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster." RESET_DISPLAY "\n");
                list_all_cluster_names(1);
                write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
                check_and_cleanup(workdir);
                return 25;
            }
            run_flag=glance_clusters(cluster_name,crypto_keyfile);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster." RESET_DISPLAY "\n");
            write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup(workdir);
            return 25;
        }
        else if(run_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is not in the registry.\n" RESET_DISPLAY,cluster_name);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup(workdir);
            return 39;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"refresh")==0){
        if(cmd_flag_check(argc,argv,"--force")==0){
            run_flag=refresh_cluster(cluster_name,crypto_keyfile,"force");
        }
        else{
            if(confirm_to_operate_cluster(cluster_name)!=0){
                write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
                check_and_cleanup(workdir);
                return 3;
            }
            run_flag=refresh_cluster(cluster_name,crypto_keyfile,"");
        }
        if(run_flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to refresh cluster %s. Exit now.\n" RESET_DISPLAY,cluster_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"OPERATION_FAILED",31);
            check_and_cleanup(workdir);
            return 31;
        }
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Successfully refreshed cluster %s.\n",cluster_name);
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"exit-current")==0){
        if(exit_current_cluster()==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exited the switched cluster. You can switch to one later.\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ -INFO- ] Failed to exit the switched cluster. Please retry later." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"OPERATION_FAILED",35);
            check_and_cleanup("");
            return 35;
        }
    }

    if(strcmp(argv[1],"usage")==0){
        cmd_keyword_check(argc,argv,"-d",export_dest);
        if(cmd_flag_check(argc,argv,"--read")==0){
            run_flag=view_system_logs(usage_log,"read",export_dest);
        }
        else{
            run_flag=view_system_logs(usage_log,"print",export_dest);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }
    if(strcmp(argv[1],"history")==0){
        cmd_keyword_check(argc,argv,"-d",export_dest);
        if(cmd_flag_check(argc,argv,"--read")==0){
            run_flag=view_system_logs(operation_log,"read",export_dest);
        }
        else{
            run_flag=view_system_logs(operation_log,"print",export_dest);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"syserr")==0){
        cmd_keyword_check(argc,argv,"-d",export_dest);
        if(cmd_flag_check(argc,argv,"--read")==0){
            run_flag=view_system_logs(syserror_log,"read",export_dest);
        }
        else{
            run_flag=view_system_logs(syserror_log,"print",export_dest);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"import")==0){
        cmd_keyword_check(argc,argv,"-s",import_source);
        cmd_keyword_check(argc,argv,"-p",pass_word);
        run_flag=import_cluster(import_source,pass_word,crypto_keyfile);
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"IMPORT_FAILED",32);
            check_and_cleanup("");
            return 32;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(argv[1],"monman")==0){
        cmd_keyword_check(argc,argv,"-n",string_temp);
        cmd_keyword_check(argc,argv,"-s",string_temp2);
        cmd_keyword_check(argc,argv,"-e",string_temp3);
        cmd_keyword_check(argc,argv,"-d",destination_path);
        cmd_keyword_check(argc,argv,"--level",string_temp4);
        if(cmd_flag_check(argc,argv,"--read")==0){
            run_flag=show_cluster_mon_data(cluster_name,SSHKEY_DIR,string_temp,string_temp2,string_temp3,string_temp4,"read",destination_path);
        }
        else{
            run_flag=show_cluster_mon_data(cluster_name,SSHKEY_DIR,string_temp,string_temp2,string_temp3,string_temp4,"print",destination_path);
        }
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"MONITOR_MANAGER_FAILED",40);
            check_and_cleanup(workdir);
            return 40;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"switch")==0){
        if(cmd_flag_check(argc,argv,"--list")==0){
            run_flag=list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        if(cmd_keyword_check(argc,argv,"-c",cluster_name)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY WARN_YELLO_BOLD "-c CLUSTER_NAME" RESET_DISPLAY FATAL_RED_BOLD " ." RESET_DISPLAY "\n");
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup(workdir);
            return 25;
        }
        run_flag=switch_to_cluster(cluster_name);
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        else if(run_flag==3){
            write_operation_log("NULL",operation_log,argc,argv,"NO_NEED_TO_SWITCH",37);
            check_and_cleanup("");
            return 37;
        }
        else if(run_flag==1){
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup("");
            return 39;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"remove")==0){
        if(cmd_flag_check(argc,argv,"--force")==0){
            run_flag=remove_cluster(cluster_name,crypto_keyfile,"force");
        }
        else{
            run_flag=remove_cluster(cluster_name,crypto_keyfile,"");
        }
        if(run_flag==1){
            write_operation_log(argv[2],operation_log,argc,argv,"CLUSTER_NAME_CHECK_FAILED",21);
            check_and_cleanup(workdir);
            return 21; 
        }
        else if(run_flag==3){
            write_operation_log("NULL",operation_log,argc,argv,"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup(workdir);
            return 39;
        }
        else if(run_flag==5){
            write_operation_log(argv[2],operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==7){
            write_operation_log(argv[2],operation_log,argc,argv,"CLUSTER_DESTROY_FAILED",41);
            check_and_cleanup(workdir);
            return 41;
        }
        write_operation_log(argv[2],operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"viewlog")==0){
        cmd_keyword_check(argc,argv,"-d",string_temp);
        if(cmd_flag_check(argc,argv,"--err")==0){
            strcpy(stream_name,"err");
        }
        else{
            strcpy(stream_name,"std");
        }
        if(cmd_flag_check(argc,argv,"--hist")==0){
            strcpy(log_type,"archive");
        }
        else{
            strcpy(log_type,"realtime");
        }
        if(cmd_flag_check(argc,argv,"--print")==0){
            run_flag=view_run_log(workdir,stream_name,log_type,"print",string_temp);
        }
        else{
            run_flag=view_run_log(workdir,stream_name,log_type,"",string_temp);
        }
        if(run_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the log. Have you specified or switched to a cluster?\n" RESET_DISPLAY );
            list_all_cluster_names(1);
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,argv[2],0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"ssh")==0){
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,cluster_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup(workdir);
            return 43;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Trying to ssh " HIGH_GREEN_BOLD "%s@%s" RESET_DISPLAY ".\n",user_name,cluster_name);
        if(strcmp(user_name,"root")==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] SSH as root is VERY RISKY and *NOT* recommended! Only for operator or admins." RESET_DISPLAY "\n");
        }
        run_flag=cluster_ssh(workdir,user_name,cluster_role);
        if(run_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the ssh key. You can still try to use password to login." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"graph")==0){
        cmd_keyword_check(argc,argv,"--level",string_temp);
        if(strcmp(string_temp,"csv")==0){
            level_flag=2;
        }
        else if(strcmp(string_temp,"txt")==0){
            level_flag=1;
        }
        else{
            level_flag=0;
        }
        if(check_pslock(workdir)!=0){
            if(cluster_empty_or_not(workdir)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n\n" RESET_DISPLAY,cluster_name);
            }
            else{
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n" RESET_DISPLAY,cluster_name);
            }
            run_flag=graph(workdir,crypto_keyfile,level_flag);
            if(run_flag==1){
                write_operation_log(cluster_name,operation_log,argc,argv,"GRAPH_FAILED",45);
                check_and_cleanup(workdir);
                return 47;
            }
            else if(run_flag==-1){
                write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
                check_and_cleanup(workdir);
                return 125;
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"GRAPH_NOT_UPDATED",47);
            check_and_cleanup(workdir);
            return 47;
        }
        decrypt_files(workdir,crypto_keyfile);
        printf("\n");
        run_flag=graph(workdir,crypto_keyfile,level_flag);
        if(run_flag==1){
            print_empty_cluster_info();
            delete_decrypted_files(workdir,crypto_keyfile);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        else if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"vault")==0){
        cmd_keyword_check(argc,argv,"-u",user_name);
        if(cmd_flag_check(argc,argv,"--bkey")==0){
            strcpy(vault_bucket_flag,"bucket");
        }
        if(cmd_flag_check(argc,argv,"--rkey")==0){
            strcpy(vault_root_flag,"root");
        }
        run_flag=get_vault_info(workdir,crypto_keyfile,user_name,vault_bucket_flag,vault_root_flag);
        if(run_flag==1){
            print_empty_cluster_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        else if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"rotate-key")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        cmd_keyword_check(argc,argv,"--ak",cloud_ak);
        cmd_keyword_check(argc,argv,"--sk",cloud_sk);
        if(cmd_flag_check(argc,argv,"--echo")==0){
            run_flag=rotate_new_keypair(workdir,cloud_ak,cloud_sk,crypto_keyfile,"echo");
        }
        else{
            run_flag=rotate_new_keypair(workdir,cloud_ak,cloud_sk,crypto_keyfile,"");
        }
        if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==-3){
            write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else if(run_flag==1){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==3){
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_KEYPAIR",23);
            check_and_cleanup(workdir);
            return 23;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(get_cloud_flag(workdir,cloud_flag)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the cloud flag. Have you switched to any cluster?\n");
        printf("|          Exit now." RESET_DISPLAY "\n");
        write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FLAG_CHECK_FAILED",7);
        check_and_cleanup(workdir);
        return 7;
    }

    if(strcmp(argv[1],"export")==0){
        if(cluster_empty_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is empty, nothing to be exported." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        cmd_keyword_check(argc,argv,"--ul",user_name_list);
        cmd_keyword_check(argc,argv,"-p",pass_word);
        cmd_keyword_check(argc,argv,"-d",export_dest);
        if(cmd_flag_check(argc,argv,"--admin")==0){
            run_flag=export_cluster(cluster_name,user_name_list,"admin",crypto_keyfile,pass_word,export_dest);
        }
        else{
            run_flag=export_cluster(cluster_name,user_name_list,"",crypto_keyfile,pass_word,export_dest);
        }
        if(run_flag==0){
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"EXPORT_FAILED",30);
            check_and_cleanup(workdir);
            return 30;
        }
    }

    if(strcmp(argv[1],"dataman")==0){
        if(cluster_empty_or_not(workdir)==0){
            print_empty_cluster_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        if(cmd_keyword_check(argc,argv,"--dcmd",data_cmd)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a valid command to use the data manager.\n");
            printf(GENERAL_BOLD "|       +- Bucket:" RESET_DISPLAY " put, get, copy, list, delete, move\n");
            printf(GENERAL_BOLD "|       +- Remote:" RESET_DISPLAY " cp, mv, ls, rm, mkdir, cat, more, less, tail, rput, rget \n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",data_cmd);
            getchar();
        }
        int i=0;
        while(strcmp(data_cmd,dataman_commands[i])!=0){
            i++;
            if(i==DATAMAN_COMMAND_NUM){
                break;
            }
        }
        if(i==DATAMAN_COMMAND_NUM){
            printf(FATAL_RED_BOLD "[ FATAL: ] The command " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is incorrect. Valid commands:" RESET_DISPLAY "\n",data_cmd);
            printf(GENERAL_BOLD "|       +- Bucket:" RESET_DISPLAY " put, get, copy, list, delete, move\n");
            printf(GENERAL_BOLD "|       +- Remote:" RESET_DISPLAY " cp, mv, ls, rm, mkdir, cat, more, less, tail, rput, rget \n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(data_cmd,"put")!=0&&strcmp(data_cmd,"get")!=0&&strcmp(data_cmd,"copy")!=0&&strcmp(data_cmd,"list")!=0&&strcmp(data_cmd,"delete")!=0&&strcmp(data_cmd,"move")!=0){
            if(cluster_state_flag==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,cluster_name);
                write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
                check_and_cleanup(workdir);
                return 43;
            }
        }
        if(strcmp(data_cmd,"put")==0||strcmp(data_cmd,"get")==0||strcmp(data_cmd,"copy")==0||strcmp(data_cmd,"move")==0||strcmp(data_cmd,"cp")==0||strcmp(data_cmd,"mv")==0||strcmp(data_cmd,"rput")==0||strcmp(data_cmd,"rget")==0){
            if(cmd_keyword_check(argc,argv,"-s",source_path)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a source path for this command.\n");
                if(strcmp(data_cmd,"put")!=0&&strcmp(data_cmd,"get")!=0&&strcmp(data_cmd,"copy")!=0&&strcmp(data_cmd,"move")!=0){
                    printf("|          Use prefix @h/ , @d/, @p/, @a/, @R/, @t/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                }
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%s",source_path);
                getchar();
            }
            if(cmd_keyword_check(argc,argv,"-d",destination_path)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a destination path for this command.\n");
                if(strcmp(data_cmd,"put")!=0&&strcmp(data_cmd,"get")!=0&&strcmp(data_cmd,"copy")!=0&&strcmp(data_cmd,"move")!=0){
                    printf("|          Use prefix @h/ , @d/, @p/, @a/, @R/, @t/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                }
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%s",destination_path);
                getchar();
            }
        }
        else{
            if(cmd_keyword_check(argc,argv,"-t",target_path)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a target path for this command.\n");
                if(strcmp(data_cmd,"list")!=0&&strcmp(data_cmd,"delete")!=0){
                    printf("|          Use prefix @h/ , @d/ , @p/, @a/, @R/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                }
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%s",target_path);
                getchar();
            }
        }
        if(strcmp(data_cmd,"put")==0||strcmp(data_cmd,"get")==0||strcmp(data_cmd,"copy")==0||strcmp(data_cmd,"move")==0||strcmp(data_cmd,"delete")==0||strcmp(data_cmd,"list")==0||strcmp(data_cmd,"cp")==0||strcmp(data_cmd,"mv")==0||strcmp(data_cmd,"rput")==0||strcmp(data_cmd,"rget")==0||strcmp(data_cmd,"rm")==0){
            if(strcmp(data_cmd,"list")!=0&&cmd_flag_check(argc,argv,"--force")!=0&&cmd_flag_check(argc,argv,"-rf")!=0&&cmd_flag_check(argc,argv,"-f")!=0){
                printf(GENERAL_BOLD "[ -INFO- ] You may need --force or -f flag to do force operation.\n" RESET_DISPLAY );
            }
            else{
                strcpy(force_flag_string,"force");
            }
            if(strcmp(data_cmd,"mv")!=0&&cmd_flag_check(argc,argv,"--recursive")!=0&&cmd_flag_check(argc,argv,"-rf")!=0&&cmd_flag_check(argc,argv,"-r")!=0){
                printf(GENERAL_BOLD "[ -INFO- ] You may need --recursive or -r flag when operating folders.\n" RESET_DISPLAY );
            }
            else{
                strcpy(recursive_flag,"recursive");
            }
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Data operation started ...\n\n");
        if(strcmp(data_cmd,"put")==0||strcmp(data_cmd,"get")==0||strcmp(data_cmd,"copy")==0||strcmp(data_cmd,"move")==0){
            run_flag=bucket_cp(workdir,user_name,source_path,destination_path,recursive_flag,force_flag_string,crypto_keyfile,cloud_flag,data_cmd);
            if(strcmp(data_cmd,"move")==0&&run_flag==0){
                run_flag=bucket_rm_ls(workdir,user_name,source_path,"recursive","",crypto_keyfile,cloud_flag,"delete");
            }
        }
        else if(strcmp(data_cmd,"list")==0||strcmp(data_cmd,"delete")==0){
            run_flag=bucket_rm_ls(workdir,user_name,target_path,recursive_flag,force_flag_string,crypto_keyfile,cloud_flag,data_cmd);
        }
        else if(strcmp(data_cmd,"rm")==0||strcmp(data_cmd,"ls")==0||strcmp(data_cmd,"mkdir")==0){
            run_flag=direct_rm_ls_mkdir(workdir,user_name,SSHKEY_DIR,target_path,force_flag_string,recursive_flag,data_cmd);
        }
        else if(strcmp(data_cmd,"cp")==0||strcmp(data_cmd,"mv")==0){
            run_flag=direct_cp_mv(workdir,user_name,SSHKEY_DIR,source_path,destination_path,recursive_flag,force_flag_string,data_cmd);
        }
        else if(strcmp(data_cmd,"rput")==0||strcmp(data_cmd,"rget")==0){
            run_flag=remote_bucket_cp(workdir,user_name,SSHKEY_DIR,source_path,destination_path,recursive_flag,force_flag_string,cloud_flag,crypto_keyfile,data_cmd);
        }
        else{
            run_flag=direct_file_operations(workdir,user_name,SSHKEY_DIR,target_path,data_cmd);
        }
        if(run_flag==0){
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
        else{
            if(run_flag==3){
                printf(FATAL_RED_BOLD "[ FATAL: ] The source and dest path must include @ prefixes." RESET_DISPLAY "\n");
            }
            printf(WARN_YELLO_BOLD "\n[ -WARN- ] Data operation failed or canceled. Check the console output above.\n");
            printf("|      <>  Command: %s | Cluster: %s | User: %s\n" RESET_DISPLAY,data_cmd,cluster_name,user_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"DATAMAN_OPERATION_FAILED",28);
            check_and_cleanup(workdir);
            return 28;
        }
    }

    if(check_pslock(workdir)==1){
        printf(FATAL_RED_BOLD "[ FATAL: ] Another process is operating this cluster, please wait and retry.\n");
        printf("|          Exit now." RESET_DISPLAY "\n");
        write_operation_log(cluster_name,operation_log,argc,argv,"PROCESS_LOCKED",53);
        check_and_cleanup(workdir);
        return 53;
    }
    if(strcmp(argv[1],"get-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=get_default_conf(cluster_name,crypto_keyfile,1);
        if(run_flag==1||run_flag==127){
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The default configuration file has been downloaded to the local place.\n");
            printf("|          You can init directly, or edit it before init. Exit now.\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }
    if(strcmp(argv[1],"edit-conf")==0||strcmp(argv[1],"rm-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(strcmp(argv[1],"edit-conf")==0){
            run_flag=edit_configuration_file(cluster_name,crypto_keyfile);
        }
        else{
            run_flag=remove_conf(cluster_name);
        }
        if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] No configuration file found. You can run " WARN_YELLO_BOLD "hpcopr get-conf" RESET_DISPLAY FATAL_RED_BOLD " first.\n");
            printf("|          Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"NO_CONFIG_FILE",55);
            check_and_cleanup(workdir);
            return 55;
        }
        else{
            if(strcmp(argv[1],"rm-conf")==0){
                printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The previous configuration file has been deleted.\n");
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(argv[1],"init")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster has already been initialized. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"ALREADY_INITED",57);
            check_and_cleanup(workdir);
            return 57;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=cluster_init_conf(cluster_name,argc,argv);
        if(run_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Invalid cloud vendor. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else if(run_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a configuration file. Exit now." RESET_DISPLAY "\n");
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Invalid format for the --nn and/or --un. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        else if(run_flag==-3){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Configuration file found. Omitted all the specified params." RESET_DISPLAY "\n");
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] Configuration file not found. Using the specified or default params." RESET_DISPLAY "\n");
        }
        if(strcmp(cloud_flag,"CLOUD_A")==0){
            run_flag=alicloud_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            run_flag=qcloud_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            run_flag=aws_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_D")==0){
            run_flag=hwcloud_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_E")==0){
            run_flag=baiducloud_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_F")==0){
            run_flag=azure_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_G")==0){
            run_flag=gcp_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Unknown Cloud Service Provider. Exit now." RESET_DISPLAY "\n");
            check_and_cleanup(workdir);
            return 59;
        }
        if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"WORKDIR_NOT_EXISTS",61);
            check_and_cleanup(workdir);
            return 61;
        }
        else if(run_flag==1){
            write_operation_log(cluster_name,operation_log,argc,argv,"AWS_REGION_VALID_FAILED",63);
            check_and_cleanup(workdir);
            return 63;
        }
        else if(run_flag==2){
            write_operation_log(cluster_name,operation_log,argc,argv,"DOWNLOAD/COPY_FILE_FAILED",65);
            check_and_cleanup(workdir);
            return 65;
        }
        else if(run_flag==3){
            write_operation_log(cluster_name,operation_log,argc,argv,"ZONE_ID_ERROR",67);
            check_and_cleanup(workdir);
            return 67;
        }
        else if(run_flag==4){
            write_operation_log(cluster_name,operation_log,argc,argv,"AWS_INVALID_KEYPAIR",69);
            check_and_cleanup(workdir);
            return 69;
        }
        else if(run_flag==5){
            write_operation_log(cluster_name,operation_log,argc,argv,"TF_INIT_FAILED",71);
            check_and_cleanup(workdir);
            return 71;
        }
        else if(run_flag==7){
            write_operation_log(cluster_name,operation_log,argc,argv,"TF_APPLY_FAILED_ROLLED_BACK",73);
            check_and_cleanup(workdir);
            return 73;
        }
        else if(run_flag==9){
            write_operation_log(cluster_name,operation_log,argc,argv,"TF_ROLLBACK_FAILED",75);
            check_and_cleanup(workdir);
            return 75;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
        check_and_cleanup(workdir);
        return 49;
    }

    if(strcmp(argv[1],"payment")==0){
        if(cmd_flag_check(argc,argv,"--od")!=0&&cmd_flag_check(argc,argv,"--month")!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify the new payment method with " WARN_YELLO_BOLD "--od" FATAL_RED_BOLD " or " WARN_YELLO_BOLD "--month" RESET_DISPLAY " .\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--od")==0){
            run_flag=switch_cluster_payment(cluster_name,"od",crypto_keyfile);
        }
        else if(cmd_flag_check(argc,argv,"--month")==0){
            run_flag=switch_cluster_payment(cluster_name,"month",crypto_keyfile);
        }
        if(run_flag==0){
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SWITCH_PAYMENT_FAILED",42);
            check_and_cleanup(workdir);
            return 42;
        }
    }

    if(strcmp(argv[1],"rebuild")==0){
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please wake up the cluster first." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup(workdir);
            return 43;
        }
        if(cmd_flag_check(argc,argv,"--mc")==0){
            run_flag=rebuild_nodes(workdir,crypto_keyfile,"mc");
        }
        else if(cmd_flag_check(argc,argv,"--mcdb")==0){
            run_flag=rebuild_nodes(workdir,crypto_keyfile,"mcdb");
        }
        else if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=rebuild_nodes(workdir,crypto_keyfile,"all");
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify '--mc', '--mcdb', or '--all' as the second param.\n");
            printf("|          Run 'hpcopr help --cmd rebuild' for more details. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"REBUILD_FAILED",34);
            check_and_cleanup(workdir);
            return 34;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",34);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"nfsup")==0){
        if(strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] This command is only applicable to CLOUD_D, CLOUD_F, and CLOUD_G." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FLAG_NOT_APPLICABLE",8);
            check_and_cleanup(workdir);
            return 8;
        }
        if(cluster_empty_or_not(workdir)==0){
            print_empty_cluster_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please wake up the cluster first." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup(workdir);
            return 43;
        }
        if(cmd_keyword_check(argc,argv,"--vol",string_temp)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a positive number as the new volume." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(string_to_positive_num(string_temp)<1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a positive number as the new volume." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        get_state_value(workdir,"shared_volume_gb:",string_temp2);
        if(string_to_positive_num(string_temp)<string_to_positive_num(string_temp2)){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a new volume larger than the previous volume %s." RESET_DISPLAY "\n",string_temp2);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=nfs_volume_up(workdir,crypto_keyfile,string_temp);
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"NFS_VOLUME_UP_FAILED",10);
            check_and_cleanup(workdir);
            return 10;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(argv[1],"sleep")==0){
        if(strcmp(cloud_flag,"CLOUD_F")==0){
            printf(WARN_YELLO_BOLD "[ -INFO- ] Currently Azure (HPC-NOW Code: CLOUD_F) doesn't support this operation." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FUNCTION_UNSUPPORTED",6);
            check_and_cleanup("");
            return 6;
        }
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is " RESET_DISPLAY HIGH_CYAN_BOLD "not running" RESET_DISPLAY FATAL_RED_BOLD ". No need to hibernate." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup("");
            return 43;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup("");
            return 3;
        }
        run_flag=cluster_sleep(workdir,crypto_keyfile);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"wakeup")==0){
        if(strcmp(cloud_flag,"CLOUD_F")==0){
            printf(WARN_YELLO_BOLD "[ -INFO- ] Currently Azure (HPC-NOW Code: CLOUD_F) doesn't support this operation." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FUNCTION_UNSUPPORTED",6);
            check_and_cleanup("");
            return 6;
        }
        if(cluster_full_running_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "fully running" RESET_DISPLAY FATAL_RED_BOLD ". No need to wake up." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"RUNNING_STATE",38);
            check_and_cleanup(workdir);
            return 38;
        }
        if(cluster_state_flag!=0){
            if(cmd_flag_check(argc,argv,"--all")!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "minimal running" RESET_DISPLAY FATAL_RED_BOLD ". Please try " RESET_DISPLAY "\n");
                printf(FATAL_RED_BOLD "|          " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup --all" RESET_DISPLAY FATAL_RED_BOLD " to wake up the whole cluster." RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"RUNNING_STATE",38);
                check_and_cleanup(workdir);
                return 38;
            }
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--all")!=0){
            run_flag=cluster_wakeup(workdir,crypto_keyfile,"minimal");
            sprintf(string_temp,"%s default",argv[1]);
        }
        else{
            run_flag=cluster_wakeup(workdir,crypto_keyfile,"all");
            sprintf(string_temp,"%s all",argv[1]);
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"reconfc")==0||strcmp(argv[1],"reconfm")==0){
        if(cmd_keyword_check(argc,argv,"--conf",string_temp)!=0||cmd_flag_check(argc,argv,"--list")==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Available configuration list:\n|\n");
            if(check_reconfigure_list(workdir)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the list. Have you inited this cluster?" RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
                check_and_cleanup(workdir);
                return 125;
            }
            if(strcmp(argv[1],"reconfc")==0&&check_down_nodes(workdir)!=0&&strcmp(cloud_flag,"CLOUD_B")==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to turn on all the compute nodes first." RESET_DISPLAY "\n");
            }
            if(strcmp(argv[1],"reconfm")==0&&cluster_state_flag==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to wake up the cluster first." RESET_DISPLAY "\n");
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
    }

    if(strcmp(argv[1],"userman")==0){
        if(cmd_keyword_check(argc,argv,"--ucmd",user_cmd)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a user manager command below:\n");
            printf("|          list | add | delete | enable | disable | passwd \n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",user_cmd);
            getchar();
        }
        if(strcmp(user_cmd,"list")!=0&&strcmp(user_cmd,"add")!=0&&strcmp(user_cmd,"delete")!=0&&strcmp(user_cmd,"enable")!=0&&strcmp(user_cmd,"disable")!=0&&strcmp(user_cmd,"passwd")!=0){
            print_usrmgr_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(user_cmd,"list")!=0&&strcmp(cluster_role,"opr")!=0&&strcmp(cluster_role,"admin")!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need the opr or admin role to run " WARN_YELLO_BOLD "--ucmd %s" RESET_DISPLAY FATAL_RED_BOLD " . Current role: " RESET_DISPLAY WARN_YELLO_BOLD "%s " RESET_DISPLAY "\n",user_cmd,cluster_role);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_CLUSTER_ROLE",36);
            check_and_cleanup(workdir);
            return 36;
        }
        if(strcmp(user_cmd,"list")==0){
            run_flag=hpc_user_list(workdir,crypto_keyfile,0);
            if(cluster_state_flag==0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The specified/switched cluster is not running." RESET_DISPLAY "\n");
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;   
        }
        usrmgr_check_flag=usrmgr_prereq_check(workdir,user_cmd);
        if(usrmgr_check_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"USERMAN_PREREQ_CHECK_FAILED",77);
            check_and_cleanup(workdir);
            return 77;
        }
        else if(usrmgr_check_flag==3){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(strcmp(user_cmd,"enable")==0||strcmp(user_cmd,"disable")==0){
            cmd_keyword_check(argc,argv,"-u",user_name);
            run_flag=hpc_user_enable_disable(workdir,SSHKEY_DIR,user_name,crypto_keyfile,user_cmd);
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argc,argv,user_cmd,run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(user_cmd,"add")==0){
            cmd_keyword_check(argc,argv,"-u",user_name);
            cmd_keyword_check(argc,argv,"-p",pass_word);
            run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,user_name,pass_word);
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argc,argv,user_cmd,run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(user_cmd,"delete")==0){
            cmd_keyword_check(argc,argv,"-u",user_name);
            run_flag=hpc_user_delete(workdir,crypto_keyfile,SSHKEY_DIR,user_name);
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argc,argv,user_cmd,run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else{
            cmd_keyword_check(argc,argv,"-u",user_name);
            cmd_keyword_check(argc,argv,"-p",pass_word);
            run_flag=hpc_user_setpasswd(workdir,SSHKEY_DIR,crypto_keyfile,user_name,pass_word);
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
    }

    if(strcmp(argv[1],"destroy")==0){
        if(cmd_flag_check(argc,argv,"--force")==0){
            run_flag=cluster_destroy(workdir,crypto_keyfile,"force");
        }
        else{
            run_flag=cluster_destroy(workdir,crypto_keyfile,"");
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(cluster_state_flag==0){
        if(command_flag==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster is not running. Please wake up first.\n");
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] The switched cluster is not running. Please wake up first.\n");
        }
        if(strcmp(argv[1],"addc")==0){
            printf("|          Command: " RESET_DISPLAY WARN_YELLO_BOLD "hpcopr wakeup --all" RESET_DISPLAY FATAL_RED_BOLD ". Exit now." RESET_DISPLAY "\n");
        }
        else{
            printf("|          Command: " RESET_DISPLAY WARN_YELLO_BOLD "hpcopr wakeup --min | --all" RESET_DISPLAY FATAL_RED_BOLD ". Exit now." RESET_DISPLAY "\n");
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_IS_ASLEEP",43);
        check_and_cleanup(workdir);
        return 43;
    }

    if(strcmp(argv[1],"appman")==0){
        if(cmd_keyword_check(argc,argv,"--acmd",app_cmd)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a valid command: store, avail, build, install, remove.\n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",app_cmd);
            getchar();
        }
        int i=0;
        while(strcmp(app_cmd,appman_commands[i])!=0){
            i++;
            if(i==6){
                break;
            }
        }
        if(i==6){
            printf(FATAL_RED_BOLD "[ FATAL: ] The command " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is incorrect. Valid commands:" RESET_DISPLAY "\n",app_cmd);
            printf(GENERAL_BOLD "|         " RESET_DISPLAY " store, avail, build, install, remove\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(app_cmd,"store")==0){
            run_flag=app_list(workdir,"all",user_name,"",SSHKEY_DIR,"");
        }
        else if(strcmp(app_cmd,"avail")==0){
            run_flag=app_list(workdir,"installed",user_name,"",SSHKEY_DIR,"");
        }
        else{
            if(cmd_keyword_check(argc,argv,"--app",app_name)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify an app name, i.e. " WARN_YELLO_BOLD "of9" FATAL_RED_BOLD " . Exit now." RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                check_and_cleanup(workdir);
                return 9;
            }
            if(strcmp(app_cmd,"check")==0){
                run_flag=app_list(workdir,"check",user_name,app_name,SSHKEY_DIR,"");
            }
            else{
                run_flag=app_operation(workdir,user_name,app_cmd,app_name,SSHKEY_DIR);
            }
        }
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"APPMAN_FAILED",44);
            check_and_cleanup(workdir);
            return 44;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(argv[1],"jobman")==0){
        if(cluster_state_flag==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] No compute node is running." RESET_DISPLAY "\n");
        }
        if(cmd_keyword_check(argc,argv,"--jcmd",job_cmd)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a valid command: submit, list, cancel\n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",job_cmd);
            getchar();
        }
        int i=0;
        while(strcmp(job_cmd,jobman_commands[i])!=0){
            i++;
            if(i==3){
                break;
            }
        }
        if(i==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The command " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is incorrect. Please read the help for details." RESET_DISPLAY "\n",job_cmd);
            printf(GENERAL_BOLD "|          " RESET_DISPLAY " submit, list, cancel\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(job_cmd,"submit")==0){
            run_flag=get_job_info(argc,argv,workdir,user_name,SSHKEY_DIR,crypto_keyfile,&job_info);
            if(run_flag!=0){
                write_operation_log(cluster_name,operation_log,argc,argv,"JOBMAN_FAILED",46);
                check_and_cleanup(workdir);
                return 46;
            }
            run_flag=job_submit(workdir,user_name,SSHKEY_DIR,&job_info);
        }
        else if(strcmp(job_cmd,"list")==0){
            run_flag=job_list(workdir,user_name,SSHKEY_DIR);
        }
        else{
            if(cmd_keyword_check(argc,argv,"--jid",job_id)!=0){
                job_list(workdir,user_name,SSHKEY_DIR);
                printf("[ INPUT: ] Please specify the job id to be canceled: ");
                fflush(stdin);
                scanf("%s",job_id);
                getchar();
            }
            run_flag=job_cancel(workdir,user_name,SSHKEY_DIR,job_id);
        }
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"JOBMAN_FAILED",46);
            check_and_cleanup(workdir);
            return 46;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }
    
    if(strcmp(argv[1],"delc")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=delete_compute_node(workdir,crypto_keyfile,"all");
        }
        else if(cmd_keyword_check(argc,argv,"--nn",node_num_string)==0){
            run_flag=delete_compute_node(workdir,crypto_keyfile,node_num_string);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either '--all' or '--nn NODE_NUM'. Exit now." RESET_DISPLAY "\n");
            run_flag=9;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"addc")==0){
        if(check_down_nodes(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to turn all compute node(s) on before adding new nodes.\n");
            printf("|          Exit now." RESET_DISPLAY "\n");
            check_and_cleanup(workdir);
            return 1;
        }
        if(cmd_keyword_check(argc,argv,"--nn",node_num_string)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a number (range: 1-%d) as the second parameter.\n",MAXIMUM_ADD_NODE_NUMBER);
            printf("|          Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=add_compute_node(workdir,crypto_keyfile,node_num_string);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"shutdownc")==0){
        if(strcmp(cloud_flag,"CLOUD_F")==0){
            printf(WARN_YELLO_BOLD "[ -INFO- ] Currently Azure (HPC-NOW Code: CLOUD_F) doesn't support this operation." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FUNCTION_UNSUPPORTED",6);
            check_and_cleanup("");
            return 6;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=shutdown_compute_nodes(workdir,crypto_keyfile,"all");
        }
        else if(cmd_keyword_check(argc,argv,"--nn",node_num_string)==0){
            run_flag=shutdown_compute_nodes(workdir,crypto_keyfile,node_num_string);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either '--all' or '--nn NODE_NUM'. Exit now." RESET_DISPLAY "\n");
            run_flag=9;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"turnonc")==0){
        if(strcmp(cloud_flag,"CLOUD_F")==0){
            printf(WARN_YELLO_BOLD "[ -INFO- ] Currently Azure (HPC-NOW Code: CLOUD_F) doesn't support this operation." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FUNCTION_UNSUPPORTED",6);
            check_and_cleanup("");
            return 6;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(workdir,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=turn_on_compute_nodes(workdir,crypto_keyfile,"all");
        }
        else if(cmd_keyword_check(argc,argv,"--nn",node_num_string)==0){
            run_flag=turn_on_compute_nodes(workdir,crypto_keyfile,node_num_string);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either '--all' or '--nn NODE_NUM'. Exit now." RESET_DISPLAY "\n");
            run_flag=9;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"reconfc")==0){
        if(strcmp(cloud_flag,"CLOUD_B")==0){
            if(check_down_nodes(workdir)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to turn all compute node(s) on before reconfiguring them.\n");
                printf("|          Exit now." RESET_DISPLAY "\n");
                check_and_cleanup(workdir);
                return 1;
            }
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        cmd_keyword_check(argc,argv,"--conf",string_temp);
        cmd_keyword_check(argc,argv,"--ht",string_temp2);
        run_flag=reconfigure_compute_node(workdir,crypto_keyfile,string_temp,string_temp2);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"reconfm")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        cmd_keyword_check(argc,argv,"--conf",string_temp);
        run_flag=reconfigure_master_node(workdir,crypto_keyfile,string_temp);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    write_operation_log(NULL,operation_log,argc,argv,"FATAL_ABNORMAL",run_flag);
    check_and_cleanup("");
    return 123;
}