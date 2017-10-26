/*
 * This file is part of Connect4 Game Solver <http://connect4.gamesolver.org>
 * Copyright (C) 2007 Pascal Pons <contact@gamesolver.org>
 *
 * Connect4 Game Solver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Connect4 Game Solver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Connect4 Game Solver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "position.hpp"
#include "TranspositionTable.hpp"

namespace GameSolver { namespace Connect4 {

  class Solver {
  private:
    unsigned long long nodeCount; // counter of explored nodes.

    int columnOrder[Position::WIDTH]; // column exploration order

    TranspositionTable<Position::WIDTH*(Position::HEIGHT+1),                    // nb bits key
                      log2(Position::MAX_SCORE - Position::MIN_SCORE + 1) + 2,  // nb bits value
                      23> transTable;                                           // log2 of size of transposition table

    /**
     * Reccursively score connect 4 position using negamax variant of alpha-beta algorithm.
     * @param: position to evaluate, this function assumes nobody already won and 
     *         current player cannot win next move. This has to be checked before
     * @param: alpha < beta, a score window within which we are evaluating the position.
     *
     * @return the exact score, an upper or lower bound score depending of the case:
     * - if actual score of position <= alpha then actual score <= return value <= alpha
     * - if actual score of position >= beta then beta <= return value <= actual score
     * - if alpha <= actual score <= beta then return value = actual score
     */
    int negamax(const Position &P, int alpha, int beta);

  public:

    int solve(const Position &P, bool weak = false);

    unsigned long long getNodeCount() 
    {
      return nodeCount;
    }

    void reset() 
    {
      nodeCount = 0;
      transTable.reset();
    }

    // Constructor
    Solver();
  };

}} // end namespaces

#endif

