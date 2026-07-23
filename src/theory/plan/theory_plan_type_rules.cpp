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

#include "theory/plan/theory_plan_type_rules.h"

#include "base/check.h"
#include "expr/node_manager.h"
#include "util/plan_index.h"

namespace cvc5::internal {
namespace theory {
namespace plan {

namespace {

/** The declared sort of a planning entity, from its operator payload. */
TypeNode sortOf(CVC5_UNUSED NodeManager* nm, TNode n)
{
  return n.getOperator().getConst<PlanIndex>().getSort();
}

}  // namespace

TypeNode PlanEntityTypeRule::preComputeType(NodeManager* nm, TNode n)
{
  return sortOf(nm, n);
}

TypeNode PlanEntityTypeRule::computeType(NodeManager* nodeManager,
                                         TNode n,
                                         CVC5_UNUSED bool check,
                                         CVC5_UNUSED std::ostream* errOut)
{
  const Kind k = n.getKind();
  Assert(k == Kind::PLAN_DOES || k == Kind::PLAN_FLUENT || k == Kind::PLAN_AUX);
  // Only PLAN_FLUENT may be non-Boolean: an action and an auxiliary are
  // propositions by construction, and a mis-sorted payload would silently
  // produce a non-Boolean "atom" that no theory could assert.
  Assert(k == Kind::PLAN_FLUENT
         || n.getOperator().getConst<PlanIndex>().getSort().isBoolean())
      << "non-Boolean sort on a planning proposition";
  return sortOf(nodeManager, n);
}

}  // namespace plan
}  // namespace theory
}  // namespace cvc5::internal
