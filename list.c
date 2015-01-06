/*
	Sena KOÇ 07051102
	Yaşar MATRAÇ 07051124
*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include "apue.h"
#include <time.h>
#include <sys/time.h>

int containShowStartWithDot = -1; // -a
int containShowInColumn = -1;  // -C
int containShowLongListFormat = -1; // -l
int containIsSymbolic = -1; // -L
int containAppendSlash = -1; // -p
int containShowReverse = -1;  // -r
int containRecursive = -1; // -R
int containListByFileSize = -1;  // -S
int containHelp = -1;  // --help
int indexOfPath = -1;  // dosya uzantisi arg indexi
char *path;  // dosya uzantisi

// '.' ile başlayan dosyaları yakalamak için kullanılan fonksiyon
int isStartDot(char* str){
    if(str[0] == '.')
        return 0;
    return -1;
}

// gelen parametreleri yakalamak için kullanılan fonksiyon
void determineContains(char *argv[] , int argc){
	int i = 1;
	while(i < argc){
            if( argv[i][0] != '-' ){
                    indexOfPath = i;
            }
            else{
                if(strcmp(argv[i],"-a") == 0){
                    containShowStartWithDot = 0;
                }
                else if(strcmp(argv[i],"-C") == 0){
                    containShowInColumn = 0;
                }
                else if(strcmp(argv[i],"-l") == 0){
                    containShowLongListFormat = 0;
                }
                else if(strcmp(argv[i],"-L") == 0){
                    containIsSymbolic = 0;
                }
                else if(strcmp(argv[i],"-p") == 0){
                    containAppendSlash = 0;
                }
                else if(strcmp(argv[i],"-r") == 0){
                    containShowReverse = 0;
                }
                else if(strcmp(argv[i],"-R") == 0){
                    containRecursive = 0;
                }
                else if(strcmp(argv[i],"-S") == 0){
                    containListByFileSize = 0;
                }
                else if(strcmp(argv[i],"--help") == 0){
                    containHelp = 0;
                }
            }
            i++;
	}
}

// kullanıcı isimlerini string olarak set eder
char* setPermissionString(__mode_t mode,char *permission){ // stat.st_mode parametresi alır.    
    //----------klasor mu ?------------//
    if(S_ISDIR(mode)){
        strcat(permission,"d");        
    }
    else{        
        strcat(permission,"-");
    }
    //------------USER------------//
    if(S_IRUSR & mode){
        strcat(permission,"r");        
    }
    else{        
        strcat(permission,"-");
    }
    if(S_IWUSR & mode){
        strcat(permission,"w");        
    }
    else{        
        strcat(permission,"-");
    }
    if(S_IXUSR & mode){
        strcat(permission,"x");        
    }
    else{        
        strcat(permission,"-");
    }
    //------------GRUP------------//
    if(S_IRGRP & mode){
        strcat(permission,"r");        
    }
    else{        
        strcat(permission,"-");
    }
    if(S_IWGRP & mode){
        strcat(permission,"w");        
    }
    else{        
        strcat(permission,"-");
    }
    if(S_IXGRP & mode){
        strcat(permission,"x");        
    }
    else{        
        strcat(permission,"-");
    }
    //------------OTHER-----------//
    if(S_IROTH & mode){
        strcat(permission,"r");        
    }
    else{        
        strcat(permission,"-");
    }
    if(S_IWOTH & mode){
        strcat(permission,"w");        
    }
    else{        
        strcat(permission,"-");
    }
    if(S_IXOTH & mode){
        strcat(permission,"x");        
    }
    else{        
        strcat(permission,"-");
    }
    return permission;    
}
// kullanıcı adını isim olarak set eder
void setUserName(__uid_t id, char *userName){
    struct passwd *passptr;
    setpwent();
    while((passptr = getpwent()) != NULL ){
        if(passptr->pw_uid == id){
            strcpy(userName,passptr->pw_name);
        }
    }
    endpwent();    
}
// grub ismini string olarak set eder
void setGroupName(__gid_t id, char *groupName){
    struct group *gropptr;
    setgrent();
    while((gropptr = getgrent()) != NULL ){
        if(gropptr->gr_gid == id){
            strcpy(groupName,gropptr->gr_name);
        }
    }
    endgrent();
}
// zamanı string olarak set eder
void setTime(struct timespec ts, char *timeString){    
    time_t rawtime = ts.tv_sec;
    struct tm *info;
    info = localtime(&rawtime);
    strftime(timeString,64,"%b %d %Y",info );
}
//belirlenen opsiyonlara gore dizini print eder
void printDirent(struct dirent *dir,struct stat st){   
    char printFormat[512];    
    char dirName[200] = "";
    strcpy(dirName,dir->d_name);
    // -p ve directory olup olmadığını kontrol ediliyor
    if(containAppendSlash == 0 && S_ISDIR(st.st_mode) != 0 ){
        strcat(dirName,"/");
    }
    // -L ve sembolik link olup olmadığı kontrol ediliyor.
    if(containIsSymbolic == 0 && S_ISLNK(st.st_mode) != 0 ){
        char realPath[100] = "";
        realpath(dirName,realPath);
        strcat(dirName,realPath);
    }
    // uzun formatta basmak için
    if(containShowLongListFormat == 0){        
        char permissions[11] = ""; 
        char username[50] = "";
        char groupname[50] = "";
        char timestring[64] = "";        
        setPermissionString(st.st_mode,permissions);
        setUserName(st.st_uid,username);
        setGroupName(st.st_gid,groupname);
        setTime(st.st_mtim,timestring);
        sprintf(printFormat,"%10s %3d %7s %7s %10d %10s %.20s\n",permissions,(int)st.st_nlink,groupname,username,(int)st.st_size,timestring,dirName);
    }
    // kolonlar halinde basılıyor
    else if(containShowInColumn == 0){
        strcpy(printFormat,dirName);
        strcat(printFormat,"\t");
    }
    else{
        strcpy(printFormat,dirName);
        strcat(printFormat,"\n");
    }
    printf("%s",printFormat);
}


// boyuta göre sırala
void sortBySize(struct dirent ***namelist,int n, char *dirpath){
    struct dirent **tempDir;
    long tempSize = 0;
    long *sizes = (long *) malloc(n*sizeof(long));
    struct stat st;
    int i = 0;
    for(;i < n ; i++){
        char filePath[512] = "";   
        sprintf(filePath,"%s/%s",dirpath,(*namelist)[i]->d_name); 
        stat(filePath,&st);
        sizes[i] = (long)st.st_size;        
    }
    i = 0;
    int j = 1;
    for(;i < n-1 ; i++){
        j = i + 1;
        for(;j<n;j++){
            if(sizes[i] > sizes[j] ){
                *tempDir = (*namelist)[i];
                (*namelist)[i] = (*namelist)[j];
                (*namelist)[j] = *tempDir;
                tempSize = sizes[i];
                sizes[i] = sizes[j];
                sizes[j] = tempSize;
            }
        }
    }    
}

void visit(char *currentPath){
    struct dirent **namelist;
    int n;
    n = scandir(currentPath, &namelist, NULL, alphasort);    
    if (n < 0){
        printf("%s acilamıyor\n", currentPath);
        exit(0);
    }
    if(containListByFileSize == 0){
        sortBySize(&namelist,n,currentPath);
    }    
    int begin = 0;
    int i = 0;
    // tersine veya düz göstermek için ayarlamalar
    if(containShowReverse == 0){
        begin = n-1;
        i = -1;        
    }
    else{
        begin = 0;
        i = 1 ;
    }    
    while (begin >= 0 && begin < n) {
        struct stat st;
        char filePath[512] = "";   
        sprintf(filePath,"%s/%s",currentPath,namelist[begin]->d_name); 
        stat(filePath,&st);
        // . ile başlayanları gösterir  // . ile başlayanları göstermez
        if(containShowStartWithDot == 0 || isStartDot(namelist[begin]->d_name) != 0 ){           
            printDirent(namelist[begin],st);   
            // recursive listeleme istendiğinde bu bloga girer
            if(S_ISDIR(st.st_mode) != 0 && containRecursive == 0 && isStartDot(namelist[begin]->d_name)){
                printf("--------------\n");
                printf("%s: \n",namelist[begin]->d_name);            
                visit(filePath);
                printf("--------------\n");
            }        
        }       
	begin = begin + i;
    }
    free(namelist);
}



int main(int argc, char *argv[]){        
	determineContains(argv,argc);
	if(containHelp == 0){
            char help[1000] = "-a: . ile başlayan girişleri de yazdır.\n"
                            "-C: girişleri kolonlar halinde yazdır\n" 
                            "-l : uzun listeleme formatını kullan\n"
                            "-L: Eğer parametre sembolik link ise linkin gösterdiği "
                            "gerçek dosya yada dizin adını yazdır\n"
                            "-p : Eğer dosya adı bir dizin ise sonuna / ekle\n"
                            "-r: sıralamayı tersine çevir\n"
                            "-R : alt dizinleri recursive olarak yazdır\n"
                            "-S : dosya büyüklüğüne göre sırala\n";
            printf("%s",help);
            return;            
        }
        // hiç parametre girilmemiş ise bulunduğu dizini listelicek
        if(indexOfPath == -1){
            char pwd[200] = "";
            getcwd(pwd,sizeof(pwd));
            path = pwd;
        }
        else{
            path = argv[indexOfPath];
        }
        visit(path);
	exit(0);
}
