#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include <cstdint>

class Simulator
{
public:
  static uint64_t curr_tick();
  static void set_stop();

  /**
   * @brief implements `elaborate`, `simulate` and `finalize`.
   * @param `cyc = 0` means no cycle count limit, otherwise
   *        the simulation will be stopped when reaching `cyc`.
   * @example
   *        Simulator::run(1000) will simulates for 1000 cycles.
   */
  static void run(uint64_t cyc = 0);

  static void elaborate();
  static void simulate(uint64_t cyc);
  static void finalize();

// private:
//   static void check_ports_connection();
//   static void update_logger_level();

private:
  static uint64_t s_tick;
  static bool s_stop;
};

#endif  /* __SIMULATOR_H__ */
