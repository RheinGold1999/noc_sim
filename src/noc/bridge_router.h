#ifndef __BRIDGE_ROUTER_H__
#define __BRIDGE_ROUTER_H__

#include "noc/router.h"
#include "noc/data_type.h"

template <typename T>
class StreamPortOut;

template <typename T>
class StreamPortIn;

template <typename T>
class FIFO;

class BridgeRouter :
  public Router
{
public:
  StreamPortOut<Flit*>** loc_inj_o;
  StreamPortIn<Flit*>** loc_eje_i;

  StreamPortOut<Flit*>** glb_inj_o;
  StreamPortIn<Flit*>** glb_eje_i;

public:
  BridgeRouter(
    const ModelBase* parent,
    const std::string& name,
    const Coord& coord
  );
  ~BridgeRouter();

  void transfer() override;
  void process() override;
  void update() override;

  bool is_loc2glb(const Flit* flit);
  bool is_glb2loc(const Flit* flit);

  void check_addr(const NodeAddr& addr);
  NodeAddr get_addr() const override;
  void set_addr(const NodeAddr& addr);

private:
  FIFO<Flit*>** m_loc2glb_que;
  FIFO<Flit*>** m_glb2loc_que;

  Flit** m_loc_arb_flits;
  Flit** m_glb_arb_flits;

  /**
   * @brief The addr works like network gateway
   * @example If the BridgeRouter addr is 1.3.x, a local flit 
   *          with dst_addr 1.2.3 will be passed to the global 
   *          ring, and a global flit with dst_addr 1.3.2 will 
   *          be passed to the local ring.
   * @note The addr should NOT start with x, it should be like
   *       3.x.x.
   */
  NodeAddr m_addr;
  /** @brief Only for the convenience during the network connection.
   *         The real addr for routing is m_addr;
   */
  Coord m_coord;  // only for the convenience in netword connection
};

#endif /* __BRIDGE_ROUTER_H__ */