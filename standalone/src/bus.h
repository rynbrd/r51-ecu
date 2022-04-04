#ifndef __R51_BUS_H__
#define __R51_BUS_H__

#include <Canny.h>
#include <Caster.h>

typedef Caster::Bus<Canny::Frame> Bus;
typedef Caster::Node<Canny::Frame> Node;
typedef Caster::Broadcast<Canny::Frame> Broadcast;

#endif  // __R51_BUS_H__
