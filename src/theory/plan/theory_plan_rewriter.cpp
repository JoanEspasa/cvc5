/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Rewriter for the theory of planning.
 */

#include "theory/plan/theory_plan_rewriter.h"

namespace cvc5::internal {
namespace theory {
namespace plan {

TheoryPlanRewriter::TheoryPlanRewriter(NodeManager* nm) : TheoryRewriter(nm) {}

}  // namespace plan
}  // namespace theory
}  // namespace cvc5::internal
