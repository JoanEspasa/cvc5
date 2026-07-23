/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The theory of planning.
 */

#include "theory/plan/theory_plan.h"

#include "theory/theory_model.h"
#include "util/statistics_registry.h"

namespace cvc5::internal {
namespace theory {
namespace plan {

TheoryPlan::TheoryPlan(Env& env, OutputChannel& out, Valuation valuation)
    : Theory(THEORY_PLAN, env, out, valuation),
      d_rewriter(nodeManager()),
      d_state(env, valuation),
      d_im(env, *this, d_state, getStatsPrefix(THEORY_PLAN))
{
  // Theory::check() requires an official theory state object; the inference
  // manager is what any future propagation or conflict will go through.
  d_theoryState = &d_state;
  d_inferManager = &d_im;
}

TheoryPlan::~TheoryPlan() {}

TheoryRewriter* TheoryPlan::getTheoryRewriter() { return &d_rewriter; }

ProofRuleChecker* TheoryPlan::getProofChecker() { return nullptr; }

bool TheoryPlan::needsEqualityEngine(CVC5_UNUSED EeSetupInfo& esi)
{
  return false;
}

bool TheoryPlan::preNotifyFact(CVC5_UNUSED TNode atom,
                               CVC5_UNUSED bool pol,
                               CVC5_UNUSED TNode fact,
                               CVC5_UNUSED bool isPrereg,
                               CVC5_UNUSED bool isInternal)
{
  // Inert by design at this stage: accept the fact and do nothing with it.
  // Returning true also stops Theory::check from reaching notifyFact, which
  // asserts a non-null equality engine.
  return true;
}

bool TheoryPlan::collectModelValues(TheoryModel* m,
                                    CVC5_UNUSED const std::set<Node>& termSet)
{
  // d_facts is a context-dependent list, so it still holds every fact asserted
  // at the current level -- the same iteration TheoryEngine::printAssertions
  // uses (theory_engine.cpp:345-347).
  for (context::CDList<Assertion>::const_iterator it = facts_begin(),
                                                  end = facts_end();
       it != end;
       ++it)
  {
    TNode fact = (*it).d_assertion;
    bool pol = fact.getKind() != Kind::NOT;
    TNode atom = pol ? fact : fact[0];
    if (!m->assertPredicate(atom, pol))
    {
      return false;
    }
  }
  return true;
}

}  // namespace plan
}  // namespace theory
}  // namespace cvc5::internal
