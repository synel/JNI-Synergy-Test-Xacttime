/**
 * @chinese
 * @file   file.c
 * @author 胡俊远
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  文件相关处理模块
 * @endchinese
 *
 * @english
 * @file   file.c
 * @author Hu Junyuan
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  file handling module
 * @endenglish
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "file.h"
#include "stringchar.h"
#include "config.h"
#include "debug.h"
#include "public.h"
#ifndef _ARM_FROCK
#include "../serial/watchdog.h"
#endif

/**
 * @chinese
 * 获得文件大小
 *
 * @param filename 文件路径
 *
 * @return 成功-文件大小，失败-(-1)
 * @endchinese
 *
 * @english
 * recv data
 *
 * @param filename file path
 *
 * @return if Sucess,return file size,if fail,return -1.
 * @endenglish
 *
 */
 long get_file_size(char *filename)
{
    struct stat f_stat;

    if(access(filename,F_OK)!=0)                      //if the directory is not exit
    {
        return ERROR;
    }
    if(stat(filename,&f_stat) < 0)
    {
        return ERROR;
    }

    return (long)f_stat.st_size;
}

/**
 * @chinese
 * 移动文件
 *
 * @param src 源文件
 * @param dest 目标文件
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * move file
 *
 * @param src src path
 * @param dest destination path
 *
 * @return Success-SUCCESS,Failure-ERROR
 * @endenglish
 *
 */
int mv_file(char *src ,char *dest)
{
    unsigned int CMAXLEN = 1024, nbytes;
    char buf[CMAXLEN];
    FILE *ffp, *tfp;

    if((ffp = fopen(src, "rb")) == NULL)
    {
        return ERROR;
    }
    if((tfp = fopen(dest, "wb")) == NULL)
    {
        fclose(ffp);
        return ERROR;
    }

    while(!feof(ffp) && !ferror(ffp))
    {
        nbytes = fread(buf, sizeof(char), CMAXLEN, ffp);

        if(fwrite(buf, 1U, nbytes, tfp) != nbytes)
        {
            fclose(ffp);
            fclose(tfp);
            return ERROR;
        }

        if(nbytes < CMAXLEN)
            break;
    }

    fclose(ffp);
    fclose(tfp);
    remove(src);

    return SUCCESS;
}

/*--------------------------------------------------------------------------*
@Function		: cp_file - copy file
@Include      	: "public.h"
@Description		: from：source file；
            : to : destination file copy the source file to destination file,if the destination is not exit,create it.After copying, delete source files
@Return Value	: Success 0 .Failure -1
@Create time		: 2009-06-15
*---------------------------------------------------------------------------*/
/**
 * @chinese
 * 复制文件
 *
 * @param src 源文件
 * @param dest 目标文件
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * copy file
 *
 * @param src src path
 * @param dest destination path
 *
 * @return Success-0,Failure-ERROR
 * @endenglish
 *
 */
int cp_file(char *from ,char *to)
{
    unsigned int CMAXLEN = 1024, nbytes;
    FILE *ffp, *tfp;
    char buf[CMAXLEN];

    if((ffp = fopen(from, "rb")) == NULL)
    {
        return -1;
    }
    creat_directory(to);
    if((tfp = fopen(to, "wb")) == NULL)
    {
        fclose(ffp);
        return -1;
    }

    while(!feof(ffp) && !ferror(ffp))
    {
        nbytes = fread(buf, 1, CMAXLEN, ffp);
        if(fwrite(buf, 1U, nbytes, tfp) != nbytes)
        {
            fclose(ffp);
            fclose(tfp);
            return -1;
        }
        if(nbytes < CMAXLEN)
            break;
    }

    fclose(tfp);
    fclose(ffp);
    return 0;
}


/**
 * @chinese
 *创建文件夹,函数不创建路径内以'/'分割的的最后一个目录，故在此目录的尾部添加一个虚拟文件。
 *
 * @param path 文件路径
 *
 * @return 成功-0,失败-（-1）
 * @endchinese
 *
 * @english
 * create directory, this function won't create a directory which following the last `/` seperator,so so please adding  a virtual file to the tailing directory.
 *
 * @param path file path
 *
 * @return success:0,fail:-1
 * @endenglish
 *
 */
int creat_directory(char *path)
{
    char buff[512],buff1[512],*p=NULL,*p1=NULL;
    int len=0,i=0;

    if(path == NULL || strlen(path)>sizeof(buff))
        return FALSE;

    p1=path;
    p=buff;
    memset(buff,0,sizeof(buff));
    cut_rn(p1);

    while(*p1)
    {
        if(*p1=='/')
        {
            if(len==1)
            	p1++;
            else
                *p++=*p1++;
            len=1;
        }
        else
        {
            len=0;
            *p++=*p1++;
        }
        i++;
        if(i>=sizeof(buff))
            break;
        // plog("%c,%d,%c,%d\n",*p,*p,*p1,*p1);
    }

    p=buff;
    p1=buff1;
    memset(buff1,0,sizeof(buff1));

    if(*p=='/')
    {
    	*p1='/';
    	p++;
    }//if the first character is '/',it is root.the pwd should also be obtained
    while((len=strcspn(p,"/")))
    {
        if(len==(int)strlen(p))
            break;
        strncat(p1,p,len+1);
        if(access(p1,F_OK)!=0)                      //if the directory is not exit
        {
            if(mkdir(p1,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH)!=0)
            {

                return -1;  //create directory
            }
        }
        p+=len;
        p++;
    }
    //    printf("==========8\n");
    return TRUE;
}


/**
 * @chinese
 * 文件编码转换
 *
 * @param path 转换文件
 *
 * @return 是-TRUE,不是-FALSE
 * @endchinese
 *
 * @english
 * File encoding conversion
 *
 * @param path file's path
 *
 * @return Yes-TRUE,No-FALSE
 * @endenglish
 *
 */
int transform_code(char *path)
{
    int retval=0;
    char file_name[256];

    if(path == NULL)
    {
        return FALSE;
    }
    retval = access(path,F_OK);
    if(retval != 0)
        return FALSE;
#ifndef _ARM_A23
    sprintf(file_name,"iconv -f GBK -t UTF-8 %s > /tmp/~.wts",path);
    system(file_name);
    rename("/tmp/~.wts",path);
#else
    sprintf(file_name,"su -c \"busybox iconv -f GBK -t UTF-8 %s\" > /mnt/obb/tmp/~.wts",path);
    system(file_name);
    rename("/mnt/obb/tmp/~.wts",path);
#endif
    return TRUE;
}

/**
 *count up the number of file in directory
 *
 * @param photopath directory file
 *
 * @return Success number of file，Failure 0
 */
int count_file_number(char *photopath)
{
    int total=0;
    DIR *db;
    char filename[512];
    struct dirent *ptr;
    db=opendir(photopath);
    if(db==NULL)
    {
        return 0;
    }

    while ((ptr=readdir(db)))
    {
#ifndef _ARM_FROCK
#ifndef _PC44
    	send_watchdog_info(COUNT_FILE);
#endif
#endif
        if((strcmp(ptr->d_name,".")==0)||(strcmp(ptr->d_name,"..")==0))
        {
            continue;
        }
        memset(filename,0,sizeof(filename));
        if(ptr->d_type == DT_DIR)
        {
            sprintf(filename,"%s/%s",photopath,ptr->d_name); //get second level directory
            total += count_file_number(filename);
        }
        else
        {
            total++;
        }
    }
    closedir(db);
    return total;
}


/**
 * @chinese
 *测试是否为文件夹
 *
 * @param path 文件路径
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * test whether path is a directory
 *
 * @param path file's path
 *
 * @return success:0,fail:-1
 * @endenglish
 *
 */
int check_directory(char *path)
{
    struct stat buf;

    if(lstat(path,&buf)<0)
    {
        return ERROR;
    }
    if(S_ISDIR(buf.st_mode)||S_ISLNK(buf.st_mode))
    {
        return SUCCESS;
    }
    return ERROR;
}

int open_access_path(char *newpath,DIR *db)
{
    if(db == NULL)
    {
        return FALSE;
    }
    if(access(newpath,F_OK) != 0)
    {
        if(creat_directory(newpath) == 0)
        {
            closedir(db);
            return FALSE;
        }
    }
    return TRUE;
}
/*
 * 拷贝目录下的文件
 *
 *@param oldpath 拷贝的目录的路径
 *@return 成功：0；失败：-1；
 */
int safe_cp_file_file(char * oldpath,char *newpath)
{
    int retval = -1;
    static int count = 0;
    struct dirent *p;
    DIR *db;
    char path2[512],path3[512];

    if(check_directory(oldpath) == ERROR)
    {
        return cp_file(oldpath,newpath);
    }
    db = opendir(oldpath);
    retval = open_access_path(newpath,db);
    if(retval == FALSE)
        return ERROR;

    while ((p=readdir(db)))
    {
        if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
        {
            continue;
        }
        else
        {
            memset(path2,0,sizeof(path2));
            memset(path3,0,sizeof(path3));
            sprintf(path2,"%s/%s",oldpath,p->d_name);
            sprintf(path3,"%s/%s",newpath,p->d_name);
        }
        if(check_directory(path2) == SUCCESS)
            continue;
        count++;
        //safe_cp_file_file(path2,path3);
        cp_file(path2,path3);
        if(count>2000)
        {
            count=0;
            sync();
        }
    }
    closedir(db);
    return 0;
}
/*
 * 拷贝目录下的文件夹和文件
 *
 *@param oldpath 拷贝的目录的路径
 *@return 成功：0；失败：-1；
 */
int safe_cp_file_both(char * oldpath,char *newpath)
{
    int retval = -1;
    static int count = 0;
    struct dirent *p;
    DIR *db;
    char path2[512],path3[512];

    if(check_directory(oldpath) == ERROR)
    {
        return cp_file(oldpath,newpath);
    }
    db = opendir(oldpath);
    retval = open_access_path(newpath,db);
    if(retval == FALSE)
        return ERROR;

    while ((p=readdir(db)))
    {
        if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
        {
            continue;
        }
        else
        {
            memset(path2,0,sizeof(path2));
            memset(path3,0,sizeof(path3));
            sprintf(path2,"%s/%s",oldpath,p->d_name);
            sprintf(path3,"%s/%s",newpath,p->d_name);
        }
        count++;
        safe_cp_file_both(path2,path3);
        if(count>2000)
        {
            count=0;
            sync();
        }
    }
    closedir(db);
    return 0;
}

/*
 * 拷贝目录下的文件夹
 *
 *@param oldpath 拷贝的目录的路径
 *@return 成功：0；失败：-1；
 */
int safe_cp_file_folder(char * oldpath,char *newpath)
{
    int retval = -1;
    static int count = 0;
    struct dirent *p;
    DIR *db;
    char path2[512],path3[512];

    if(check_directory(oldpath) == ERROR)
    {
        return cp_file(oldpath,newpath);
    }

    db = opendir(oldpath);
    retval = open_access_path(newpath,db);
    if(retval == FALSE)
        return ERROR;

    while ((p=readdir(db)))
    {
        if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
        {
            continue;
        }
        else
        {
            memset(path2,0,sizeof(path2));
            memset(path3,0,sizeof(path3));
            sprintf(path2,"%s/%s",oldpath,p->d_name);
            sprintf(path3,"%s/%s",newpath,p->d_name);
        }
        if(check_directory(path2) == ERROR)
            continue;
        count++;
        safe_cp_file_both(path2,path3);
        if(count>2000)
        {
            count=0;
            sync();
        }
    }
    closedir(db);
    return 0;
}
/**
 * @chinese
 *拷贝文件/文件夹
 *
 * @param oldpath 拷贝的文件路径
 * type
 * 1 拷贝目录下的文件,目标目录请带着文件名
 * 2 拷贝目录下的文件夹
 * 3 拷贝目录下的文件和文件夹
 * 4 带着当前目录一起拷贝
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * copy file safely
 *
 * @param oldpath file path which file will be deleted
 *
 * @return success:0,fail:-1
 * @endenglish
 *
 */
int safe_cp_file(char * oldpath,char *newpath,int type)
{
    int retval = -1, tmplen = 0, len = 0;
    char path5[512],path4[512],path3[512],q[128];
    if((strlen(oldpath) >= 512) || (oldpath == NULL))
    {
        return FALSE;  //Prevent stack overflow
    }
    switch(type)
    {
    case 1:
        retval = safe_cp_file_file(oldpath,newpath);
        break;
    case 2:
        retval = safe_cp_file_folder(oldpath,newpath);
        break;
    case 3:
        retval = safe_cp_file_both(oldpath,newpath);
        break;
    case 4:
        {
            memset(path3,0,sizeof(path3));
            memset(path4,0,sizeof(path4));
            memset(path5,0,sizeof(path5));
            memset(q,0,sizeof(q));

            sprintf(path4,"%s",oldpath);
            if(path4[0]=='/')
                len=len+1;
            while(1)
            {
                tmplen=strcspn(path4+len, "/");
                if(tmplen == 0)
                {
                    memcpy(path5,q,sizeof(q));
                    sprintf(path3,"%s/%s",newpath,path5);
                    break;
                }
                memcpy(q,path4+len,tmplen);
                len=tmplen+len+1;
            }
            retval = safe_cp_file_both(oldpath,path3);
        }
        break;
    default:
        break;
    }
    return retval;
}
/*
 * 删除目录下的文件
 *
 *@param oldpath 删除的目录的路径
 *@return 成功：0；失败：-1；
 */
int safe_rm_file_file(char *oldpath)
{
    char path1[512],path2[512];
    DIR *db;
    struct dirent *p;
    int len=0,rmdirflag=0;
    static int rmcurentcount=0;

    memset(path1,0,sizeof(path1));
    memset(path2,0,sizeof(path2));
    strcpy(path2,oldpath);
    len=strlen(path2);
    if((len-->2)&&(path2[len--]=='*')&&(path2[len]=='/'))//if parameter is similar to 'note/frame/*',do not delete frame
    {
        len++;
        len++;
        rmdirflag=1;
        path2[len]=0;
        strncpy(path1,oldpath,len-2);
    }
    else
    {
        len++;
        len++;
        if(check_directory(oldpath) == ERROR)
        {
            rmcurentcount++;
            if(rmcurentcount>2000)
            {
                sync();
                rmcurentcount=0;
            }
            return remove(oldpath);
        }
        else
        {
            strcpy(path1,oldpath);
        }
    }

    db=opendir(path1);
    if(db==NULL)
    {
        return ERROR;
    }
    while((p=readdir(db)))
    {
        if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
        {
            continue;
        }
        else
        {
            memset(path2,0,sizeof(path2));
            sprintf(path2,"%s/%s",path1,p->d_name);
        }
        if(check_directory(path2) == SUCCESS)
            continue;
        //safe_rm_file_file(path2);
        rmcurentcount++;
        if(rmcurentcount>2000)
        {
            sync();
            rmcurentcount=0;
        }
        remove(path2);
    }
    closedir(db);
    return 0;
}
/*
 * 删除目录下的文件夹和文件
 *
 *@param oldpath 删除的目录的路径
 *@return 成功：0；失败：-1；
 */
int safe_rm_file_both(char *oldpath,int type)
{
    char path1[512],path2[512];
    DIR *db;
    struct dirent *p;
    int len=0,rmdirflag=0;
    static int rmcurentcount=0;

    memset(path1,0,sizeof(path1));
    memset(path2,0,sizeof(path2));
    strcpy(path2,oldpath);
    len=strlen(path2);
    if((len-->2)&&(path2[len--]=='*')&&(path2[len]=='/'))//if parameter is similar to 'note/frame/*',do not delete frame
    {
        len++;
        len++;
        rmdirflag=1;
        path2[len]=0;
        strncpy(path1,oldpath,len-2);
    }
    else
    {
        len++;
        len++;
        if(check_directory(oldpath) == ERROR)
        {
            rmcurentcount++;
            if(rmcurentcount>2000)
            {
                sync();
                rmcurentcount=0;
            }
            return remove(oldpath);
        }
        else
        {
            strcpy(path1,oldpath);
        }
    }

    while(count_file_number(path1))
    {
        memset(path2,0,sizeof(path2));
        db=opendir(path1);
        if(db==NULL)
        {
            return ERROR;
        }
        while((p=readdir(db)))
        {
            if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
            {
                continue;
            }
            else
            {
                memset(path2,0,sizeof(path2));
                sprintf(path2,"%s/%s",path1,p->d_name);
            }
            safe_rm_file_both(path2,4);
        }
        closedir(db);
    }
    if((rmdirflag==0) && (type == 4))
    {
        return remove(path1);
    }
    return 0;
}
/*
 * 删除目录下的文件夹
 *
 *@param oldpath 删除的目录的路径
 *@return 成功：0；失败：-1；
 */
int safe_rm_file_folder(char *oldpath)
{
    char path1[512],path2[512];
    DIR *db;
    struct dirent *p;
    int len=0,rmdirflag=0;
    static int rmcurentcount=0;

    memset(path1,0,sizeof(path1));
    memset(path2,0,sizeof(path2));
    strcpy(path2,oldpath);
    len=strlen(path2);
    if((len-->2)&&(path2[len--]=='*')&&(path2[len]=='/'))//if parameter is similar to 'note/frame/*',do not delete frame
    {
        len++;
        len++;
        rmdirflag=1;
        path2[len]=0;
        strncpy(path1,oldpath,len-2);
    }
    else
    {
        len++;
        len++;
        if(check_directory(oldpath) == ERROR)
        {
            rmcurentcount++;
            if(rmcurentcount>2000)
            {
                sync();
                rmcurentcount=0;
            }
            return remove(oldpath);
        }
        else
        {
            strcpy(path1,oldpath);
        }
    }

    db=opendir(path1);
    if(db==NULL)
    {
        return ERROR;
    }
    while((p=readdir(db)))
    {
        if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
        {
            continue;
        }
        else
        {
            memset(path2,0,sizeof(path2));
            sprintf(path2,"%s/%s",path1,p->d_name);
        }

        if(check_directory(path2) == ERROR)
            continue;
        safe_rm_file_both(path2,4);
    }
    closedir(db);
    return 0;
}
/**
 * @chinese
 *删除文件夹/文件
 *
 * @param oldpath 删除的文件路径
 * * type
 * 1 删除目录下的文件
 * 2 删除目录下的文件夹
 * 3 删除目录下的文件和文件夹
 * 4 带着当前目录一起删除
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * rm directory/file safely
 *
 * @param oldpath file path which file will be deleted
 *
 * @return success:0,fail:-1
 * @endenglish
 *
 */
int safe_rm_file(char *oldpath,int type)
{
    int retval = -1;
    if((strlen(oldpath)>=512)||(oldpath==NULL))
    {
        return FALSE;  //Prevent stack overflow
    }
    switch(type)
    {
    case 1:
        retval = safe_rm_file_file(oldpath);
        break;
    case 2:
        retval = safe_rm_file_folder(oldpath);
        break;
    case 3:
        retval = safe_rm_file_both(oldpath,type);
        break;
    case 4:
        retval = safe_rm_file_both(oldpath,type);
        break;
    default:
        break;
    }
    return retval;
}

/**
 * @chinese
 *获取文件夹下目录结构列表
 * @param path 文件路径
 *
 * @return
 * @endchinese
 *
 * @english
 * get directory list
 * @param path directory path
 *
 * @return
 * @endenglish
 */
FILE* get_directory_list(char *path)
{
    DIR *db;
    struct dirent *p;
#ifndef _ARM_A23
    char namebuf[128],*tmp_file = "/tmp/~~.tmp";
#else
    char namebuf[128],*tmp_file = "/mnt/obb/tmp/~~.tmp";
#endif
    FILE *tmp;
    tmp=fopen(tmp_file,"w");
    if(tmp==NULL)
    {
        return NULL;
    }
    db=opendir(path);
    if(db==NULL)
    {
        return NULL;
    }
    while ((p=readdir(db)))
    {
        if(strcmp(p->d_name,".")==0||strcmp(p->d_name,"..")==0)
            continue;
        else
        {
            memset(namebuf,0,128);
            sprintf(namebuf,"%s/%s",path,p->d_name);
            if(check_directory(namebuf) == TRUE)
            {
                memset(namebuf,0,128);
                sprintf(namebuf,"%s/",p->d_name);
            }
            else
            {
                memset(namebuf,0,128);
                sprintf(namebuf,"%s",p->d_name);
            }
        }
        fprintf(tmp,"%s\r\n",namebuf);
    }
    closedir(db);
    fclose(tmp);
    tmp=fopen(tmp_file,"r");
    return tmp;
}

/**
 * @chinese
 * 根据条数处理记录文件,从源文件中拷贝number条数据到目标文件中
 *
 * @param src 源文件
 * @param dest 目标文件
 * @param number 操作的数据条数
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * copy file
 *
 * @param src src path
 * @param dest destination path
 *
 * @return Success-SUCCESS,Failure-ERROR
 * @endenglish
 *
 */
int dial_record_file(char *src ,char *dst,int number)
{
    FILE *srcfile=NULL,*dstfile=NULL;    
    int cur_num=number;
    char buf[1024];

    if(access(src,F_OK)!=0)
    {
        return ERROR;
    }
    srcfile=fopen(src,"r");
    dstfile=fopen(dst,"w");
    if(srcfile==NULL||dstfile==NULL)
    {
        if(srcfile==NULL)fclose(srcfile);
        if(dstfile==NULL)fclose(dstfile);
        return ERROR;
    }
    while(fgets(buf,sizeof(buf),srcfile)&&cur_num>0)
    {
        fputs(buf,dstfile);
        cur_num--;
    }

    fclose(srcfile);
    fclose(dstfile);
    return number-cur_num;//返回拆分的行数
}

/**
 * @chinese
 * 删除源文件中记录,从源文件中删除number条数据
 *
 * @param src 源文件
 * @param number 删除的数据条数
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * copy file
 *
 * @param src src path
 * @param dest destination path
 *
 * @return Success-SUCCESS,Failure-ERROR
 * @endenglish
 *
 */
int delete_record_file(char *src ,int number)
{
    FILE *srcfile=NULL,*tmpfile=NULL;
    char buf[1024];
    char tmppath[256];
    int cur_num=0,ret=0;


    if(src==NULL || number<0)
    {
        return ERROR;
    }

    srcfile=fopen(src,"r");
    memset(tmppath,0,sizeof(tmppath));
    sprintf(tmppath,"%s_tmp",src);
    tmpfile=fopen(tmppath,"w");

    if(tmpfile==NULL||srcfile==NULL)
    {
        if(srcfile==NULL)
            fclose(srcfile);

        if(tmpfile==NULL)
            fclose(tmpfile);
        return ERROR;
    }
    memset(buf,0,sizeof(buf));
    while(fgets(buf,sizeof(buf),srcfile))
    {
        if(cur_num++<number)
        {
            continue;
        }
        ret=fputs(buf,tmpfile);
        if(ret<0)
            break;
        memset(buf,0,sizeof(buf));
    }
    fclose(srcfile);
    fclose(tmpfile);
    if(ret<0)
    {
        return ERROR;
    }
    ret = rename(tmppath,src);
#if 0
    ret=mv_file(tmppath,src);
#endif
    if(ret == ERROR)
    {
        return ERROR;
    }

    return cur_num;
}


/**
 * @chinese
 * 统计文件行数
 *
 * @param src 源文件名
 *
 * @return 文件行数
 * @endchinese
 *
 */
int count_file_line(char * filename)
{
    FILE * fd = 0;
    char buf[1024];
    int lines = 0;

    if(filename == NULL || (access(filename,F_OK)!=0))
    {
        return lines;
    }
    memset(buf,0,sizeof(buf));
    fd = fopen(filename, "r");
    if(!fd)
    {
        return lines;
    }
    while(!feof(fd))
    {
        if(fgets(buf, sizeof(buf), fd) == NULL)
            break;
        lines++;
    }
    fclose(fd);
    return lines;
}


/**
 * @chinese
 * 调用system执行命令
 *
 * @param src 执行的指令
 *
 * @return
 * @endchinese
 *
 */
typedef void (*sighandler_t)(int);
int pox_system(const char *cmd_line)
{
    int ret = 0;
    sighandler_t old_handler;
    old_handler = signal(SIGCHLD, SIG_DFL);
    ret = system(cmd_line);
    signal(SIGCHLD, old_handler);
    return ret;
}






/**
 * @chinese
 * 复制文件
 *
 * @param src 源文件
 * @param dest 目标文件
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * copy file
 *
 * @param src src path
 * @param dest destination path
 *
 * @return Success-0,Failure-ERROR
 * @endenglish
 *写入数据是，在0，3，6字节处，分别写入0，其他数据依次写入。
 */
int cp_file_jiami(char *from ,char *to)
{
    unsigned int CMAXLEN = 1024, nbytes;
    FILE *ffp, *tfp;
    char buf[CMAXLEN],buf2[CMAXLEN];
    int i=0;
    int ret;

    if((ffp = fopen(from, "rb")) == NULL)
    {
        return -1;
    }
    creat_directory(to);
    if((tfp = fopen(to, "wb")) == NULL)
    {
        fclose(ffp);
        return -1;
    }

    while(!feof(ffp) && !ferror(ffp))
    {
        nbytes = fread(buf, 1, CMAXLEN, ffp);
        for(i=0;i<nbytes;i++)
            buf2[i]=buf[nbytes-1-i];
        if(fwrite(buf2, 1U, nbytes, tfp) != nbytes)
        {
            fclose(ffp);
            fclose(tfp);
            return -1;
        }
        if(nbytes < CMAXLEN)
            break;
    }

    fclose(tfp);
    fclose(ffp);
    return 0;
}




/*
 * 拷贝目录下的文件
 *
 *@param oldpath 拷贝的目录的路径
 *@return 成功：0；失败：-1；
 */
int safe_cp_file_file_jiami(char * oldpath,char *newpath)
{
    int retval = -1;
    static int count = 0;
    struct dirent *p;
    DIR *db;
    char path2[512],path3[512];

    if(check_directory(oldpath) == ERROR)
    {
        return cp_file_jiami(oldpath,newpath);
    }
    db = opendir(oldpath);
    retval = open_access_path(newpath,db);
    if(retval == FALSE)
        return ERROR;

    while ((p=readdir(db)))
    {
        if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
        {
            continue;
        }
        else
        {
            memset(path2,0,sizeof(path2));
            memset(path3,0,sizeof(path3));
            sprintf(path2,"%s/%s",oldpath,p->d_name);
            sprintf(path3,"%s/%s",newpath,p->d_name);
        }
        if(check_directory(path2) == SUCCESS)
            continue;
        count++;
        //safe_cp_file_file(path2,path3);
        cp_file_jiami(path2,path3);
        if(count>2000)
        {
            count=0;
            sync();
        }
    }
    closedir(db);
    return 0;
}


/*
 * 拷贝目录下的文件夹和文件
 *
 *@param oldpath 拷贝的目录的路径
 *@return 成功：0；失败：-1；
 */
int safe_cp_file_both_jiami(char * oldpath,char *newpath)
{
    int retval = -1;
    static int count = 0;
    struct dirent *p;
    DIR *db;
    char path2[512],path3[512];

    if(check_directory(oldpath) == ERROR)
    {
        return cp_file_jiami(oldpath,newpath);
    }
    db = opendir(oldpath);
    retval = open_access_path(newpath,db);
    if(retval == FALSE)
        return ERROR;

    while ((p=readdir(db)))
    {
        if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
        {
            continue;
        }
        else
        {
            memset(path2,0,sizeof(path2));
            memset(path3,0,sizeof(path3));
            sprintf(path2,"%s/%s",oldpath,p->d_name);
            sprintf(path3,"%s/%s",newpath,p->d_name);
        }
        count++;
        safe_cp_file_both_jiami(path2,path3);
        if(count>2000)
        {
            count=0;
            sync();
        }
    }
    closedir(db);
    return 0;
}

/**
 * @chinese
 *拷贝文件/文件夹
 *
 * @param oldpath 拷贝的文件路径
 * type
 * 1 拷贝目录下的文件,目标目录请带着文件名
 * 2 拷贝目录下的文件夹
 * 3 拷贝目录下的文件和文件夹
 * 4 带着当前目录一起拷贝
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * copy file safely
 *
 * @param oldpath file path which file will be deleted
 *
 * @return success:0,fail:-1
 * @endenglish
 *
 */
int safe_cp_file_jiami(char * oldpath,char *newpath,int type)
{
    int retval = -1, tmplen = 0, len = 0;
    char path5[512],path4[512],path3[512],q[128];
    if((strlen(oldpath) >= 512) || (oldpath == NULL))
    {
        return FALSE;  //Prevent stack overflow
    }
    //printf("%s,%s,%s,%d\n",__func__,oldpath,newpath,type);
    switch(type)
    {
    case 1:
        retval = safe_cp_file_file(oldpath,newpath);
        break;
    case 2:
        retval = safe_cp_file_folder(oldpath,newpath);
        break;
    case 3:
        retval = safe_cp_file_both_jiami(oldpath,newpath);
        break;
    case 4:
        {
            memset(path3,0,sizeof(path3));
            memset(path4,0,sizeof(path4));
            memset(path5,0,sizeof(path5));
            memset(q,0,sizeof(q));

            sprintf(path4,"%s",oldpath);
            if(path4[0]=='/')
                len=len+1;
            while(1)
            {
                tmplen=strcspn(path4+len, "/");
                if(tmplen == 0)
                {
                    memcpy(path5,q,sizeof(q));
                    sprintf(path3,"%s/%s",newpath,path5);
                    break;
                }
                memcpy(q,path4+len,tmplen);
                len=tmplen+len+1;
            }
            retval = safe_cp_file_both(oldpath,path3);
        }
        break;
    default:
        break;
    }
    return retval;
}


//path ---->directory

int path_to_directory(char *path,char *directory)
{
    char *str=NULL;
    int len=0;
    str=strrchr(path,'/');
    if(str)
        if(path==str)
            len=1;
    else
        len=str-path+1;
    else len=strlen(path);
    strncpy(directory,path,len);
    directory[len]=0;
    return len;
}



#define DWORD unsigned long
#define BYTE unsigned char

int fileEncryptionAndDecryption(char *szSource,char *szDestination)
{
    FILE *hSource=NULL;
    FILE *hDestination=NULL;

    DWORD dwKey=0xfcba0000;

    char* pbBuffer=NULL;
    DWORD dwBufferLen=sizeof(DWORD);
    DWORD dwCount;
    DWORD dwData;

    if(szSource==NULL||szDestination==NULL)
    {
        return -1;
    }

    hSource = fopen(szSource,"rb");// 打开源文件.
    if (hSource==NULL)
    {
        printf("open Source File error !");
        return -1 ;
    }

    hDestination = fopen(szDestination,"wb");    //打开目标文件
    if (hDestination==NULL)
    {
        if(hSource!=NULL)
        {
            fclose(hSource);
        }
        printf("open Destination File error !");
        return -1 ;
    }

    //分配缓冲区
    pbBuffer=(char* )malloc(dwBufferLen);
    if(pbBuffer==NULL)
    {
        //关闭文件、释放内存
        fclose(hSource);
        fclose(hDestination);
        return -1;
    }
    do {
        // 从源文件中读出dwBlockLen个字节
        dwCount = fread(pbBuffer, 1, dwBufferLen, hSource);	
        //加密数据
        dwData = *(DWORD*)pbBuffer;  //char* TO dword
        dwData^=dwKey;        //xor operation
//        pbBuffer = (char *) &dwData;
        // 将加密过的数据写入目标文件
//        fwrite(pbBuffer, 1, dwCount, hDestination);
		fwrite((char *) &dwData, 1, dwCount, hDestination);
    } while(!feof(hSource));

    //关闭文件、释放内存
    fclose(hSource);
    fclose(hDestination);
    if(pbBuffer)
    {
//   		pbBuffer = NULL; 		// 20171120 find bug
        free(pbBuffer);		
		pbBuffer = NULL;
    }
    printf("%s is encrypted to %s\n",szSource,szDestination);
    return 1;
}


/*
    count space of directory  , util Byte
 */
unsigned long my_du(char *path)
{

    unsigned  total=0;
    int filesize=0;
    DIR *db;
    char filename[512];
    struct dirent *ptr;

    if(check_directory(path)==ERROR)
    {
        filesize=get_file_size(filename);
        total+=filesize>=0?filesize:0;
        return total;
    }

    db=opendir(path);
    if(db==NULL)
    {
        return 0;
    }

    while ((ptr=readdir(db)))
    {
        if((strcmp(ptr->d_name,".")==0)||(strcmp(ptr->d_name,"..")==0))
        {
            continue;
        }
        memset(filename,0,sizeof(filename));
        if(strlen(path)+strlen(ptr->d_name)>sizeof(filename))
            return 0;
        sprintf(filename,"%s/%s",path,ptr->d_name); //get second level directory
        if(ptr->d_type == DT_DIR)
        {
            total= my_du(filename);
        }
        else
        {
            filesize=get_file_size(filename);
            total+=filesize>=0?filesize:0;
        }
    }
    closedir(db);
    return total;
}


/*
 count space of directory  , util: 'b' or 'B' is Byte;'k' or 'K'  is KByte; 'm' or 'M' is M byte
 'g' or 'G' is G byte; other is default Kbyte;
 */
unsigned long count_directory_size(char *path,char * util)
{
    unsigned  total;
    total=my_du(path);

    switch(util[0])
    {
    case 'b':
    case 'B':
        break;
    case 'k':
    case 'K':
        total=total/1024;
        break;
    case 'm':
    case 'M':
        total=total/1024/1024;
        break;
    case 'g':
    case 'G':
        total=total/1024/1024/1024;
        break;
    default:
        total=total/1024;
        break;
    }
    // printf("%s %u\n",__func__,total);
    return total;
}

