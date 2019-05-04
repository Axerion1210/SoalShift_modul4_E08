# SoalShift_modul4_E08

#### Bagas Yanuar Sudrajad - 05111740000074 
#### Octavianus Giovanni Yaunatan - 05111740000113

## SOAL 1

Semua nama file dan folder harus terenkripsi 

Enkripsi yang Atta inginkan sangat sederhana, yaitu [Caesar cipher](https://en.wikipedia.org/wiki/Caesar_cipher). Namun, Kusuma mengatakan enkripsi tersebut sangat mudah dipecahkan. Dia menyarankan untuk *character list* diekspansi,tidak hanya alfabet, dan diacak. Berikut *character list* yang dipakai:

```
qE1~ YMUR2"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\8s;g<{3.u*W-0
```

Misalkan ada file bernama “halo” di dalam folder “INI_FOLDER”, dan *key* yang dipakai adalah 17, maka:

**“INI_FOLDER/halo”** saat belum di-*mount* maka akan bernama **“n,nsbZ]wio/QBE#”**, saat telah di-*mount* maka akan otomatis terdekripsi kembali menjadi **“INI_FOLDER/halo”** .

Perhatian: Karakter ‘/’ adalah karakter ilegal dalam penamaan file atau folder dalam *NIX, maka dari itu dapat diabaikan

### Langkah-langkah

- Buat fungsi untuk enkripsi dan dekripsi nama.

- Menerapkan enkripsi dan dekripsi:

  Enkripsi diterapkan untuk penulisan file pada sistem, sedangkan dekripsi digunakan untuk berinteraksi dengan user yang mengakses FUSE.

### Implementasi

Program enkripsi:

```c
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
```

Program dekripsi:

```c
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
```

Semua fungsi lainnya yang menggunakan nama file atau path disunting menjadi menggunakan fungsi di atas, yaitu enkripsi untuk berkomunikasi dengan sistem, dan dekripsi untuk berkomunikasi dengan user. Contoh:

untuk menghapus direktori:

```c
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	char pathMiris[1000];
	char miris[15] = "filemiris.txt";
	encrypt(miris);

	sprintf(pathMiris, "%s/%s", dirpath,miris);
	FILE *rusak = fopen(pathMiris, "a");

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

		stat(temp, &info);
		struct passwd *pw = getpwuid(info.st_uid);
		struct group  *gr = getgrgid(info.st_gid);
		int readable = access(temp, R_OK);			//return 0 if it is readable
		char date[30];

		if (de->d_type == DT_REG && strcmp(temp, pathMiris)!=0 && readable!=0 && (strcmp(pw->pw_name, "chipset") || strcmp(pw->pw_name, "ic_controller")) && strcmp(gr->gr_name, "rusak")) {
			strftime(date, 30, "%Y-%m-%d %H:%M:%S", localtime(&(info.st_atime)));
			fprintf(rusak, "%s\t\t%d\t\t%d\t\t%s\n", de->d_name, gr->gr_gid, pw->pw_uid, date);
			remove(temp);
			continue;
		}

        decrypt(de->d_name);
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	fclose(rusak);
	closedir(dp);
	return 0;
}
```

## SOAL 2

Semua file video yang tersimpan secara terpecah-pecah (*splitted*) harus secara otomatis tergabung (*joined*) dan diletakkan dalam folder “Videos”

Urutan operasi dari kebutuhan ini adalah:

1. 1. Tepat saat sebelum file system di-*mount*

   2. 1. Secara otomatis folder “Videos” terbuat di root directory file system
      2. Misal ada sekumpulan file pecahan video bernama “computer.mkv.000”, “computer.mkv.001”, “computer.mkv.002”, dst. Maka secara otomatis file pecahan tersebut akan di-*join* menjadi file video “computer.mkv”

Untuk mempermudah kalian, dipastikan hanya video file saja yang terpecah menjadi beberapa file. File pecahan tersebut dijamin terletak di root folder fuse

1. 1. 1. Karena mungkin file video sangat banyak sehingga mungkin saja saat menggabungkan file video, file system akan membutuhkan waktu yang lama untuk sukses ter-*mount*. Maka pastikan saat akan menggabungkan file pecahan video, file system akan membuat **1 thread/proses(fork) baru** yang dikhususkan untuk menggabungkan file video tersebut
      2. Pindahkan seluruh file video yang sudah ter-*join* ke dalam folder “Videos”
      3. Jangan tampilkan file pecahan di direktori manapun

   2. Tepat saat file system akan di-*unmount*

   3. 1. Hapus semua file video yang berada di folder “Videos”, tapi jangan hapus file pecahan yang terdapat di root directory file system
      2. Hapus folder “Videos” 

### Langkah-langkah

### Implementasi

## SOAL 3

Sebelum diterapkannya file system ini, Atta pernah diserang oleh hacker LAPTOP_RUSAK yang menanamkan user bernama “chipset” dan “ic_controller” serta group “rusak” yang tidak bisa dihapus. Karena paranoid, Atta menerapkan aturan pada file system ini untuk menghapus “file bahaya” yang memiliki spesifikasi:

- Owner Name 	: ‘chipset’ atau ‘ic_controller’
- Group Name	: ‘rusak’
- Tidak dapat dibaca

Jika ditemukan file dengan spesifikasi tersebut ketika membuka direktori, Atta akan menyimpan nama file, group ID, owner ID, dan waktu terakhir diakses dalam file “filemiris.txt” (format waktu bebas, namun harus memiliki jam menit detik dan tanggal) lalu menghapus “file bahaya” tersebut untuk mencegah serangan lanjutan dari LAPTOP_RUSAK.

### Langkah-langkah

Lakukan perubahan pada fungsi readdir untuk mendeteksi file yang sesuai karakteristik

### Implementasi

Fungsi readdir

```c

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

	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	char miris[15] = "filemiris.txt";
	char pathMiris[1000];
	encrypt(miris);

	sprintf(pathMiris, "%s/%s", dirpath,miris);
	FILE *rusak = fopen(pathMiris, "a");

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

		char temp2[1000];
		sprintf(temp2, "%s/%s", fpath, de->d_name);
		stat(temp2, &info);
		struct passwd *pw = getpwuid(info.st_uid);
		struct group  *gr = getgrgid(info.st_gid);
		int readable = access(temp2, R_OK);			//return 0 if it is readable
		char date[30];

		if (de->d_type == DT_REG && readable!=0 && strcmp(temp2,pathMiris) != 0 && (strcmp(pw->pw_name, "chipset")==0 || strcmp(pw->pw_name, "ic_controller")==0) && strcmp(gr->gr_name, "rusak")==0) {
			strftime(date, 30, "%Y-%m-%d %H:%M:%S", localtime(&(info.st_atime)));
			decrypt(de->d_name);
			// printf("123123123 %s\n",date);
			fprintf(rusak, "%s\t\t%d\t\t%d\t\t%s\n", de->d_name, gr->gr_gid, pw->pw_uid, date);
			remove(temp2);
			continue;
		}

        decrypt(de->d_name);
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	fclose(rusak);
	closedir(dp);
	return 0;
}
```



## SOAL 4

Pada folder **YOUTUBER,** setiap membuat folder permission foldernya akan otomatis menjadi 750. Juga ketika membuat file permissionnya akan otomatis menjadi 640 dan ekstensi filenya akan bertambah “***.iz1****”.* File berekstensi “***.iz1****”* tidak bisa diubah permissionnya dan memunculkan error bertuliskan “*File ekstensi iz1 tidak boleh diubah permissionnya.*”

### Langkah-langkah

- Buat fungsi mkdir, create, utimens, write, dan chmod yang sesuai dengan ketentuan.

  mkdir untuk membuat direktori. create untuk membuat file. write untuk nano. utimens untuk akses properti waktu file atau folder.

### Implementasi

Fungsi mkdir

```c
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
	if(strstr(tmp,"/YOUTUBER")) //jika dalam folder youtuber (ada substring youtuber)
		res = mkdir(fpath, 0750);
	else // bukan dalam folder youtuber	
		res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;
	return 0;
}
```

Fungsi create

```c
static int xmp_create(const char *path, mode_t mode,
                       struct fuse_file_info *fi)
 {
        char fpath[1000], tmp[1000];
		if(strstr(path,"/YOUTUBER")) 
            //jika dalam folder youtuber (terdapat substring/youtuber)
			strcat(path,".iz1"); // tambah ekstensi ".z1"
		strcpy(tmp, path); //update path
		encrypt(tmp);

		if(strcmp(tmp,"/") == 0)
		{
			path=dirpath;
			sprintf(fpath,"%s",path);
		}
		else sprintf(fpath, "%s%s",dirpath,tmp);
			
		int res;

		if(strstr(path,"/YOUTUBER") != 0) // penentuan permission
			res = open(fpath, fi->flags, 0640);
		else
			res = open(fpath, fi->flags, mode);
 
        if (res == -1)
                return -errno;
 
        fi->fh = res;
        return 0;
 }
```

Fungsi write

```c
static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
    char tmp[1000];
	if(strstr(path,"/YOUTUBER")) // jika sesuai kriteria
		strcat(path,".iz1"); // tambahkan ekstensi
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
```

Fungsi chmod

```c
static int xmp_chmod(const char *path, mode_t mode)
{
	char fpath[1000];
    char tmp[1000];
    strcpy(tmp,path);
    encrypt(tmp);

	if(strstr(path,".iz1") && strstr(path,"/YOUTUBER")){ // jika sesuai kriteria
		pid_t child_id;
		child_id = fork(); // buat thread untuk mengeluarkan error
		if (child_id < 0) {
			exit(EXIT_FAILURE);
		}
		
        //tampilkan error
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
```

Fungsi utimens

```c
static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	char fpath[1000];
    char tmp[1000];
	if(strstr(path,"/YOUTUBER"))// jika pada folder youtuber
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
```



## SOAL 5

Ketika mengedit suatu file dan melakukan save, maka akan terbuat folder baru bernama **Backup** kemudian hasil dari save tersebut akan disimpan pada backup dengan nama **namafile_[timestamp].ekstensi.** Dan ketika file asli dihapus, maka akan dibuat folder bernama **RecycleBin**, kemudian file yang dihapus beserta semua backup dari file yang dihapus tersebut (jika ada) di zip dengan nama **namafile_deleted_[timestamp].zip** dan ditaruh ke dalam folder RecycleBin (file asli dan backup terhapus). Dengan format **[timestamp]** adalah **yyyy-MM-dd_HH:mm:ss**

### Langkah-langkah

Modifikasi fungsi write dan unlink sesuai permintaan soal

### Implementasi

Fungsi Write

```c
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

	encrypt(path);
	sprintf(tmp, "%s/%s", dirpath,path);
	if(access(tmp, R_OK)<0)				//jika tidak ada
		return res;
	char backup[] = "Backup", pathBackup[1000];
	encrypt(backup);
	strncpy(pathBackup, path, lastCharPos(path, '/'));// path directory
	pathBackup[lastCharPos(path, '/')] = '\0'; // kasih end
	sprintf(tmp, "%s/%s", pathBackup, backup); //  path backupan
	strcpy(pathBackup, tmp); // pathbackuo = tmp = path file backup
	sprintf(tmp, "%s%s", dirpath, pathBackup); // tmp ditambah dirpath
	mkdir(tmp, 0777); // mkdir
	decrypt(path);
	char filePathWithoutExt[1000], ext[100], timestamp[1000], fileNameBackup[1000], ch;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(timestamp, "%04d-%02d-%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	int posSlash = lastCharPos(path, '/');
	int posDot = lastCharPos(path, '.');
	
	if (posDot==0) {
		posDot = strlen(path);
		ext[0] = '\0';
	}
	else{
		strcpy(ext, path+posDot);
		if (strcmp(ext, ".swp")==0)	
			return res;
	}
	// /satu/dua.txt
	strncpy(filePathWithoutExt, path+posSlash+1, posDot-(posSlash+1)); // nama file tanpa ext
	filePathWithoutExt[posDot-(posSlash+1)] = '\0'; //end of string
	sprintf(fileNameBackup,"%s_%s%s", filePathWithoutExt, timestamp, ext);//nama akhir
	encrypt(fileNameBackup); 
	encrypt(path);
	sprintf(tmp, "%s%s", dirpath, path); // path akhir
	FILE *source = fopen(tmp, "r");
	sprintf(tmp, "%s%s/%s", dirpath, pathBackup, fileNameBackup);
	FILE *target = fopen(tmp, "w");
	while ((ch = fgetc(source)) != EOF)
		fprintf(target, "%c", ch);
	fclose(target);
	fclose(source);
	return res;
}

```

Fungsi Hapus

```c
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


	sprintf(zipname, "%s_deleted_%s\0", rawname, timestamp);
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
```

