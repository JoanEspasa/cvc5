/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The index payload carried by planning atoms and terms.
 */

#include "util/plan_index.h"

#include <ostream>

#include "expr/type_node.h"

namespace cvc5::internal {

PlanIndex::PlanIndex(uint32_t index, uint32_t timestep, TypeNode sort)
    : d_index(index),
      d_timestep(timestep),
      d_sort(new TypeNode(sort))
{
}

PlanIndex::PlanIndex(const PlanIndex& other)
    : d_index(other.d_index),
      d_timestep(other.d_timestep),
      d_sort(new TypeNode(other.getSort()))
{
}

PlanIndex& PlanIndex::operator=(const PlanIndex& other)
{
  d_index = other.d_index;
  d_timestep = other.d_timestep;
  (*d_sort) = other.getSort();
  return *this;
}

PlanIndex::~PlanIndex() {}

TypeNode PlanIndex::getSort() const { return *d_sort.get(); }

bool PlanIndex::operator==(const PlanIndex& other) const
{
  return d_index == other.d_index && d_timestep == other.d_timestep
         && getSort() == other.getSort();
}

bool PlanIndex::operator!=(const PlanIndex& other) const
{
  return !(*this == other);
}

size_t PlanIndexHashFunction::operator()(const PlanIndex& pi) const
{
  // index and timestep pack losslessly into 64 bits; the sort is mixed in
  // afterwards so that two entities differing only by sort hash apart.
  uint64_t k = (static_cast<uint64_t>(pi.getIndex()) << 32)
               | static_cast<uint64_t>(pi.getTimestep());
  return std::hash<uint64_t>()(k) ^ std::hash<TypeNode>()(pi.getSort());
}

std::ostream& operator<<(std::ostream& os, const PlanIndex& pi)
{
  return os << pi.getIndex() << "@" << pi.getTimestep() << ":" << pi.getSort();
}

}  // namespace cvc5::internal
