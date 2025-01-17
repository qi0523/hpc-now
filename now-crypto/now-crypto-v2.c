/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

/*
 * This is a *psedo* and very *simple* crypto program! Not a real one. If you'd like to use a real
 * crypto program, try AES, or at least DES. Maybe we will switch to it in th future.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CRYPTO_VERSION "0.2.2"
#define ENCRYPTED_FILE_HEADER "---ENCRYPTED BY HPC-NOW SERVICES WITH LOCAL CRYPTO KEYFILE"

int file_encryption_decryption(char* option, char* orig_file, char* target_file, int encrypt_key){
    int real_key=((encrypt_key%1000*17+1301)%100+19)*17%1000;
    int salt_key=real_key%31+3;
    int origc='\0';
    int newc='\0';
    char ch='\0';
    int i=1,j=1;
    int k=0;
    char header_buffer[256]="";
    char header_valid[256]="";
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    if(strcmp(option,"encrypt")!=0&&strcmp(option,"decrypt")!=0){
        return -1;
    }
    file_p=fopen(orig_file,"r");
    if(file_p==NULL){
        return -3;
    }
    if(strcmp(option,"encrypt")==0){
        file_p_2=fopen(target_file,"w+");
        if(file_p_2==NULL){
            fclose(file_p);
            return -5;
        }
        fprintf(file_p_2,"%s v%s---\n",ENCRYPTED_FILE_HEADER,CRYPTO_VERSION);
    }
    else{
        while((ch=fgetc(file_p))!='\n'&&k<256){
            *(header_buffer+k)=ch;
            k++;
        }
        sprintf(header_valid,"%s v%s---",ENCRYPTED_FILE_HEADER,CRYPTO_VERSION);
        if(strncmp(header_buffer,header_valid,58)!=0){
            if(*(header_buffer+0)<'0'||*(header_buffer+0)>'9'){
                fclose(file_p);
                return -7;
            }
            if(*(header_buffer+3)!=','&&*(header_buffer+4)!=','){
                fclose(file_p);
                return -7;
            }
            file_p=fopen(orig_file,"r");
        }
        file_p_2=fopen(target_file,"w+");
        if(file_p_2==NULL){
            fclose(file_p);
            return -5;
        }
    }
    while(!feof(file_p)){
        if(strcmp(option,"encrypt")==0){
            origc=fgetc(file_p);
        }
        else{
            fscanf(file_p,"%d%c",&origc,&ch);
        }
        if(i%salt_key==0){
            if(strcmp(option,"encrypt")==0){
                newc=origc+real_key+(i*j)%salt_key;
            }
            else{
                newc=origc-real_key-(i*j)%salt_key;
            }
            j++;
        }
        else{
            if(strcmp(option,"encrypt")==0){
                newc=origc+real_key+i%salt_key;
            }
            else{
                newc=origc-real_key-i%salt_key;
            }
        }
        i++;
        if(strcmp(option,"encrypt")==0){
            if(i%33==0){
                fprintf(file_p_2,"%d\n",newc);
            }
            else{
                fprintf(file_p_2,"%d,",newc);
            }
        }
        else{
            if(newc!=-1){
                fputc(newc,file_p_2);
            }
            else{
                break;
            }
        }
    }
    fclose(file_p);
    fclose(file_p_2);
    return i;
}

int md5convert(char* md5string){
    int length=strlen(md5string);
    int i,sum=0;
    if(length!=32){
        return -1;
    }
    for(i=0;i<length;i++){
        sum+=*(md5string+i);
    }
    sum=((sum*13+17)*19+31)*37+41;
    return sum;
}

int main(int argc,char *argv[]){
    printf("Version: %s\n",CRYPTO_VERSION);
    int run_flag=0;
    if(argc!=5){
        printf("Error: Not Enough parameters.\n"); 
        return 1;
    }
    if(md5convert(argv[4])==-1){
        printf("Error: Not a valid crypto-key.\n");
        return 3;
    }
    run_flag=file_encryption_decryption(argv[1],argv[2],argv[3],md5convert(argv[4]));
    if(run_flag==-1){
        printf("Error: Option Error.\n");
        return 5;
    }
    else if(run_flag==-3){
        printf("Error: Failed to open the original file.\n");
        return 7;
    }
    else if(run_flag==-5){
        printf("Error: Cannot create the target file.\n");
        return 9;
    }
    else if(run_flag==-7){
        printf("Error: Not a HPC-NOW encrypted file.\n");
        return 11;
    }
    else{
        printf("Done: %d Characters.\n",run_flag);
        return 0; 
    }
}