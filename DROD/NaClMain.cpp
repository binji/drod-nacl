#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libtar.h>
#include <SDL_main.h>
#include <stdio.h>
#if 0
#include <sys/mount.h>
#else
#include "nacl_io/nacl_io.h"
#endif
#include <sys/stat.h>
#include <unistd.h>

// TODO(binji): hack for pepper_29
#include "nacl_io/kernel_intercept.h"
#include "nacl_io/kernel_proxy.h"
#include "nacl_io/mount_mem.h"
#include "nacl_io/mount_node_dir.h"
#include "nacl_io/mount_node_mem.h"
#include "sdk_util/auto_lock.h"
#include <ppapi/cpp/module.h>
#include <vector>

class MountNodeMemHack : public MountNode {
 public:
  explicit MountNodeMemHack(Mount* mount): MountNode(mount) {
    stat_.st_mode |= S_IFREG;
  }

  using MountNode::Init;

  // Normal read/write operations on a file
  virtual Error Read(size_t offs, void* buf, size_t count, int* out_bytes) {
    *out_bytes = 0;

    AutoLock lock(&lock_);
    if (count == 0)
      return 0;

    size_t size = stat_.st_size;

    if (offs + count > size) {
      count = size - offs;
    }

    memcpy(buf, &data_[offs], count);
    *out_bytes = static_cast<int>(count);
    return 0;
  }

  virtual Error Write(size_t offs,
                      const void* buf,
                      size_t count,
                      int* out_bytes) {
    *out_bytes = 0;
    AutoLock lock(&lock_);

    if (count == 0)
      return 0;

    if (count + offs > stat_.st_size) {
      Resize(count + offs);
      count = stat_.st_size - offs;
    }

    memcpy(&data_[offs], buf, count);
    *out_bytes = static_cast<int>(count);
    return 0;
  }

  virtual Error FTruncate(off_t new_size) {
    AutoLock lock(&lock_);
    Resize(new_size);
    return 0;
  }

 private:
  void Resize(off_t new_size) {
    if (new_size > data_.capacity()) {
      // While the node size is small, grow exponentially. When it starts to get
      // larger, grow linearly.
      size_t extra = std::min<size_t>(new_size, 16 * 1024 * 1024);
      data_.reserve(new_size + extra);
    } else if (new_size < stat_.st_size) {
      // Shrink to fit. std::vector usually doesn't reduce allocation size, so
      // use the swap trick.
      std::vector<char>(data_).swap(data_);
    }
    data_.resize(new_size);
    stat_.st_size = new_size;
  }

  std::vector<char> data_;
};

class MountNodeDirHack : public MountNodeDir {
 public:
  using MountNodeDir::AddChild;
};

class MountMemHack : public MountMem {
 public:
  virtual Error Open(const Path& path, int mode, ScopedMountNode* out_node) {
    out_node->reset(NULL);

    ScopedMountNode node;
    Error error = FindNode(path, 0, &node);
    if (error) {
      if ((mode & O_CREAT) == 0)
        return ENOENT;

      // Find the parent.
      ScopedMountNode parent;
      error = FindNode(path.Parent(), S_IFDIR, &parent);
      if (error)
        return error;

      // Initialize the new node.
      MountNodeMemHack* hack_node = new MountNodeMemHack(this);
      node.reset(hack_node);
      error = hack_node->Init(OpenModeToPermission(mode));
      if (error)
        return error;

      MountNodeDirHack* hack_parent =
          static_cast<MountNodeDirHack*>(parent.get());
      error = hack_parent->AddChild(path.Basename(), node);
      if (error)
        return error;

      *out_node = node;
      return 0;
    }

    return MountMem::Open(path, mode, out_node);
  }
};

class KernelProxyHack : public KernelProxy {
 public:
  KernelProxyHack() {}
  void Init(PepperInterface* ppapi) {
    KernelProxy::Init(ppapi);
    factories_["memfs"] = MountMemHack::Create<MountMemHack>;
  }

  virtual int fchmod(int fd, int prot) { return 0; }
};

extern "C" {
int utime(const char* filename, const void* times) {
  return 0;
}
}
// END HACK

extern int drod_main(int argc, char *argv[]);

static int extract_all(TAR* tar, const char* dst) {
  char *filename;
  char buf[MAXPATHLEN];
  int i;

  while ((i = th_read(tar)) == 0) {
    filename = th_get_pathname(tar);
    if (tar->options & TAR_VERBOSE)
      th_print_long_ls(tar);
    if (dst != NULL)
      snprintf(buf, sizeof(buf), "%s/%s", dst, filename);
    else
      strlcpy(buf, filename, sizeof(buf));
    if (th_get_size(tar) == 0)
      mkdir(buf, 0777);
    else if (tar_extract_file(tar, buf) != 0) {
      printf("Failed to extract to \"%s\". errno=%d\n", buf, errno);
      return -1;
    }
  }

  return (i == 1 ? 0 : -1);
}

static void extract_tar(const char* src, const char* dst) {
  TAR* tar;
  int ret = tar_open(&tar, const_cast<char*>(src), NULL, O_RDONLY, 0,
                     TAR_VERBOSE);
  assert(ret == 0);

  ret = extract_all(tar, dst);
  assert(ret == 0);

  ret = tar_close(tar);
  assert(ret == 0);
}

int main(int argc, char* argv[]) {
  // TODO(binji): hack for pepper_29.
  ki_uninit();
  pp::Module* module = pp::Module::Get();
  PPB_GetInterface browser_interface = module->get_browser_interface();
  PP_Instance instance = module->current_instances().begin()->first;
  ki_init_ppapi(new KernelProxyHack, instance, browser_interface);
  // END HACK
  
  umount("/");
  mount("", "/", "memfs", 0, NULL);
  mount("", "/home", "html5fs", 0, "type=PERSISTENT");
//  mount("", "/home", "memfs", 0, NULL);
  mount("", "/.tars", "httpfs", 0, NULL);

  setenv("HOME", "/home/drod", 1);
  mkdir("/home/drod", 0777);

  printf("Extracting usr...\n");
  extract_tar("/.tars/drod_usr.tar", "/");  // read-only data
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
