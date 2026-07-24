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
#include "util/plan_index.h"

namespace cvc5::internal {
namespace theory {
namespace plan {

namespace {

/** The declared sort of a planning entity, from its operator payload. */
TypeNode sortOf(TNode n)
{
  return n.getOperator().getConst<PlanIndex>().getSort();
}

}  // namespace

TypeNode PlanEntityTypeRule::preComputeType(CVC5_UNUSED NodeManager* nm, TNode n)
{
  return sortOf(n);
}

TypeNode PlanEntityTypeRule::computeType(CVC5_UNUSED NodeManager* nodeManager,
                                         TNode n,
                                         bool check,
                                         std::ostream* errOut)
{
  const Kind k = n.getKind();
  Assert(k == Kind::PLAN_DOES || k == Kind::PLAN_FLUENT || k == Kind::PLAN_AUX);
  const TypeNode sort = sortOf(n);
  // Only PLAN_FLUENT may be non-Boolean: an action and an auxiliary are
  // propositions by construction, and a mis-sorted payload would otherwise
  // produce a non-Boolean "atom" that no theory could assert. The payload is
  // supplied from outside (TermManager::mkOp), so this is a type error to be
  // reported rather than an internal invariant to assert: an Assert would
  // compile away in a production build and let the malformed term through.
  if (check && k != Kind::PLAN_FLUENT && !sort.isBoolean())
  {
    if (errOut)
    {
      (*errOut) << "Operator " << k << " expects a Boolean sort in its index. "
                << "Found '" << sort << "'. Only PLAN_FLUENT may be "
                << "non-Boolean.";
    }
    return TypeNode::null();
  }
  return sort;
}

}  // namespace plan
}  // namespace theory
}  // namespace cvc5::internal
