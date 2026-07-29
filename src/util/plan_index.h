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

#include "cvc5_public.h"

#ifndef CVC5__UTIL__PLAN_INDEX_H
#define CVC5__UTIL__PLAN_INDEX_H

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>

namespace cvc5::internal {

class TypeNode;

/**
 * The payload of a PLAN_*_OP: which entity, at which timestep, of which sort.
 *
 * A planning encoding names every entity by a pair -- (action, timestep),
 * (fluent, timestep) -- so THEORY_PLAN owns them all uniformly:
 *
 *   PLAN_DOES(a, t)    Bool                 action `a` executed at `t`
 *   PLAN_FLUENT(f, t)  any sort             the state of fluent `f` at `t`
 *   PLAN_AUX(k, t)     Bool                 encoder-private auxiliary
 *
 * **The sort is a full TypeNode, not an enumeration.** A planning entity is
 * nullary -- its whole content is the operator index -- so its sort cannot be
 * derived from children and has to be carried here. Carrying an enum would
 * limit fluents to Bool/Int/Real; carrying a TypeNode lets a fluent be
 * datatype-sorted (a finite object universe), set-sorted or array-sorted, which
 * is what makes planning *modulo theories* expressible at all. The same device
 * is used by AscriptionType (expr/ascription_type.h) -- "a way to coerce a Type
 * into the expression tree".
 *
 * Carrying the id and timestep as an *operator index* rather than as child terms is
 * likewise deliberate: children of Integer type would be owned by THEORY_ARITH,
 * and `PreRegisterVisitor::preRegisterWithTheory` throws a LogicException for a
 * subterm whose theory is not enabled. Integer children would therefore force
 * arithmetic into the logic of every planning problem -- including purely
 * propositional ones. As an operator index, the pair stays inside THEORY_PLAN.
 *
 * `d_entityId` is opaque to cvc5: its meaning is fixed by the encoder that emitted
 * the entity (a ground action id for PLAN_DOES, a ground fluent id for
 * PLAN_FLUENT, an encoder-private tag for PLAN_AUX).
 *
 * NOTE the sort must be Boolean for every kind except PLAN_FLUENT; the type
 * rule enforces it.
 */
class PlanIndex
{
 public:
  PlanIndex(uint32_t entityId, uint32_t timestep, TypeNode sort);
  PlanIndex(const PlanIndex& other);
  PlanIndex& operator=(const PlanIndex& other);
  ~PlanIndex();

  /** Which action / fluent / auxiliary proposition. */
  uint32_t getEntityId() const { return d_entityId; }
  /** The timestep it is indexed at. */
  uint32_t getTimestep() const { return d_timestep; }
  /** The sort of the entity. */
  TypeNode getSort() const;

  bool operator==(const PlanIndex& other) const;
  bool operator!=(const PlanIndex& other) const;

 private:
  uint32_t d_entityId;
  uint32_t d_timestep;
  std::unique_ptr<TypeNode> d_sort;
}; /* class PlanIndex */

/**
 * Hash function for PlanIndex.
 */
struct PlanIndexHashFunction
{
  size_t operator()(const PlanIndex& pi) const;
}; /* struct PlanIndexHashFunction */

std::ostream& operator<<(std::ostream& os, const PlanIndex& pi);

}  // namespace cvc5::internal

#endif /* CVC5__UTIL__PLAN_INDEX_H */
