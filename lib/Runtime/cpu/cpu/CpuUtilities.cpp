/*
 * ###########################################################################
 * Copyright (c) 2010, Los Alamos National Security, LLC.
 * All rights reserved.
 *
 *  Copyright 2010. Los Alamos National Security, LLC. This software was
 *  produced under U.S. Government contract DE-AC52-06NA25396 for Los
 *  Alamos National Laboratory (LANL), which is operated by Los Alamos
 *  National Security, LLC for the U.S. Department of Energy. The
 *  U.S. Government has rights to use, reproduce, and distribute this
 *  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
 *  LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
 *  FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
 *  derivative works, such modified software should be clearly marked,
 *  so as not to confuse it with the version available from LANL.
 *
 *  Additionally, redistribution and use in source and binary forms,
 *  with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Los Alamos National Security, LLC, Los
 *      Alamos National Laboratory, LANL, the U.S. Government, nor the
 *      names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 * ###########################################################################
 *
 * Notes
 *  system_rt for the non-numa case (no hwloc)
 * #####
 */

#include "scout/Runtime/Settings.h"
#include "scout/Runtime/cpu/CpuUtilities.h"
#include <unistd.h>

using namespace scout;
using namespace scout::cpu;

namespace scout{
  namespace  cpu{

    class system_rt_{
    public:
      system_rt_(system_rt* o)
      : o_(o) {
        Settings *settings = Settings::Instance();
        totalProcessingUnits_ = sysconf(_SC_NPROCESSORS_ONLN);

         int val = settings->nThreads();
         if (val) nThreads_ = val;
         else nThreads_ = totalProcessingUnits_;
         if (settings->debug()) std::cerr << "nThreads " << nThreads_ << std::endl;
      }

      ~system_rt_(){
      }

      size_t totalProcessingUnits() const {
        return  totalProcessingUnits_;
      }

      size_t nThreads() const {
       return nThreads_;
      }
    private:
      system_rt* o_;
      size_t totalProcessingUnits_;
      size_t nThreads_;
    };
  } // end namespace cpu;
} // end namespace scout


system_rt::system_rt(){
  x_ = new system_rt_(this);
}

system_rt::~system_rt(){
  delete x_;
}

size_t system_rt::totalSockets() const {
  return 1;
}

size_t system_rt::totalNumaNodes() const {
  return 1;
}

size_t system_rt::totalCores() const {
  return x_->totalProcessingUnits();
}

size_t system_rt::totalProcessingUnits() const {
  return x_->totalProcessingUnits();
}

size_t system_rt::processingUnitsPerCore() const {
  return x_->totalProcessingUnits();
}

size_t system_rt::numaNodesPerSocket() const {
  return 1;
}

size_t system_rt::memoryPerSocket() const {
  return 0;
}

size_t system_rt::memoryPerNumaNode() const {
  return 0;
}

size_t system_rt::processingUnitsPerNumaNode() const {
  return x_->totalProcessingUnits();
}

std::string system_rt::treeToString() const {
  return NULL;
}

void* system_rt::allocArrayOnNumaNode(size_t size, size_t nodeId) {
  return NULL;
}


void system_rt::freeArrayFromNumaNode(void* m) {
}


bool system_rt::bindThreadToNumaNode(size_t nodeId) {
  return false;
}

int system_rt::bindThreadOutside(pthread_t& thread) {
  return 0;
}

int system_rt::bindThreadInside() {
  return 0;
}


size_t system_rt::nThreads() {
  return x_->nThreads();
}

size_t system_rt::nDomains() {
  return 1;
}
