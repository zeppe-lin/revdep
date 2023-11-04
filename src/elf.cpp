//! \file  elf.cpp
//! \brief Elf class implementation.
//!        See COPYING and COPYRIGHT files for corresponding information.

#include <stdexcept>

#include <fcntl.h>
#include <gelf.h>
#include <unistd.h>

#include "elf.h"

using namespace std;

__attribute__((constructor))
static void initialize()
{
  if (elf_version(EV_CURRENT) == EV_NONE)
    throw runtime_error("libelf initialization failure");
}

static bool isValidElf(Elf *elf, int &machine)
{
  GElf_Ehdr ehdr;

  if (elf_kind(elf) != ELF_K_ELF)
    return false;

  if (gelf_getehdr(elf, &ehdr) == NULL)
    return false;

  switch (ehdr.e_type)
  {
    case ET_EXEC: break;
    case ET_DYN:  break;
    default:      return false;
  }

  switch (ehdr.e_ident[EI_OSABI])
  {
    case ELFOSABI_NONE:  break;
    case ELFOSABI_LINUX: break;
    default:             return false;
  }

  switch (ehdr.e_machine)
  {
#if   defined(__i386__)
    case EM_386:      break;
#elif defined(__x86_64__)
    case EM_386:      break;
    case EM_X86_64:   break;
#elif defined(__arm__)
    case EM_ARM:      break;
#elif defined(__aarch64__)
    case EM_AARCH64:  break;
#elif defined(__powerpc__)
    case EM_PPC:      break;
#elif defined(__powerpc64__)
    case EM_PPC64:    break;
#elif defined(__riscv)
    case RM_RISCV:    break;
#else
# error "unsupported architecture"
#endif
    default:          return false;
  }

  machine = ehdr.e_machine;

  return true;
}

static bool getDynamicSection(Elf       *elf,
                              GElf_Shdr &shdr,
                              Elf_Scn   *&scn)
{
  size_t    phdrnum;
  size_t    i;
  GElf_Phdr phdr;

  if (elf_getphdrnum(elf, &phdrnum) == -1)
    return false;

  for (i = 0; i < phdrnum; ++i)
  {
    if (gelf_getphdr(elf, i, &phdr) == NULL)
      continue;

    if (phdr.p_type != PT_DYNAMIC)
      continue;

    scn = gelf_offscn(elf, phdr.p_offset);

    if (scn == NULL)
      continue;

    if (gelf_getshdr(scn, &shdr) == NULL)
      continue;

    if (shdr.sh_type == SHT_DYNAMIC)
      break;
  }

  return (i != phdrnum);
}

static bool readDynamicSection(Elf          *elf,
                               StringVector &needed,
                               StringVector &rpath,
                               StringVector &runpath)
{
  GElf_Shdr shdr;
  Elf_Scn   *scn = NULL;
  Elf_Data  *data;
  size_t    size;
  GElf_Dyn  dyn;

  if (!getDynamicSection(elf, shdr, scn))
    return false;

  data = elf_getdata(scn, NULL);

  if (data == NULL)
    return false;

  size = shdr.sh_size / gelf_fsize(elf, ELF_T_DYN, 1, EV_CURRENT);

  for (size_t i = 0; i < size; ++i)
  {
    if (gelf_getdyn(data, i, &dyn) == NULL)
      break;

    switch (dyn.d_tag)
    {
      case DT_NEEDED:
        needed.push_back(
            elf_strptr(elf, shdr.sh_link, dyn.d_un.d_val)
            );
        break;

      case DT_RPATH:
        split(
            elf_strptr(elf, shdr.sh_link, dyn.d_un.d_val),
            rpath,
            ':'
            );
        break;

      case DT_RUNPATH:
        split(
            elf_strptr(elf, shdr.sh_link, dyn.d_un.d_val),
            runpath,
            ':'
            );
        break;
    }
  }

  return true;
}

Elf::Elf(const string &path)
{
  _initialized = false;

  int fd = open(path.c_str(), O_RDONLY);

  if (fd == -1)
    return;

  Elf *elf = elf_begin(fd, ELF_C_READ_MMAP, NULL);

  if (elf == NULL)
  {
    close(fd);
    return;
  }

  if (   !isValidElf(elf, _machine)
      || !readDynamicSection(elf, _needed, _rpath, _runpath))
  {
    elf_end(elf);
    close(fd);
    return;
  }

  _path = path;
  _initialized = true;

  elf_end(elf);
  close(fd);
}

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
