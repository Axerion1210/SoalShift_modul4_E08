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

#ifdef linux
#define _XOPEN_SOURCE 700
#endif

char cipher[] = "qE1~ YMUR2\"`hNIdPzi\%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV\']jcp5JZ&Xl|\\8s;g<{3.u*W-0";
int key;
static const char *dirpath = "/home/ivan/shift4";

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
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
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

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

        // printf("=====%s\n",de->d_name);

        char *tmp = de->d_name;
		if(strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0)
			continue;
        decrypt(tmp);

		res = (filler(buf, tmp, &st, 0));
			if(res!=0) break;
	}

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

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	char fpath[1000];
    char tmp[1000];

    strcpy(tmp,path);
	if(strncmp(path,"/YOUTUBER/",10)==0)
		sprintf(tmp,"%s.iz1",tmp);
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;
	if (S_ISREG(mode)) {
		if(strncmp(path,"/YOUTUBER/",10)==0)
			res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, 0640);
		else	
			res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	}
	else if (S_ISFIFO(mode))
		res = mkfifo(fpath, mode);
	else
		res = mknod(fpath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
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
	if(strncmp(path,"/YOUTUBER/",10)==0)
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

#ifdef HAVE_UTIMENSAT
static int xmp_utimens(const char *path, const struct timespec ts[2])
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

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int xmp_chmod(const char *path, mode_t mode)
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

	res = chmod(fpath, mode);
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

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

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


	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
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

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	
	#ifdef HAVE_UTIMENSAT
	.utimens	= xmp_utimens,
	#endif
	
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.release	= xmp_release,
	.fsync		= xmp_fsync,

	
};

int main(int argc, char *argv[])
{
	umask(0);
	scanf("%d",&key);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}