#include "../_precomp.h"
#include "public.h"

char kstr[500][45];

#if defined _2410
char tmpfilepath[128]="/etc/update/~~.tmp";//path of temporary file
#elif defined _2416
char tmpfilepath[128]="/tmp/~~.tmp";
#endif

int user_data_len=992;

/*--------------------------------------------------------------------------*
@Function		: mv_file - move file
@Include      	: "public.h"
@Description		: from：source file；
				to : destination file move the source file to destination file,if the destination is not exit,create it.After moving, delete source files
@Return Value	: Success 0 .Failure -1 
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int mv_file(char *from ,char *to)
{
	unsigned int CMAXLEN = 1024, nbytes;
	char buf[CMAXLEN];
	FILE *ffp, *tfp;

	if((ffp = fopen(from, "rb")) == NULL)		return -1;
	if((tfp = fopen(to, "wb")) == NULL) {
		fclose(ffp);
		return -1;
	}

	while(!feof(ffp) && !ferror(ffp)) {
		nbytes = fread(buf, 1, CMAXLEN, ffp);
		if(fwrite(buf, 1U, nbytes, tfp) != nbytes) 
		{
			fclose(ffp);
			fclose(tfp);
			return -1;
		}
		if(nbytes < CMAXLEN)	break;
	}
	fclose(ffp);
	fclose(tfp);
	remove(from);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function		: cp_file - copy file
@Include      	: "public.h"
@Description		: from：source file；
			: to : destination file copy the source file to destination file,if the destination is not exit,create it.After copying, delete source files
@Return Value	: Success 0 .Failure -1 
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int cp_file(char *from ,char *to)
{
	unsigned int CMAXLEN = 1024, nbytes;
	FILE *ffp, *tfp;
	char buf[CMAXLEN];

	if((ffp = fopen(from, "rb")) == NULL)
	{
		return -1;
	}
	creatdir(to);
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

/*--------------------------------------------------------------------------*
@Function		: cp_file - copy file
@Include      	: "public.h"
@Description		: from：source file；
			: to : destination file copy the source file to destination file,if the destination is not exit,create it.After copying, delete source files
@Return Value	: Success 0 .Failure -1 
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int appen_file(char *from ,char *to)
{
	unsigned int CMAXLEN = 1024, nbytes;
	FILE *ffp, *tfp;
	char buf[CMAXLEN];

	if((ffp = fopen(from, "rb")) == NULL)
	{
		return -1;
	}
	creatdir(to);
	if((tfp = fopen(to, "a+")) == NULL) 
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

/*--------------------------------------------------------------------------*
@Function		: testdir - test whether a file is a directory
@Include      	: "public.h"
@Description		: path：file path
@Return Value		: Success 0 .Failure -1 
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int testdir(char *path)
{
	struct stat buf;

	if(lstat(path,&buf)<0)
	{
		return 0;
	}
	if(S_ISDIR(buf.st_mode)||S_ISLNK(buf.st_mode))
	{
		return 1;
	}
   return 0;
}

/*--------------------------------------------------------------------------*
@Function		: printftime - Print time interval
@Include      	: "public.h"
@Description		: fun：Tips statement ；
				print the time that the function execution time
@Return Value	: void
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
void printftime(char *fun)
{
	static struct timeval oldtime;
	struct timeval newtime;

	gettimeofday(&newtime,NULL);
	if(fun)
	{
		printf(" %s--- %ld\n",fun,(newtime.tv_sec-oldtime.tv_sec)*1000000+
							newtime.tv_usec-oldtime.tv_usec);
		memcpy(&oldtime,&newtime,sizeof(oldtime));
	}
	else
		memcpy(&oldtime,&newtime,sizeof(oldtime));
}

/*--------------------------------------------------------------------------*
@Function		: safe_cp - copy file
@Include      	: "public.h"
@Description		: oldpath：source file or directory
				newpath : destination file or directory when using cp, rm batch processing files, sudden power off may cause u disk, SD card permanent damage.safe_cp ensure to flush buffer after a file was handled
@Return Value	: Success 0 ,Failure-1;
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int safe_cp(char * oldpath,char *newpath)
{
	char path2[512],path3[512];
	DIR *db;
	struct dirent *p;
	static int count=0;

	if(testdir(oldpath)!=1)		return cp_file(oldpath,newpath);
	memset(path2,0,sizeof(path2));
	memset(path3,0,sizeof(path3));
	db=opendir(oldpath);
	if(db==NULL)		return -1;

	if(access(newpath,F_OK)!=0)
		if(creatdir(newpath)!=0)
		{
			closedir(db);
			return -1;
		}
	while ((p=readdir(db))) 
	{
		if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
			continue;
		else
      		{
			memset(path3,0,sizeof(path3));
			memset(path2,0,sizeof(path2));
			sprintf(path2,"%s/%s",oldpath,p->d_name);
	             sprintf(path3,"%s/%s",newpath,p->d_name);
       		}
	      count++;
      		if(safe_cp(path2,path3)!=0)
	        {
		        closedir(db);
		         //printf("safe_cp 1\n");
		        return -1;
	        }
		if(count>100)
		{	
			count=0;
	        	sync();
		}
	  }
	  closedir(db);
	  //printf("safe_cp 2\n");
	  return 0;
}

/*--------------------------------------------------------------------------*
@Function		: safe_rm - delete file
@Include      	: "ublic.h"
@Description		: oldpath：file or directory that you will delete
@Return Value	: Success 0 ，Failure-1；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int safe_rm(char * oldpath)
{
	char path2[512],path1[512],buf[512];
	DIR *db;
	struct dirent *p;
	int max=10,len=0,rmdirflag=0;
	static int rmtotal=-1,rmcurentcount=0;

	if((strlen(oldpath)>=512)||(oldpath==NULL))		return -1;  //Prevent stack overflow
	memset(path2,0,sizeof(path2));
	strcpy(path2,oldpath);
	len=strlen(path2);
	if((len-->2)&&(path2[len--]=='*')&&(path2[len]=='/'))//if parameter is similar to 'note/frame/*',do not delete frame
	{
		  len++;
		  len++;
		  rmdirflag=1;
		  path2[len]=0;
		  memset(path1,0,sizeof(path1));
		  strncpy(path1,oldpath,len-2);
		  if(rmtotal==-1)		rmtotal=dir_countfile_all(path1);  //total of file in this directory
	 }
	else 
	{ 
		  len++;
		  len++;
		  if(testdir(oldpath)!=1)
		   { 
		      rmcurentcount++;
		      sprintf(buf,"%s %d/%d",kstr[159],rmcurentcount,rmtotal);  //display deleting progress
			//	printf("%s",buf);
		      if(rmcurentcount%100==0)		sync();
		     return remove(oldpath);
		    }
		  else
		  {
		      strcpy(path1,oldpath);
		      if(rmtotal==-1)
		        {
		            rmcurentcount=0 ; 
		            rmtotal=dir_count_file(path1);//total of file in this directory
 		        }
		  }
	 }
	//  printf("%s,%d\n",path1,dir_count_file(path1));
	while(dir_count_file(path1)&&max--)
	{
		memset(path2,0,sizeof(path2));
		db=opendir(path1);
		if(db==NULL)
		{
			rmtotal=-1;
			rmcurentcount=0;
			return -1;
		}
		while((p=readdir(db)))
		{
		      if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
		         continue;
		      else
		       {   
		          memset(path2,0,sizeof(path2));
		          sprintf(path2,"%s/%s",path1,p->d_name);
		       }
		     if(safe_rm(path2)!=0)
		      {
		         //closedir(db);
		         printf("fial\n");
		      }
	        //sync();
    		}
	  //printf("dir----tail\n");
	  closedir(db);
 	}
   	if(rmdirflag==0)
      	   return rmdir(path1);
	else
	{      
         rmtotal=-1;
         rmcurentcount=0;
          return 0;  
        }
}

/*--------------------------------------------------------------------------*
@Function		: safe_rm_frame - delete image file
@Include      	: "public.h"
@Description		: oldpath：path of image file
@Return Value	: Success 0 ，Failure-1；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int safe_rm_frame(char * oldpath) 
{
	char path2[128],path1[128];
	DIR *db;
	struct dirent *p;
	int max=10,len=0,rmdirflag=0;

	if(strlen(oldpath)>=128||oldpath==NULL)	return -1;          //Prevent stack overflow
	memset(path2,0,128);
	strcpy(path2,oldpath);
	len=strlen(path2);
	if((len-->2)&&(path2[len--]=='*')&&(path2[len]=='/'))//if parameter is similar to 'note/frame/*',do not delete frame
	{
		  len++;
		  len++;
		  rmdirflag=1;
		  path2[len]=0;
		  memset(path1,0,128);
		  strncpy(path1,oldpath,len-2);
 	}
	else 
	 { 
		  len++;
		  len++;
		  if(testdir(oldpath)!=1)		return remove(oldpath);
		  else 	strcpy(path1,oldpath);
	  }
	//  printf("%s,%d\n",path1,dir_count_file(path1)); 
	while(dir_count_file(path1)&&max--)
	{
		  memset(path2,0,128);
		  db=opendir(path1);
		  if(db==NULL)	return -1;
		  while((p=readdir(db)))
		   {
			      if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
				         continue;
			      else
			       {   
				          memset(path2,0,128);
				          sprintf(path2,"%s/%s",path1,p->d_name);
			       }
			     if(safe_rm(path2)!=0)
			       {
				         //closedir(db);
					         printf("fial\n");
        			}
		    }
	   	  closedir(db);
	 }
	if(rmdirflag==0)
      		   return rmdir(path1);
   	else return 0;  

}

/*--------------------------------------------------------------------------*
@Function		: creatdirfile - get file list
@Include      	: "public.h"
@Description		: path：path of file
				get all the file lists in path
@Return Value	: Success file list stream，Failure NULL
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
FILE* creatdirfile(char *path)
{
	DIR *db;
	struct dirent *p;
	char namebuf[128];
	FILE *tmp;

	tmp=fopen(tmpfilepath,"w");
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
		if(testdir(namebuf))
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
	tmp=fopen(tmpfilepath,"r");
	return tmp;
}


/*--------------------------------------------------------------------------*
@Function		: safe_cp_photo - copy image
@Include      	: "public.h"
@Description		: 
				oldpath：source directory
				newpath : destination directory
@Return Value	: Success 0 ，Failure-1；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int safe_cp_photo(char * oldpath,char *newpath)
{
	char path2[128],path3[128];
	DIR *db;
	struct dirent *p;
	int xh;

	if(testdir(oldpath)!=1)		return cp_file(oldpath,newpath);
	memset(path2,0,128);
	memset(path3,0,128);
	db=opendir(oldpath);
	if(db==NULL)		return -1;
	if(access(newpath,F_OK)!=0)
		if(mkdir(newpath,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH)!=0)	return -1;
	while ((p=readdir(db)))
	{
		if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
			continue;
		else
		{
			memset(path3,0,128);
            		memset(path2,0,128);
			sscanf(p->d_name,"%d.pht",&xh);
			sprintf(path2,"%s/%s",oldpath,p->d_name);
			sprintf(path3,"%s/%d/%s",newpath,xh/1000,p->d_name);
		}
		if(safe_cp(path2,path3)!=0)
		{
			closedir(db);
			return -1;
		}
	}
	closedir(db);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function		: getfile - get file name
@Include      	: "public.h"
@Description		: oldpath：path of file
				get file name in the path
				for example: you will get 1_2.pis form note/finger/1_2.pis 
@Return Value	: Success file name ，Failure  NULL；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
char * getfile(char *path)
{
	static char filename[1024];
	char *tmp1,*tmp2,filepath[1024];

	if(path==NULL)	return NULL;
	memset(filepath,0,sizeof(filepath));
	strcpy(filepath,path);
	tmp2=tmp1=strtok(filepath,"/");
	while((tmp1=strtok(NULL,"/")))
	{
		tmp2=tmp1;
	}
	memset(filename,0,sizeof(filename));
	if(tmp2)	strcpy(filename,tmp2);
	else return NULL;
	return filename;
}

/*--------------------------------------------------------------------------*
@Function		:creatdir - create directory
@Include      	: "public.h"
@Description		: path：
			according to path to create recursive directory 
@Return Value	: Success 0 ，Failure-1；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int creatdir(char *path)
{
	char buff[user_data_len],buff1[user_data_len],*p=NULL,*p1=NULL;
	int len=0,i=0;

	p1=path;
	p=buff;
	memset(buff,0,user_data_len);
	cut(p1);

	while(*p1)
	{
		if(*p1=='/')
		{
			if(len==1)	p1++;
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
		if(i>=user_data_len)	break;
		 // printf("%c,%d,%c,%d\n",*p,*p,*p1,*p1);
	}

	p=buff;
	p1=buff1;
	memset(buff1,0,user_data_len);
	
	if(*p=='/')		{*p1='/';p++;}//if the first character is '/',it is root.the pwd should also be obtained

	while((len=strcspn(p,"/")))
	{
		if(len==(int)strlen(p))	break;
		strncat(p1,p,len+1);
		if(access(p1,F_OK)!=0)                      //if the directory is not exit
		if(mkdir(p1,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH)!=0)
		{
			return -1;  //create directory
		}
		p+=len;
		p++;
	}
	return 0;
}

/*--------------------------------------------------------------------------*
@Function		: cut - remove '\r\n' of a string
@Include      	: "public.h"
@Description		: tmp：string data
@Return Value	: void
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
void cut(char *tmp)
{
	if(tmp==NULL)	return;
	while(*tmp)
	{
		if(*tmp=='\n'||*tmp=='\r')
		  {
			*tmp=0;
			return ;
		  }
		 else tmp++;
	}
}

/*--------------------------------------------------------------------------*
@Function		: dir_count_file - count up the number of file in directory
@Include      	: "public.h"
@Description		: path：directory file
			path is the bottom directory
@Return Value	: Success	number of file，Failure 0；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int dir_count_file(char *path)
{
	int i=0;
	DIR *db;
	struct dirent *p;

	db=opendir(path);
	if(db==NULL)		return 0;
	while ((p=readdir(db)))
	{  
		if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
			continue;
		else
		{  
			i++;
		}
	}
	closedir(db);
	return i;
}


/*--------------------------------------------------------------------------*
@Function		: dir_countfile_all - count up the number of file in directory
@Include      	: "public.h"
@Description		: path：directory file
@Return Value	: Success number of file，Failure 0；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int dir_countfile_all(char *photopath)
{
	int count=0;
	DIR *db;
	char filename[64];
	struct dirent *p;

	db=opendir(photopath);
	if(db==NULL)		return 0;
	memset(filename,0,64);
	while ((p=readdir(db)))
	{  
		if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
			continue;
		else
		{  
			sprintf(filename,"%s/%s",photopath,p->d_name); //get second level directory
			if(testdir(filename))
				count+=dir_count_file(filename); //count up the number of file in directory
			else count++;
		}
		memset(filename,0,64);
	}
	closedir(db);
	return count;
}

/*--------------------------------------------------------------------------*
@Function		:countfile_suffix - count up file with specific types in directory
@Include      	: "public.h"
@Description		: path：directory file
			suffix :	file type
@Return Value	: Success	: number of file，Failure 0
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int countfile_suffix(char *path,char *suffix)
{
	DIR *db;
	char *ptr;
	struct dirent *p;
	int count=0;

	db=opendir(path);
	if(!db)	return -1;
	while ((p=readdir(db)))
	{
		if(strcmp(p->d_name,".")==0||strcmp(p->d_name,"..")==0)
			continue;
		else
		{
			if(suffix)
			{
				ptr=p->d_name;
				ptr+=(strlen(p->d_name)-strlen(suffix));
				if(strcmp(ptr,suffix)==0)		count++;
			}
			else 	count++;
      		}
 	}
	closedir(db);
	return count++;
}

/*--------------------------------------------------------------------------*
@Function		: digittest - check whether all characters are Arabic numerals
@Include      	: "public.h"
@Description		: ptr：pointer to string
@Return Value	: Success 1，Failure 0
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int digittest(const char *ptr)
{
	const char *str;
	str=ptr;
	while(str)
	{
		if(!isdigit(*str++))	return 0;
	}
	return 1;
}

/*--------------------------------------------------------------------------*
@Function		: UsbContion - check whether U-disk has connected
@Include      	: "public.h"
@Description		: check whether U-disk has connected
@Return Value	: Success TRUE，Failure FALSE；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int UsbContion()
{
	DIR *db;
	FILE *fp;
	struct dirent *p;
	char buf[256],filename[126];
	char *dirname="/proc/scsi/usb-storage-0/";

	db=opendir(dirname);
	if(!db)	return -1;
	while ((p=readdir(db)))
	{
	    if(strcmp(p->d_name,".")==0||strcmp(p->d_name,"..")==0)
      		 continue;
	    else
     	     {
			memset(filename,0,sizeof(filename));
			sprintf(filename,"%s%s",dirname,p->d_name);
			if(access(filename,F_OK)==-1)		return FALSE;
			if((fp=fopen(filename,"r"))==NULL)		return FALSE;
   	
			memset(buf,0,sizeof(buf));	
			while (fgets(buf,sizeof(buf),fp))
			{
				cut(buf);
				if(strstr(buf,"Attached: Yes"))
				{
					fclose(fp);
					closedir(db);
					return TRUE;
				}
				memset(buf,0,sizeof(buf));
			}
			fclose(fp);
      		}
	}
	closedir(db);
	 return FALSE;
}


/*--------------------------------------------------------------------------*
@Function		:SortSearch- search a record in a file(not sorted)
@Include      	: "public.h"
@Description		:	
				  char *filename :	the sorted file in which you will search a record.
				  int rlength: length of record
				  int kpos: postion of key word (the number of bytes before the key word in one record)
			    int klength: length of key word
				 char *key: the key word you want to search
@Return Value	: Success TRUE，Failure FALSE；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int SortSearch(char *filename,int rlength,int kpos,int klength,char *key)
{
	int ret=0;
	FILE *fp=NULL;
	char record[256]={0};
	char temp[256]={0};

	if((fp=fopen(filename,"r"))==NULL)
	{
		perror("fopen");
		return -1;
	}
	
	fread(record,rlength+1,1,fp);
	
	while(!feof(fp))
	{
		printf("record=%s",record);
		memset(temp,0,sizeof(temp));
		strncpy(temp,record+kpos,klength);
		ret=strcmp(temp,key);
		printf("ret = %d\n",ret);
		
		if(ret==0)
		{	
			printf("OK! the record you want is : %s\n",record);
			return TRUE;
		}
		fread(record,rlength+1,1,fp);
	
	}	
	
	return FALSE;
}

/*--------------------------------------------------------------------------*
@Function		: BinarySearch- Binary search in a file(sorted)
@Include      	: "public.h"
@Description		:	
				  char *filename :	the sorted file in which you will search a record.
				  int rlength: length of record
				    int kpos: postion of key word (the number of bytes before the key word in one record)
			    int klength: length of key word
				    char *key: the key word you want to search
@Return Value	: Success the position of the record，Failure FALSE；
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int BinarySearch(char *filename,int rlength,int kpos,int klength,char *key)
{
	struct stat buf;
	int total=0;
	int midpos=0;
	int high=0;
	int low=0;
	int ret=0;
	FILE *fp=NULL;
	char record[256]={0};
	char temp[256]={0};

	if((fp=fopen(filename,"r"))==NULL)
	{
		perror("fopen");
		return -1;
	}
	
	fstat(fileno(fp),&buf);
	total=buf.st_size;
	high=total/rlength;
	
	while(low<=high)
	{
		midpos=(low+high)/2;
		rewind(fp);
		fseek(fp,midpos*rlength,SEEK_CUR);
		fread(record,rlength,1,fp);
		printf("record=%s",record);
		memset(temp,0,sizeof(temp));
		strncpy(temp,record+kpos,klength);
		ret=strcmp(temp,key);
		printf("ret = %d\n",ret);
		
		if(ret==0)
		{	
			printf("OK! the record you want is : %s\n",record);
			return midpos;
		}
		else if(ret<0)
		{
			low=midpos+1;
		}
		else
		{
			high=midpos-1;
		}
	
	}	
	
	return FALSE;
}


/*--------------------------------------------------------------------------*
@Function		: readini - read ini
@Include      	: "com.h"
@Description		: file : file stream
			grop  : a group of data
			member : data field
			values : data content
			read values from file stream
@Return Value	: void
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
void readini(FILE *file,char *group,char *member,char *values)
{
	char buf[256],tmpbuf1[256],tmpbuf2[256],*p1;
	int len=0,flag=0;

	memset(buf,0,128);
	rewind(file);                   //pointer to file header
	while(fgets(buf,128,file))
	{
		p1=buf;
		cut(buf);
		if(strcmp(buf,group)==0)
		{
			flag=1;
			continue;
		}// find the group position and set it
		if(flag)
		{
			if((strlen(buf)==0)||(*p1=='['))
			{
				values=NULL;
				break;
             	}//quit when meet blank line or the next group
			len=strcspn(buf,"=");     //cut up members of group and value with '='
			memset(tmpbuf1,0,sizeof(tmpbuf1));
			memset(tmpbuf2,0,sizeof(tmpbuf2));
			strncpy(tmpbuf1,buf,len);
			p1+=len;
			p1++;
			if(strcmp(tmpbuf1,member)==0)
			{
				strcpy(values,p1);
				break;
			}//read values of members
		}
      		memset(buf,0,sizeof(buf));
	}
	cut(values);   
}

/*--------------------------------------------------------------------------*
@Function		: read_terminalno - read machine ID
@Include      	: "com.h"
@Description		: the last six bits of ip are used as ID
@Return Value	: Success	return ID; Failure 0;
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int readbh(char *param) 
{
	char buf[32],*p1,*p2;//,bianhao[126];
	FILE * menurc;
	int bh1=0,bh2=0;
	char menurcpath[128]="/etc/qte/.qt/menurc";

	menurc=fopen(menurcpath,"r");
	if(menurc!=NULL)
	{
		readini(menurc,"[menu-02]","ip",buf);
		fclose(menurc);
	}
	else  return FALSE;

	strtok(buf,".");
	strtok(NULL,".");
	p1=strtok(NULL,".");
	p2=strtok(NULL,".");
	if(p1)
		bh1=atoi(p1);
	if(p2)
		bh2=atoi(p2);
	sprintf(param,"%03d%03d",bh1,bh2);
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: read_terminalno - read machine ID
@Include      	: "com.h"
@Description		: the last six bits of ip are used as ID
@Return Value	: Success	return ID; Failure 0;
@Create time		: 2009-06-15		
*---------------------------------------------------------------------------*/
int read_terminalno() 
{
	char buf[32],*p1,*p2,bianhao[126];
	FILE * menurc;
	int bh1=0,bh2=0;
	char menurcpath[128]="/etc/qte/.qt/menurc";

	menurc=fopen(menurcpath,"r");
	if(menurc!=NULL)
	{
		readini(menurc,"[menu-02]","ip",buf);
		fclose(menurc);
	}
	else  return 0;

	strtok(buf,".");
	strtok(NULL,".");
	p1=strtok(NULL,".");
	p2=strtok(NULL,".");
	if(p1)
		bh1=atoi(p1);
	if(p2)
		bh2=atoi(p2);
	sprintf(bianhao,"%03d%03d",bh1,bh2);
	return atoi(bianhao)%1000;
}
/*--------------------------------------------------------------------------*
@Function			: read_mac_config - get Mac addr
@Include      		: "net_tcp.h"
@Description			: None
@Return Value		: mac
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
char * read_mac_config()
{
	FILE *file=NULL;
	static char mac[18];
	char buff[64],*ptr=NULL;

	file=fopen("./mac.sh","r");
	if(!file)	return NULL;

	fgets(buff,sizeof(buff),file);
	memset(buff,0,sizeof(buff));
	fgets(buff,sizeof(buff),file);
	ptr=strstr(buff,"ether");
	if(ptr)
		strncpy(mac,ptr+6,sizeof(mac)-1);
	fclose(file);
	cut(mac);
	return mac;
}


void read_menu2()
{
	char buf1[50],buf2[45],buf3[45],*p,*p2;
	FILE *menu2;
	int len=0,j;

	menu2=fopen("/etc/qte/.qt/menu2.ini","r");
	if(menu2==NULL)	return;
      memset(buf1,0,50);
     
	while(fgets(buf1,50,menu2))
	{
		cut(buf1);
		if(strncmp(buf1,"kstr",4)!=0)
			continue;
		len=strcspn(buf1,"=");
		strncpy(buf2,buf1,len);
		p=buf1;
		p+=len;
		p++;
		strcpy(buf3,p);
		p2=buf2;
		p2+=4;
		j=atoi(p2);
		if(j<0||j>500)	continue;
		strcpy(kstr[j],buf3);
	}
	fclose(menu2);
}

//获取当前时间,由参数buff返回,格式为:YYYY-MM-DD HH:MM:SS,因此要求参数buff空间长度应大于19
void getcurtime(char *buff)             
{
	time_t the_time;
	struct tm * tm_time;

	time(&the_time);
	tm_time=localtime(&the_time);
	sprintf(buff,"%04d-%02d-%02d %02d:%02d:%02d",tm_time->tm_year+1900,tm_time->tm_mon+1,tm_time->tm_mday,tm_time->tm_hour,tm_time->tm_min,tm_time->tm_sec);
}


//获取路径中的文件名.如传入参数 note/finger/1_2.pis 返回1_2.pis
char * getendname(const char *path)
{
 static char filename[1024];
 char *tmp1,*tmp2,filepath[1024];

 if(path==NULL)return NULL;
 memset(filepath,0,sizeof(filepath));
 strcpy(filepath,path);
 tmp2=tmp1=strtok(filepath,"/");
 while((tmp1=strtok(NULL,"/")))
 {
  //printf("%s \n",tmp1);
  tmp2=tmp1;
 }
memset(filename,0,sizeof(filename));
if(tmp2)strcpy(filename,tmp2);
else return NULL;
return filename;
}


////获取路径中的目录名.如传入参数 note/finger/1_2.pis 返回note/finger
char * getbeginname(const char *path)
{
 static char filepath[1024];
 char *tmp1;
 if(path==NULL)return NULL;
 memset(filepath,0,sizeof(filepath));
 strcpy(filepath,path);
 tmp1=strrchr((const char *)filepath,'/');
  if(tmp1)*tmp1='\0';
  if(strlen(filepath)==strlen(path))return "./";
return filepath;
}

//检查输入的ip地址是否正确
int check_ip(char *ptr)
{
	int data1=0,data2=0,data3=0,data4=0;
	if(!ptr)	return -1;
	if(sscanf(ptr,"%d.%d.%d.%d",&data1,&data2,&data3,&data4)!=4)return -1;
	if(data1>=0&&data1<=255&&data2>=0&&data2<=255&&data3>=0&&data3<=255&&data4>=0&&data4<=255)return 0;
	return -1;
}

//夏令时设置
int setdaylingth(time_t curdate)
{
	struct timeval tv;                                //时间结构
	struct timezone tz;                             
	char cmdbuf[128];
	struct tm *ptdate;
	int i=0,j=0;
//	char *format="%Y-%m-%d %H:%M:%S";

	ptdate = localtime (&curdate);                    //获取格式时间
	gettimeofday (&tv , &tz);                                //获取本机的时区信息
	tv.tv_sec=mktime(ptdate);                                 //设置系统时间
	sprintf(cmdbuf, "setdate -w %04d.%02d.%02d-%02d:%02d:%02d", ptdate->tm_year+1900,ptdate->tm_mon+1,ptdate->tm_mday,ptdate->tm_hour, ptdate->tm_min, ptdate->tm_sec);
	i=settimeofday(&tv,&tz);
	j=system(cmdbuf);
	if(i<0||j<0)
	{
		return -1;
	}
	return 0;
}


//文件字段分割
int field_handle(char *buf,char str[12][128])
{
	int i=0,j=0;

	if(buf==NULL)	return -1;
	while(1)//分割由","区分的各个字段
        {
		i=strcspn(buf,",");
		if(strlen(buf)==0)	break;
		memset(str[j],0,sizeof(str[j]));
		if(i!=0)
		{ 
			strncpy(str[j],buf,i);
		}
		buf+=i;//指针指向下一个字段
		buf++;//略过分割符
		j++;
	}
	return 0;
}

//修改mac地址
void write_mac_config(char *mac)
{
FILE *file=NULL;
file=fopen("./mac.sh","w");
if(!file)return ;
fprintf(file,"ifconfig eth0 down\nifconfig eth0 hw ether %s\nifconfig eth0 up\n",mac);
fclose(file);
}


// 可打印字符串转换为字节数据
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// 输入: pSrc - 源字符串指针
//nSrcLength - 源字符串长度
// 输出: pDst - 目标数据指针
// 返回: 目标数据长度
int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int i=0;
	for ( i = 0; i < nSrcLength; i += 2)
	{
		// 输出高4位
		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}
		pSrc++;
		// 输出低4位
		if ((*pSrc>='0') && (*pSrc<='9'))
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}
		pSrc++;
		pDst++;
	}

	return (nSrcLength / 2);
}



// 字节数据转换为可打印字符串
// 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
// 输入: pSrc - 源数据指针
//nSrcLength - 源数据长度
// 输出: pDst - 目标字符串指针
// 返回: 目标字符串长度
int gsmBytes2String(unsigned char* pSrc, char* pDst, int nSrcLength)
{
	const char tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表
	int i=0;

	for (i = 0; i < nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];	// 输出高4位
		*pDst++ = tab[*pSrc & 0x0f];	// 输出低4位
		pSrc++;
	}
	*pDst = '\0';   

	return (nSrcLength * 2);
}


/*--------------------------------------------------------------------------*
@Function		: rightsub - 从右边截取字符串
@Include      	: "public.h"
@Description		: 
			输入源:src--源数据指针
				postion--起始位置
				len--截取长度
				mask--长度不够时填充掩码
			输入:dst--目标字符串指针
			从src字符串的postion位置截取len长度字符串，源字符串不够用msak掩码填充。
@Return Value	: Success 0 .Failure -1 
*---------------------------------------------------------------------------*/
int rightsub(char *dst,const char *src,int postion,int len,char mask)
{
	char tmpsno[1024];
	int i=0,j=0;

	if(src==NULL||postion<0||len<0)		return -1;

	i=strlen(src);
	j=postion+len;
	memset(tmpsno,0,sizeof(tmpsno));
	if(j>i){
		memset(tmpsno,mask,j-i);
		strncat(&tmpsno[j-i],src,i);
	}
	else
		strcpy(tmpsno,src);

	i=strlen(tmpsno);
	strncpy(dst,&tmpsno[i-j],len);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function		: rightsub - 从左边截取字符串
@Include      	: "public.h"
@Description		: 
			输入源:src--源数据指针
				postion--起始位置
				len--截取长度
				mask--长度不够时填充掩码
			输入:dst--目标字符串指针
			从src字符串的postion位置截取len长度字符串，源字符串不够用msak掩码填充。
@Return Value	: Success 0 .Failure -1 
*---------------------------------------------------------------------------*/
int leftsub(char *dst,const char *src,int postion,int len,char mask)
{
	char tmpsno[1024];
	int i=0,j=0;

	if(src==NULL||postion<0||len<0)		return -1;
	i=strlen(src);
	j=postion+len;
	memset(tmpsno,0,sizeof(tmpsno));
	strncat(tmpsno,src,i);
	if(j>i){
		j=j-i;
		while(j--) tmpsno[i++] = mask;
	}

	strncpy(dst,&tmpsno[postion],len);
	return 0;
}


int  GetCount(char *format,char starter)
{
      char *P;
	int count=0;

      P = format;

      while( *format == starter)
	{
	  format++;
	}

      count = format - P + 1;
	return count;
}

/*--------------------------------------------------------------------------*
@Function		: FormatDateTime - 格式化日期时间
@Include      	: "public.h"
@Description		: 
			输入源:format--日期时间格式
			输出:result--日期时间格式化字符串
			将日期/时间按照format参数格式化成相应字符串由result返回
@Return Value	: Success TRUE .Failure FALSE
*---------------------------------------------------------------------------*/
int FormatDateTime(const char * format,char *result)
{
	time_t the_time;
	struct tm * tm_time;
	char LastToken,Starter,Token;
	int count=0;//len=0,
	int Digits=0;
	int year=2009, month=11, day=19, hour=11, min=21, sec=10;//, msec;
	
	if(format==NULL)	return FALSE;
	time(&the_time);
	tm_time=localtime(&the_time);

	year=tm_time->tm_year+1900;
	month=tm_time->tm_mon+1;
	day=tm_time->tm_mday;
	hour=tm_time->tm_hour;
	min=tm_time->tm_min;
	sec=tm_time->tm_sec;

      LastToken = ' ';
      while (isascii(*format)&&strlen(format))
	{

		Starter = *format;
		format = format+1;
		Token = Starter;
		if (Token>'a'&& Token<'z') Token=Token - 32;
		if (Token>'A'&& Token<'Z')
		{
			if (Token == 'M' && LastToken == 'H') Token = 'N';
			LastToken = Token;
		}
		count=GetCount((char *)format,Starter);
		format+=(count-1);
		result+=Digits;
		 switch(Token){
			case 'Y':
				if(count<=2)
				{
					sprintf(result,"%02d",year%100);
				}
				else
				{
					sprintf(result,"%04d",year);
				}
			break;	
			case 'M':
				switch(count){
					case 1:
					case 2: 
						sprintf(result,"%02d",month);
					break;
					case 3: 
						//AppendString(ShortMonthNames[Month]);
					break;
					default:
						//AppendString(LongMonthNames[Month]);
					break;
				}
			break;
			case 'D':
				switch(count){
					case 1:
					case 2: 
						sprintf(result,"%02d",day);
					break;
					case 3: 
						//AppendString(ShortMonthNames[Month]);
					break;
					default:
						//AppendString(LongMonthNames[Month]);
					break;
				}
			break;
			case 'H':
				switch(count){
					case 1:
					case 2: 
						sprintf(result,"%02d",hour);
					break;
					case 3: 
						//AppendString(ShortMonthNames[Month]);
					break;
					default:
						//AppendString(LongMonthNames[Month]);
					break;
				}
			break;
			case 'N':	
				sprintf(result,"%02d",min);
			break;
			case 'S':	
				sprintf(result,"%02d",sec);
			break;
			case '-':	
			case '/':	
			case ':':	
			case ' ':	
				while(count--)
				{
					sprintf(result,"%c",Token);
					Digits=strlen(result);
					result+=Digits;
				}
			break;
			default:
				while(count--)
				{
					sprintf(result,"%c",Token);
					Digits=strlen(result);
					result+=Digits;
				}
			break;
		}
		Digits=strlen(result);
	}//while

	return TRUE;
}
