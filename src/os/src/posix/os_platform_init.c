/*
 * Copyright(c) 2006 to 2018 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
/****************************************************************
 * Initialization / Deinitialization                            *
 ****************************************************************/

/** \file os/darwin/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include <sys/utsname.h>
#include <assert.h>
#include "os/os.h"

/** \brief Counter that keeps track of number of times os-layer is initialized */
static os_atomic_uint32_t _ospl_osInitCount = OS_ATOMIC_UINT32_INIT(0);

/** \brief OS layer initialization
 *
 * \b os_osInit calls:
 * - \b os_sharedMemoryInit
 * - \b os_threadInit
 */
void os_osInit (void)
{
  uint32_t initCount;

  initCount = os_atomic_inc32_nv(&_ospl_osInitCount);

  if (initCount == 1) {
    os_syncModuleInit();
    os_threadModuleInit();
    os_processModuleInit();
    os_reportInit(false);
  }

  return;
}

/** \brief OS layer deinitialization
 */
void os_osExit (void)
{
  uint32_t initCount;

  initCount = os_atomic_dec32_nv(&_ospl_osInitCount);

  if (initCount == 0) {
    os_reportExit();
    os_processModuleExit();
    os_threadModuleExit();
    os_syncModuleExit();
  } else if ((initCount + 1) < initCount){
    /* The 0 boundary is passed, so os_osExit is called more often than
     * os_osInit. Therefore undo decrement as nothing happened and warn. */
    os_atomic_inc32(&_ospl_osInitCount);
    OS_WARNING("os_osExit", 1, "OS-layer not initialized");
    /* Fail in case of DEV, as it is incorrect API usage */
    assert(0);
  }
  return;
}

/* This constructor is invoked when the library is loaded into a process. */
void __attribute__ ((constructor))
os__osInit(
        void)
{
    os_osInit();
}

/* This destructor is invoked when the library is unloaded from a process. */
void __attribute__ ((destructor))
os__osExit(
        void)
{
    os_osExit();
}
