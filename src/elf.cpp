//! \file  elf.cpp
//! \brief Elf class implementation.
//!
//! This file implements the `Elf` class, which is used to parse and
//! analyze ELF (Executable and Linkable Format) files.
//! It provides functionality to check if a file is a valid ELF, and
//! to extract information such as dynamic dependencies
//! (NEEDED libraries), RPATH, and RUNPATH from the ELF's dynamic
//! section.
//!
//! See COPYING and COPYRIGHT files for corresponding information.

#include <stdexcept> // For std::runtime_error

#include <fcntl.h>   // For file control options (O_RDONLY)
#include <gelf.h>    // For GELF (Generic ELF) library functions
#include <unistd.h>  // For close()

#include "elf.h"

using namespace std;

/*!
 * \brief Global constructor to initialize the libelf library.
 *
 * This function is automatically executed when the shared library is
 * loaded or the program starts.  It initializes the libelf library by
 * setting the ELF version to the current version (EV_CURRENT).  If
 * initialization fails, it throws a std::runtime_error exception.
 */
__attribute__((constructor))
static void
initialize()
{
  if (elf_version(EV_CURRENT) == EV_NONE)
    throw runtime_error("libelf initialization failure");
}

/*!
 * \brief Checks if an Elf object represents a valid and supported ELF
 *        file.
 *
 * This function performs several checks to determine if the provided
 * Elf object is valid for processing by this class.  It verifies the
 * ELF kind, the ELF type (executable or shared library), the OSABI,
 * and the machine architecture.
 *
 * \param elf     Pointer to the Elf object to validate.
 * \param machine [out] Integer reference to store the
 *                machine type if valid ELF.
 *
 * \return True if the Elf object is a valid and supported ELF file,
 *         False otherwise.
 */
static bool
isValidElf(Elf *elf, int &machine)
{
  GElf_Ehdr ehdr; //!< GELF ELF header structure

  if (elf_kind(elf) != ELF_K_ELF)
    return false; // Not an ELF file

  if (gelf_getehdr(elf, &ehdr) == NULL)
    return false; // Failed to retrieve ELF header

  switch (ehdr.e_type)
  {
    case ET_EXEC: break;         // Executable file
    case ET_DYN:  break;         // Shared library
    default:      return false;  // Unsupported ELF type
  }

  switch (ehdr.e_ident[EI_OSABI])
  {
    case ELFOSABI_NONE:  break;         // No OS ABI
    case ELFOSABI_LINUX: break;         // Linux OS ABI
    default:             return false;  // Unsupported OS ABI
  }

  switch (ehdr.e_machine)
  {
#if   defined(__i386__)
    case EM_386:        break;  // Intel 80386
#elif defined(__x86_64__)
    case EM_386:        break;  // Intel 80386 (compat.)
    case EM_X86_64:     break;  // AMD x86-64
#elif defined(__arm__)
    case EM_ARM:        break;  // ARM
#elif defined(__aarch64__)
    case EM_AARCH64:    break;  // ARM AARCH64
#elif defined(__loongarch__)
    case EM_LOONGARCH:  break;  // LoongArch
#elif defined(__powerpc__)
    case EM_PPC:        break;  // PowerPC
#elif defined(__powerpc64__)
    case EM_PPC:        break;  // PowerPC (compat.)
    case EM_PPC64:      break;  // PowerPC 64-bit
#elif defined(__riscv)
    case EM_RISCV:      break;  // RISC-V
#else
# error "unsupported architecture"
#endif
    default:            return false;  // Unsupported machine
  }

  machine = ehdr.e_machine;  // Store machine type

  return true;  // Valid ELF file
}

/*!
 * \brief Retrieves the dynamic section header and section descriptor
 *        from an Elf object.
 *
 * This function iterates through the program headers of the ELF file
 * to find the program header of type PT_DYNAMIC.  Once found, it
 * retrieves the corresponding section descriptor and section header
 * for the dynamic section.
 *
 * \param elf  Pointer to the Elf object.
 * \param shdr [out] Reference to a GElf_Shdr structure to store the
 *             section header.
 * \param scn  [out] Pointer reference to an Elf_Scn pointer to store
 *             the section descriptor.
 *
 * \return True if the dynamic section is found, False otherwise.
 */
static bool
getDynamicSection(Elf *elf, GElf_Shdr &shdr, Elf_Scn *&scn)
{
  size_t    phdrnum;  // Number of program headers
  size_t    i;        // Loop counter
  GElf_Phdr phdr;     // GELF program header structure

  if (elf_getphdrnum(elf, &phdrnum) == -1)
    return false;  // Failed to get program header num

  for (i = 0; i < phdrnum; ++i)
  {
    if (gelf_getphdr(elf, i, &phdr) == NULL)
      continue;  // Failed to get program header, next

    if (phdr.p_type != PT_DYNAMIC)
      continue;  // Not dynamic program header, next

    scn = gelf_offscn(elf, phdr.p_offset);  // Get section description

    if (scn == NULL)
      continue;  // Failed to get section description, next

    if (gelf_getshdr(scn, &shdr) == NULL)
      continue;  // Failed to get section header, next

    if (shdr.sh_type == SHT_DYNAMIC)
      break;  // Found dynamic section, exit loop
  }

  return (i != phdrnum);  // Return true if dynamic section
}

/*!
 * \brief Reads the dynamic section of an ELF file and extracts
 *        NEEDED, RPATH, and RUNPATH entries.
 *
 * This function reads the dynamic section entries from the ELF file.
 * It extracts the names of required shared libraries (NEEDED), the
 * RPATH entries, and the RUNPATH entries.  The extracted names and
 * paths are stored in the provided StringVector objects.
 *
 * \param elf     Pointer to the Elf object.
 * \param needed  [out] StringVector to store NEEDED libs.
 * \param rpath   [out] StringVector to store RPATH entries
 *                (split by ':').
 * \param runpath [out] StringVector to store RUNPATH entries
 *                (split by ':').
 *
 * \return True if the dynamic section was successfully read,
 *         False otherwise.
 */
static bool
readDynamicSection(Elf *elf, StringVector &needed, StringVector &rpath,
    StringVector &runpath)
{
  GElf_Shdr shdr;         // GELF section header structure
  Elf_Scn   *scn = NULL;  // ELF section descriptor
  Elf_Data  *data;        // ELF data buffer
  size_t    size;         // Size of dynamic section
  GElf_Dyn  dyn;          // GELF dynamic section entry

  if (!getDynamicSection(elf, shdr, scn))
    return false;  // Failed to get dynamic section

  data = elf_getdata(scn, NULL);  // Get data for dynamic section

  if (data == NULL)
    return false;  // Failed to get dynamic section data

  // Calc number of dynamic entries
  size = shdr.sh_size / gelf_fsize(elf, ELF_T_DYN, 1, EV_CURRENT);

  for (size_t i = 0; i < size; ++i)
  {
    if (gelf_getdyn(data, i, &dyn) == NULL)
      break;  // Failed to get dynamic entry, break

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
            rpath,  // Split RPATH string to vector
            ':'     // Delimiter for RPATH entries
            );
        break;

      case DT_RUNPATH:
        split(
            elf_strptr(elf, shdr.sh_link, dyn.d_un.d_val),
            runpath,  // Split RUNPATH string to vector
            ':'       // Delimiter to RUNPATH entries
            );
        break;
    }
  }

  return true;  // Dynamic section read successfully
}

/*!
 * \class Elf
 * \brief Represents an ELF file and provides methods to analyze it.
 *
 * This class encapsulates the logic for parsing and extracting
 * information from ELF files, such as dynamic dependencies, RPATH,
 * and RUNPATH.
 */
Elf::Elf(const string &path)
{
  _initialized = false;

  int fd = open(path.c_str(), O_RDONLY);  // Open ELF file

  if (fd == -1)
    return; // Failed to open file

  Elf *elf = elf_begin(fd, ELF_C_READ_MMAP, NULL);  // Init Elf

  if (elf == NULL)
  {
    close(fd);  // Close FD on error
    return;     // Failed to initialize Elf object
  }

  if (   !isValidElf(elf, _machine)
      || !readDynamicSection(elf, _needed, _rpath, _runpath))
  {
    elf_end(elf);  // Finalize Elf obj
    close(fd);     // Close FD
    return;        // ELF validation or dynamic read fail
  }

  _path = path;         // Store file path
  _initialized = true;  // Mark initialized

  elf_end(elf);  // Finalize elf object
  close(fd);     // Close FD
}

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
