#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

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
	char fpath[1000],a[1000];
    strcpy(a,path);
    encrypt(a);

    sprintf(fpath,"%s%s",dirpath,a);

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
    encrypt(tmp);

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,tmp);

	int res;
	if (S_ISREG(mode)) {
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
	if(strncmp(path,"/YOUTUBE/",9)==0)
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
	int fd;
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

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.mkdir		= xmp_mkdir,
	.write		= xmp_write,
	.chmod		= xmp_chmod,
	.mknod		= xmp_mknod,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,

};

int main(int argc, char *argv[])
{
	umask(0);
	scanf("%d",&key);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}