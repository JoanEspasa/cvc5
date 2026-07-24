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

#include <cvc5/cvc5_plan.h>

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "smt/env.h"
#include "theory/inference_id.h"
#include "theory/theory_model.h"
#include "util/plan_index.h"

namespace cvc5::internal {
namespace theory {
namespace plan {

TheoryPlan::TheoryPlan(Env& env, OutputChannel& out, Valuation valuation)
    : Theory(THEORY_PLAN, env, out, valuation),
      d_rewriter(nodeManager()),
      d_state(env, valuation),
      d_im(env, *this, d_state, getStatsPrefix(THEORY_PLAN)),
      d_activeActions(context())
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

bool TheoryPlan::preNotifyFact(TNode atom,
                               bool pol,
                               CVC5_UNUSED TNode fact,
                               CVC5_UNUSED bool isPrereg,
                               CVC5_UNUSED bool isInternal)
{
  // The oracle's presence is what enables planning reasoning at all: without
  // one there is no verdict to be had, so the theory stays entirely out of the
  // way and costs nothing.
  cvc5::PlanInterferenceOracle* oracle = d_env.getPlanInterferenceOracle();
  if (oracle != nullptr && pol && atom.getKind() == Kind::PLAN_DOES)
  {
    const PlanIndex& pi = atom.getOperator().getConst<PlanIndex>();
    const uint32_t action = pi.getIndex();
    const uint32_t timestep = pi.getTimestep();

    d_activeActions.push_back(ActiveAction{action, timestep, atom});

    // Only actions sharing a timestep constrain each other: interference is a
    // statement about executing actions *together*.
    std::vector<uint32_t> coactive;
    std::unordered_map<uint32_t, Node> atoms;
    for (const ActiveAction& active : d_activeActions)
    {
      if (active.d_timestep != timestep)
      {
        continue;
      }
      if (atoms.emplace(active.d_action, active.d_atom).second)
      {
        coactive.push_back(active.d_action);
      }
    }

    // Every literal put into a conflict below is currently true, so the
    // conjunction is precisely the assignment being refuted. Such a conflict
    // is its own explanation, which is why this theory needs no explain():
    // nothing is propagated whose reason could be asked for later.
    if (d_env.getPlanSemantics() == cvc5::PlanSemantics::ForallStep)
    {
      uint32_t other = 0;
      if (findInterferingPartner(action, coactive, *oracle, other))
      {
        d_im.conflict(
            nodeManager()->mkNode(Kind::AND, {atoms.at(action), atoms.at(other)}),
            InferenceId::PLAN_FORALL_MUTEX);
      }
    }
    else
    {
      std::vector<uint32_t> cycle;
      if (coactive.size() >= 2 && findInterferenceCycle(coactive, *oracle, cycle))
      {
        std::vector<Node> literals;
        literals.reserve(cycle.size());
        for (uint32_t a : cycle)
        {
          literals.push_back(atoms.at(a));
        }
        d_im.conflict(nodeManager()->mkNode(Kind::AND, literals),
                      InferenceId::PLAN_EXISTS_CYCLE);
      }
    }
  }
  // Returning true also stops Theory::check from reaching notifyFact, which
  // asserts a non-null equality engine.
  return true;
}

bool TheoryPlan::findInterferenceCycle(const std::vector<uint32_t>& actions,
                                       cvc5::PlanInterferenceOracle& oracle,
                                       std::vector<uint32_t>& cycle) const
{
  // Textbook white/grey/black DFS. The only twist is that the edges are not
  // stored anywhere: each one is discovered by asking the oracle, which is the
  // whole point of deciding interference lazily instead of building the graph.
  std::unordered_set<uint32_t> visited;
  std::unordered_set<uint32_t> onPath;
  std::vector<uint32_t> path;

  std::function<bool(uint32_t)> visit = [&](uint32_t current) -> bool {
    visited.insert(current);
    onPath.insert(current);
    path.push_back(current);
    for (uint32_t other : actions)
    {
      // Directional: interferes(x, y) and interferes(y, x) are separate
      // questions, and a pair answering yes to both is a two-element cycle.
      if (other == current || !oracle.interferes(current, other))
      {
        continue;
      }
      if (onPath.find(other) != onPath.end())
      {
        // `other` is still on the current path, so the path from it onwards
        // closes a cycle.
        auto start = std::find(path.begin(), path.end(), other);
        cycle.assign(start, path.end());
        return true;
      }
      if (visited.find(other) == visited.end() && visit(other))
      {
        return true;
      }
    }
    onPath.erase(current);
    path.pop_back();
    return false;
  };

  for (uint32_t start : actions)
  {
    if (visited.find(start) == visited.end() && visit(start))
    {
      return true;
    }
  }
  return false;
}

bool TheoryPlan::findInterferingPartner(uint32_t action,
                                        const std::vector<uint32_t>& actions,
                                        cvc5::PlanInterferenceOracle& oracle,
                                        uint32_t& other) const
{
  for (uint32_t candidate : actions)
  {
    if (candidate == action)
    {
      continue;
    }
    // Either direction suffices: forall-step requires *every* order to be
    // valid, so one-way interference already rules the pair out -- which is
    // exactly where it is stricter than exists-step.
    if (oracle.interferes(action, candidate)
        || oracle.interferes(candidate, action))
    {
      other = candidate;
      return true;
    }
  }
  return false;
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
