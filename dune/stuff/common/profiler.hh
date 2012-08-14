#ifndef DUNE_STUFF_PROFILER_HH_INCLUDED
#define DUNE_STUFF_PROFILER_HH_INCLUDED

#include "misc.hh"
#include "debug.hh"
#include "filesystem.hh"
#include "parameter/container.hh"
#include "math.hh"
#include "runinfo.hh"

#include <dune/common/exceptions.hh>
#include <dune/common/deprecated.hh>
#include <dune/common/mpihelper.hh>

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <ctime>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/config.hpp>
#include <boost/timer/timer.hpp>
#include <boost/shared_ptr.hpp>

namespace Dune {
namespace Stuff {
namespace Common {

class Profiler;

//! Stuff::Profiler global instance
Profiler& profiler();

//! wraps name, start- and end time for one timing section
struct TimingData
{
private:
  boost::shared_ptr<boost::timer::cpu_timer> timer_;

public:
  std::string name;
  TimingData(const std::string _name = "blank")
    : timer_(new boost::timer::cpu_timer)
    , name(_name)
  {
    timer_->start();
  }

  void stop()
  {
    timer_->stop();
  }

  /** \return time elapsed since object construction in milliseconds
   *  \note since typical resolutions for user+system time are 10-15ms the nanosecond results are scaled accordingly
   **/
  boost::timer::nanosecond_type delta() const
  {
    return (timer_->elapsed().user + timer_->elapsed().system) / 1e6;
  }
};

/** \brief simple inline profiling class
   *  - User can set as many (even nested) named sections whose total (=system+user) time will be computed across all
  *program
   * instances.\n
   *  - Provides csv-conform output of process-averaged runtimes.
   * \todo this could go into libdune-stuff
   **/
class Profiler
{
  friend Profiler& profiler();

protected:
  Profiler();
  ~Profiler()
  {
  }

  typedef std::map<std::string, std::pair<bool, TimingData>> KnownTimersMap;
  //! section name -> seconds
  typedef std::map<std::string, long> Datamap;
  //! "Run idx" -> Datamap = section name -> seconds
  typedef std::vector<Datamap> DatamapVector;

public:
  typedef std::vector<Dune::Stuff::Common::RunInfo> InfoContainer;
  typedef std::map<std::string, InfoContainer> InfoContainerMap;

  //! set this to begin a named section
  void startTiming(const std::string section_name);
  //! appends int to section name
  void startTiming(const std::string section_name, const int i);

  //! stop named section's counter
  long stopTiming(const std::string section_name);
  //! appends int to section name
  long stopTiming(const std::string section_name, const int i);

  //! set elapsed time back to 0 for section_name
  void resetTiming(const std::string section_name);
  //! appends int to section name
  void resetTiming(const std::string section_name, const int i);

  //! get runtime of section in current run in milliseconds
  long getTiming(const std::string section_name) const;
  //! appends int to section name
  long getTiming(const std::string section_name, const int i) const;

  //! get runtime of section in run run_number in milliseconds
  long getTimingIdx(const std::string section_name, const int run_number) const;

  /** output to currently pre-defined (csv) file, does not output individual run results, but average over all recorded
   * results
     **/
  void outputAveraged(const int refineLevel, const long numDofs, const double scale_factor = 1.0) const;

  //! default proxy for output
  void output(const InfoContainer& run_infos, const double scale_factor = 1.0) const;

  //! proxy for output of a map of runinfos
  void outputMap(const InfoContainerMap& run_infos_map, const double scale_factor = 1.0) const;

  //! file-output the named sections only
  void outputTimings(const std::string filename) const;
  void outputTimings(std::ostream& out = std::cout) const;
  /** call this with correct numRuns <b> before </b> starting any profiling
     *  if you're planning on doing more than one iteration of your code
     *  called once fromm ctor with numRuns=1
     **/
  void reset(const int numRuns);

  //! simple counter, usable to count how often a single piece of code is called
  void addCount(const int num);

  //! call this after one iteration of your code has finished. increments current run number and puts new timing data
  // into the vector
  void nextRun();

  void setOutputdir(const std::string dir);

  //! a utility class to time a limited scope of code
  class ScopedTiming : public boost::noncopyable
  {
    const std::string section_name_;

  public:
    inline ScopedTiming(const std::string& section_name)
      : section_name_(section_name)
    {
      Profiler::instance().startTiming(section_name_);
    }

    inline ~ScopedTiming()
    {
      Profiler::instance().stopTiming(section_name_);
    }
  };

private:
  DatamapVector datamaps_;
  unsigned int current_run_number_;
  //! runtime tables etc go there
  std::string output_dir_;
  // debug counter, only outputted in debug mode
  std::map<int, int> counters_;

  KnownTimersMap known_timers_map_;
  const std::string csv_sep;

  static Profiler& instance()
  {
    static Profiler pf;
    return pf;
  }

  /** output to \param filename
     * \tparam CollectiveCommunication should be Dune::CollectiveCommunication< MPI_Comm / double >
     **/
  void outputCommon(const InfoContainer& run_infos, const boost::filesystem::path& filename,
                    const double scale_factor = 1.0) const;
};

//! global profiler object
Profiler& profiler()
{
  return Profiler::instance();
}

} // namespace Common
} // namespace Stuff
} // namespace Dune

#define DSC_PROFILER Dune::Stuff::Common::profiler()

#include "profiler.cc"

#endif // DUNE_STUFF_PROFILER_HH_INCLUDED
/** Copyright (c) 2012, Rene Milk
   * All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions are met:
   *
   * 1. Redistributions of source code must retain the above copyright notice, this
   *    list of conditions and the following disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright notice,
   *    this list of conditions and the following disclaimer in the documentation
   *    and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   *
   * The views and conclusions contained in the software and documentation are those
   * of the authors and should not be interpreted as representing official policies,
   * either expressed or implied, of the FreeBSD Project.
   **/