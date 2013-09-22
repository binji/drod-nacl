#include <assert.h>
#include <fcntl.h>
#include <libtar.h>
#include <SDL_main.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

extern int drod_main(int argc, char *argv[]);

static void extract_tar(const char* src, const char* dst) {
  TAR* tar;
  int ret = tar_open(&tar, const_cast<char*>(src), NULL, O_RDONLY, 0,
                     TAR_VERBOSE);
  assert(ret == 0);

  ret = tar_extract_all(tar, const_cast<char*>(dst));
  assert(ret == 0);

  ret = tar_close(tar);
  assert(ret == 0);
}

int main(int argc, char* argv[]) {
  umount("/");
  mount("", "/", "memfs", 0, NULL);
//  mount("", "/var", "html5fs", 0, "type=PERSISTENT");
  mount("", "/var", "memfs", 0, NULL);
  mount("", "/tars", "httpfs", 0, NULL);

  printf("Extracting usr...\n");
  extract_tar("/tars/drod_usr.tar", "/");  // read-only data
  printf("Done.\n");

#if 0
  struct stat statbuf;
  // Test if an expected file is there...
  int ret = stat("/var/games/drod/drod2_0.dat", &statbuf);  // read/write data.
  if (ret != 0) {
    printf("Extracting var...\n");
    extract_tar("/tars/drod_var.tar", "/");
    printf("Done.\n");
  }

  setenv("DROD_4_0_DAT_PATH", "/var/games/drod", 1);
  setenv("DROD_4_0_RES_PATH", "/usr/local/share/games/drod", 1);
#endif

  printf("Running main...\n");
  return drod_main(argc, argv);
}
