/**
 * @file inifile.h
 * @author 刘训
 * @date 2010-10-9
 * @version 0.2
 * @brief initialization file read and write API
 *	-size of the ini file must less than 16K
 *	-after '=' in key value pair, can not support empty char. this would not like WIN32 API
 *	-support comment using ';' prefix
 *	-can not support multi line
 */

#ifndef INI_FILE_H_
#define INI_FILE_H_

#ifdef __cplusplus
extern "C"
{

#endif
int load_ini_file(const char *file, char *buf,int *file_size);
int read_profile_string( const char *section, const char *key,char *value, int size,const char *default_value, const char *file);
int read_profile_int( const char *section, const char *key,int default_value, const char *file);
int write_profile_string( const char *section, const char *key,const char *value, const char *file);
int readini(char *filename,char *group,char *member,char *values);
#ifdef __cplusplus
}; //end of extern "C" {
#endif

#endif //end of INI_FILE_H_

