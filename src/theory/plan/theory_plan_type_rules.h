/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Type rules for the theory of planning.
 */

#include "cvc5_private.h"

#ifndef CVC5__THEORY__PLAN__THEORY_PLAN_TYPE_RULES_H
#define CVC5__THEORY__PLAN__THEORY_PLAN_TYPE_RULES_H

#include "expr/node.h"
#include "expr/type_node.h"

namespace cvc5::internal {
namespace theory {
namespace plan {

/**
 * One type rule for every PLAN_* kind.
 *
 * A planning entity is nullary -- its whole content is the operator index --
 * so its sort cannot be derived from children and is carried in the payload
 * instead. That is what lets a single rule serve the Boolean atoms
 * (PLAN_DOES / PLAN_AUX, and Boolean fluents) and the numeric fluent terms
 * uniformly -- one kind per role, sort orthogonal in the payload.
 */
struct PlanEntityTypeRule
{
  static TypeNode preComputeType(NodeManager* nm, TNode n);

  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

}  // namespace plan
}  // namespace theory
}  // namespace cvc5::internal

#endif /* CVC5__THEORY__PLAN__THEORY_PLAN_TYPE_RULES_H */
