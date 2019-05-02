#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#define _XOPEN_SOURCE 700

char cipher[] = "qE1~ YMUR2\"`hNIdPzi\%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV\']jcp5JZ&Xl|\\8s;g<{3.u*W-0";
int key;
static const char *dirpath = "/home/ivan/shift4";

int lastCharPos(char *str, char chr){
	char *posChar = strrchr(str, chr);
	if(!posChar)
		return 0;
 	return (int) (posChar-str);
}

char encrypt(char *fname)
{
    char *ptr;
    for (int i = 0; i < strlen(fname); i++){
        ptr = strchr(cipher, fname[i]);
        if(ptr){
            int index = ptr - cipher;
            int chg = (index + key) % strlen(cipher);
            fname[i] = cipher[chg];
        }
    }
    return *fname;
}

char decrypt(char *fname)
{
    char *ptr;
    for (int i = 0; i < strlen(fname); i++){
        ptr = strchr(cipher, fname[i]);
        if(ptr){
            int index = ptr - cipher;
            int chg = index - key;
            while(chg < 0) chg += strlen(cipher);
            fname[i] = cipher[chg];
        }
    }
    return *fname;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
	char fpath[1000],tmp[1000];
    strcpy(tmp,path);
	// if(strstr(tmp,".izl"))
	// 	tmp[strlen(tmp)-4]='\0';
    encrypt(tmp);

    sprintf(fpath,"%s%s",dirpath,tmp);

	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    char temp[1000];
    strcpy(temp,path);
    encrypt(temp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,temp);

	char miris[15] = "filemiris.txt";
	char pathMiris[1000];
	encrypt(miris);

	sprintf(pathMiris, "%s/%s", dirpath,miris);
	FILE *rusak = fopen(pathMiris, "a");

	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st, info;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
        
		if(strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0)
			continue;

        decrypt(de->d_name);
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;

		char temp[1000];
		stat(temp, &info);
		struct passwd *pw = getpwuid(info.st_uid);
		struct group  *gr = getgrgid(info.st_gid);
		int readable = access(temp, R_OK);			//return 0 if it is readable
		char date[30];

		if (de->d_type == DT_REG && strcmp(temp, pathMiris)!=0  && readable!=0 && (strcmp(pw->pw_name, "chipset")==0 || strcmp(pw->pw_name, "ic_controller")==0) && strcmp(gr->gr_name, "rusak")==0) {
			strftime(date, 30, "%Y-%m-%d %H:%M:%S", localtime(&(info.st_atime)));
			fprintf(rusak, "%s\t\t%d\t\t%d\t\t%s\n", de->d_name, gr->gr_gid, pw->pw_uid, date);
			remove(temp);
			continue;
		}
	}

	fclose(rusak);
	closedir(dp);
	return 0;
}

static int xmp_unlink(const char *path)
{
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);
	
	int res;

	int isFile,status;
	if(isFile<0)
		return 0;

	char backup[] = "Backup", backpath[1000];
	char command[1000], timestamp[100], zipname[1000], backupfile[1000], ext[1000], rawname[1000], pathcur[1000], trash[]="RecycleBin";
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(timestamp, "%04d-%02d-%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	//^ membuat format timestamp

	encrypt(backup);
	strncpy(backpath, path, lastCharPos(path, '/'));
	backpath[lastCharPos(path, '/')] = '\0';
	sprintf(tmp, "%s/%s", backpath, backup);
	strcpy(backpath, tmp);

	int lastSlash = lastCharPos(path, '/');
	int lastDot = lastCharPos(path, '.');
	
	if (lastDot==0)
		lastDot = strlen(path);
	else{
		strcpy(ext, path+lastDot);
		if (strcmp(ext, ".swp")==0)		//mencegah file .swp untuk load
		{
			res = unlink(fpath);
			
			if (res == -1)
				return -errno;
			return 0;
		}
	}
	strcpy(backupfile, path+lastSlash+1);
	strncpy(rawname, path+lastSlash+1, lastDot-(lastSlash+1));
	rawname[lastDot-(lastSlash+1)] = '\0';

	strncpy(tmp, path, lastCharPos(path, '/'));
	tmp[lastCharPos(path, '/')] = '\0';
	sprintf(pathcur, "%s%s", dirpath, tmp);


	sprintf(zipname, "%s_deleted_%s.zip\0", rawname, timestamp);
	encrypt(zipname);
	encrypt(trash);
	encrypt(backupfile);
	encrypt(rawname);
	sprintf(command, "cd %s && mkdir -p '%s' && zip '%s/%s' '%s' '%s/%s'* && rm -f '%s/%s'*", pathcur, trash,trash, zipname, backupfile, backup, rawname, backup, rawname);

	if (fork()==0)
		execl("/bin/sh","/bin/sh", "-c", command, NULL);

	while((wait(&status))>0);

	res = unlink(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{	
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;

	res = rmdir(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);
	int res = 0;
    int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;
	int slash = 0;
	for(int i = 0; i<strlen(path);i++)
		if(path[i]=='/') slash++;
	if(strstr(path,"/YOUTUBER/") && slash <= 2)
		res = mkdir(fpath, 0750);
	else	
		res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
    char tmp[1000];
	if(strstr(path,"/YOUTUBER"))
		strcat(path,".iz1");
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);
	int fd;
	int res;

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	int slash = 0;
	for(int i = 0; i<strlen(path);i++)
		if(path[i]=='/') slash++;
	if(strstr(path,".iz1") && strstr(path,"/YOUTUBER/") && slash <= 2){
		pid_t child_id;
		child_id = fork();
		if (child_id < 0) {
			exit(EXIT_FAILURE);
		}

		if (child_id == 0) {
			char *argv[] = {"zenity", "--error", "--title", "Error", "--text", "File ekstensi iz1 tidak boleh diubah permissionnya.", NULL};
			execv("/usr/bin/zenity", argv);
  		}
		return 0;
	}

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;
	res = chmod(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	char ffrom[1000];
	char fto[1000];
    char tmp[1000];
    strcpy(tmp,from);
    encrypt(tmp);

	if(strcmp(from,"/") == 0)
	{
		from=dirpath;
		sprintf(ffrom,"%s",from);
	}
	else sprintf(ffrom, "%s%s",dirpath,tmp);


    strcpy(tmp,to);
    encrypt(tmp);

	if(strcmp(to,"/") == 0)
	{
		to=dirpath;
		sprintf(fto,"%s", to);
	}
	else sprintf(fto, "%s%s",dirpath,tmp);
	
	int res;

	res = rename(ffrom, fto);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;

	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{	
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);


	int res;

	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{	
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;

	res = readlink(fpath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{	
	char ffrom[1000];
	char fto[1000];
    char tmp[1000];
    strcpy(tmp,from);
    encrypt(tmp);

	if(strcmp(from,"/") == 0)
	{
		from=dirpath;
		sprintf(ffrom,"%s",from);
	}
	else sprintf(ffrom, "%s%s",dirpath,tmp);


    strcpy(tmp,to);
    encrypt(tmp);

	if(strcmp(to,"/") == 0)
	{
		to=dirpath;
		sprintf(fto,"%s", to);
	}
	else sprintf(fto, "%s%s",dirpath,tmp);

	int res;

	res = symlink(ffrom, fto);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{	
	char ffrom[1000];
	char fto[1000];
    char tmp[1000];
    strcpy(tmp,from);
    encrypt(tmp);

	if(strcmp(from,"/") == 0)
	{
		from=dirpath;
		sprintf(ffrom,"%s",from);
	}
	else sprintf(ffrom, "%s%s",dirpath,tmp);


    strcpy(tmp,to);
    encrypt(tmp);

	if(strcmp(to,"/") == 0)
	{
		to=dirpath;
		sprintf(fto,"%s", to);
	}
	else sprintf(fto, "%s%s",dirpath,tmp);


	int res;

	res = link(ffrom, fto);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{	
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;

	res = lchown(fpath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{	
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);


	int res;

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	char fpath[1000];
    char tmp[1000];
	if(strstr(path,"/YOUTUBER"))
		strcat(path,".iz1");
    strcpy(tmp,path);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);
	int res;

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char *path, mode_t mode,
                       struct fuse_file_info *fi)
{
	char fpath[1000], tmp[1000];
	if(strstr(path,"/YOUTUBER/"))
		strcat(path,".iz1");
	strcpy(tmp, path);
	encrypt(tmp);

	if(strcmp(tmp,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);
		
	int res;
	printf("tes wkwkwk\n");

	int slash = 0;
	for(int i = 0; i<strlen(path);i++)
		if(path[i]=='/') slash++;
	if(strstr(path,"/YOUTUBER") && slash <= 2){
		printf("tes3 wkwkwk\n");
		res = open(fpath, fi->flags, 0640);
	}
	else{
		printf("tes2 wkwkwk\n");
		res = open(fpath, fi->flags, mode);
	}
	if (res == -1)
			return -errno;

	fi->fh = res;
	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.mkdir		= xmp_mkdir,
	.write		= xmp_write,
	.chmod		= xmp_chmod,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.statfs		= xmp_statfs,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.symlink	= xmp_symlink,
	.link		= xmp_link,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.open		= xmp_open,
	.utimens	= xmp_utimens,
	.create		= xmp_create,
};

int main(int argc, char *argv[])
{
	umask(0);
	scanf("%d",&key);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}