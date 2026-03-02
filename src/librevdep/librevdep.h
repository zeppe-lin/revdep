/*!
 * \file librevdep.h
 * \brief Umbrella header for librevdep public API.
 *
 * \details
 * Including this header pulls in the complete public surface of the
 * library: context/configuration, the audit engine, optional parallel
 * runner helpers, formatting helpers, and the underlying ELF/package
 * support types.
 *
 * Consumers may include individual headers for finer granularity.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include "elf_cache.h"
#include "elf.h"
#include "pkg.h"
#include "utility.h"

#include "revdep_context.h"
#include "revdep_engine.h"
#include "revdep_parallel.h"
#include "revdep_format.h"
