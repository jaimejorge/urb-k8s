// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#ifndef __PROCESS_EXECUTOR_HPP__
#define __PROCESS_EXECUTOR_HPP__

#include <process/defer.hpp>
#include <process/deferred.hpp>
#include <process/id.hpp>
#include <process/process.hpp>

#include <stout/thread_local.hpp>

namespace process {

// Provides an abstraction that can take a standard function object
// and defer it without needing a process. Each converted function
// object will get execute serially with respect to one another when
// invoked.
class Executor
{
public:
  Executor() : process(ID::generate("__executor__"))
  {
    spawn(process);
  }

  ~Executor()
  {
    terminate(process);
    wait(process);
  }

  void stop()
  {
    terminate(&process);

    // TODO(benh): Note that this doesn't wait because that could
    // cause a deadlock ... thus, the semantics here are that no more
    // dispatches will occur after this function returns but one may
    // be occurring concurrently.
  }

  template <typename F>
  _Deferred<F> defer(F&& f)
  {
    return _Deferred<F>(process.self(), std::forward<F>(f));
  }


private:
  // Not copyable, not assignable.
  Executor(const Executor&);
  Executor& operator=(const Executor&);

  ProcessBase process;
};


// Per thread executor pointer. We use a pointer to lazily construct the
// actual executor.
extern THREAD_LOCAL Executor* _executor_;

#define __executor__                                                    \
  (_executor_ == nullptr ? _executor_ = new Executor() : _executor_)

} // namespace process {

#endif // __PROCESS_EXECUTOR_HPP__
