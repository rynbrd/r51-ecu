#ifndef __R51_BUS_H__
#define __R51_BUS_H__

#include <Caster.h>
#include "frame.h"

typedef Caster::Bus<Frame> Bus;
typedef Caster::Node<Frame> Node;
typedef Caster::Broadcast<Frame> Broadcast;

#endif  // __R51_BUS_H__
