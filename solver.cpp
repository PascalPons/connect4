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

#include <cassert>
#include "position.hpp"

using namespace GameSolver::Connect4;

namespace GameSolver { namespace Connect4 {

  /*
   * A class to solve Connect 4 position using Nagemax variant of min-max algorithm.
   */
  class Solver {
    private:
    unsigned long long nodeCount; // counter of explored nodes.

    /**
     * Reccursively score connect 4 position using negamax variant of alpha-beta algorithm.
     * @param: alpha < beta, a score window within which we are evaluating the position.
     *
     * @return the exact score, an upper or lower bound score depending of the case:
     * - if true score of position <= alpha then true score <= return value <= alpha
     * - if true score of position >= beta then beta <= return value <= true score
     * - if alpha <= true score <= beta then return value = true score
     */
    int negamax(const Position &P, int alpha, int beta) {
      assert(alpha < beta);
      nodeCount++; // increment counter of explored nodes

      if(P.nbMoves() == Position::WIDTH*Position::HEIGHT) // check for draw game
        return 0; 

      for(int x = 0; x < Position::WIDTH; x++) // check if current player can win next move
        if(P.canPlay(x) && P.isWinningMove(x)) 
          return (Position::WIDTH*Position::HEIGHT+1 - P.nbMoves())/2;

      int max = (Position::WIDTH*Position::HEIGHT-1 - P.nbMoves())/2;	// upper bound of our score as we cannot win immediately
      if(beta > max) {
        beta = max;                     // there is no need to keep beta above our max possible score.
        if(alpha >= beta) return beta;  // prune the exploration if the [alpha;beta] window is empty.
      }

      for(int x = 0; x < Position::WIDTH; x++) // compute the score of all possible next move and keep the best one
        if(P.canPlay(x)) {
          Position P2(P);
          P2.play(x);               // It's opponent turn in P2 position after current player plays x column.
          int score = -negamax(P2, -beta, -alpha); // explore opponent's score within [-beta;-alpha] windows:
                                              // no need to have good precision for score better than beta (opponent's score worse than -beta)
                                              // no need to check for score worse than alpha (opponent's score worse better than -alpha)

          if(score >= beta) return score;  // prune the exploration if we find a possible move better than what we were looking for.
          if(score > alpha) alpha = score; // reduce the [alpha;beta] window for next exploration, as we only 
                                           // need to search for a position that is better than the best so far.
        }

      return alpha;
    }

    public:

    int solve(const Position &P, bool weak = false) 
    {
      nodeCount = 0;
      if(weak) 
        return negamax(P, -1, 1);
      else 
        return negamax(P, -Position::WIDTH*Position::HEIGHT/2, Position::WIDTH*Position::HEIGHT/2);
    }

    unsigned long long getNodeCount() 
    {
      return nodeCount;
    }

  };


}} // namespace GameSolver::Connect4


/*
 * Get micro-second precision timestamp
 * uses unix gettimeofday function
 */
#include <sys/time.h>
unsigned long long getTimeMicrosec() {
  timeval NOW;
  gettimeofday(&NOW, NULL);
  return NOW.tv_sec*1000000LL + NOW.tv_usec;    
}

/*
 * Main function.
 * Reads Connect 4 positions, line by line, from standard input 
 * and writes one line per position to standard output containing:
 *  - score of the position
 *  - number of nodes explored
 *  - time spent in microsecond to solve the position.
 *
 *  Any invalid position (invalid sequence of move, or already won game) 
 *  will generate an error message to standard error and an empty line to standard output.
 */
#include <iostream>
int main(int argc, char** argv) {

  Solver solver;

  bool weak = false;
  if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'w') weak = true;

  std::string line;
  
  for(int l = 1; std::getline(std::cin, line); l++) {
    Position P;
    if(P.play(line) != line.size())
    {
      std::cerr << "Line << " << l << ": Invalid move " << (P.nbMoves()+1) << " \"" << line << "\"" << std::endl;
    }
    else
    {
      unsigned long long start_time = getTimeMicrosec();
      int score = solver.solve(P, weak);
      unsigned long long end_time = getTimeMicrosec();
      std::cout << line << " " << score << " " << solver.getNodeCount() << " " << (end_time - start_time);
    }
    std::cout << std::endl;
  }
}


