/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The callback interface through which THEORY_PLAN obtains problem-level
 * planning knowledge that the formula does not carry.
 */

#include <cvc5/cvc5_export.h>

#ifndef CVC5__API__CVC5_PLAN_H
#define CVC5__API__CVC5_PLAN_H

#include <cstdint>

namespace cvc5 {

/**
 * The parallel-step semantics THEORY_PLAN enforces on the actions sharing a
 * timestep.
 *
 * Both are decided from the same interference relation and differ only in what
 * they demand of it, which is why the relation is supplied once and the
 * semantics chosen separately.
 */
enum class PlanSemantics
{
  /**
   * The actions must be executable in *some* order. That holds exactly when
   * the interference relation restricted to them is acyclic: interference in
   * one direction only constrains the order, and a cycle is what leaves no
   * order at all.
   */
  ExistsStep,
  /**
   * The actions must be executable in *any* order, so interference between two
   * of them in either direction already forbids sharing the step.
   */
  ForallStep
};

/**
 * Answers interference queries for THEORY_PLAN.
 *
 * Two actions *interfere* if executing them in the same parallel step would be
 * unsound -- one's effects would clobber the other's precondition or effect, or
 * otherwise change what the other observes. Deciding whether a set of actions
 * may share a timestep is what THEORY_PLAN does, and it needs that verdict per
 * pair.
 *
 * The verdict is a property of the *planning problem*, not of the asserted
 * formula, so it is supplied from outside rather than derived in the solver:
 * the embedder registers an implementation with
 * Solver::setPlanInterferenceOracle and the theory calls it during search. This
 * keeps the encoding of actions -- preconditions, effects, conditional effects
 * -- out of the solver entirely.
 *
 * Contract:
 * - Arguments are PLAN_DOES payload indices, i.e. ground action ids. PlanIndex
 *   is shared with PLAN_FLUENT and PLAN_AUX, so "these are actions" is a
 *   contract of this interface, not something the type enforces.
 * - The verdict must be a **pure function of (a, b)**. The theory may call it at
 *   any point during search, in any order, repeatedly, and may cache it; it must
 *   not depend on the solver's current partial assignment.
 * - An implementation may run a *separate* cvc5 solver to compute its answer --
 *   that is the expected use, and the reason the verdict must be assignment
 *   independent. It must not call back into the solver that is asking.
 * - Non-owning: the oracle must outlive every check on the solver it is
 *   registered with.
 */
class CVC5_EXPORT PlanInterferenceOracle
{
 public:
  virtual ~PlanInterferenceOracle() = default;

  /**
   * Does action `a` interfere with action `b`?
   *
   * DIRECTIONAL, and not symmetric in general: this asks whether executing `a`
   * would prevent `b` from executing correctly, or corrupt what `b` reads or
   * writes. `interferes(a, b)` and `interferes(b, a)` are separate questions.
   *
   * @param a The index of the potentially interfering action.
   * @param b The index of the potentially interfered-with action.
   * @return True if `a` interferes with `b`.
   */
  virtual bool interferes(uint32_t a, uint32_t b) = 0;
};

}  // namespace cvc5

#endif /* CVC5__API__CVC5_PLAN_H */
