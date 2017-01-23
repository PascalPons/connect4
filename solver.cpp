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

#include "position.hpp"

using namespace GameSolver::Connect4;

namespace GameSolver { namespace Connect4 {

  /*
   * A class to solve Connect 4 position using Nagemax variant of min-max algorithm.
   */
  class Solver {
    private:
    unsigned long long nodeCount; // counter of explored nodes.

    /*
     * Recursively solve a connect 4 position using negamax variant of min-max algorithm.
     * @return the score of a position:
     *  - 0 for a draw game
     *  - positive score if you can win whatever your opponent is playing. Your score is
     *    the number of moves before the end you can win (the faster you win, the higher your score)
     *  - negative score if your opponent can force you to lose. Your score is the oposite of 
     *    the number of moves before the end you will lose (the faster you lose, the lower your score).
     */
    int negamax(const Position &P) {
      nodeCount++; // increment counter of explored nodes

      if(P.nbMoves() == Position::WIDTH*Position::HEIGHT) // check for draw game
        return 0; 

      for(int x = 0; x < Position::WIDTH; x++) // check if current player can win next move
        if(P.canPlay(x) && P.isWinningMove(x)) 
          return (Position::WIDTH*Position::HEIGHT+1 - P.nbMoves())/2;

      int bestScore = -Position::WIDTH*Position::HEIGHT; // init the best possible score with a lower bound of score.
      
      for(int x = 0; x < Position::WIDTH; x++) // compute the score of all possible next move and keep the best one
        if(P.canPlay(x)) {
          Position P2(P);
          P2.play(x);               // It's opponent turn in P2 position after current player plays x column.
          int score = -negamax(P2); // If current player plays col x, his score will be the opposite of opponent's score after playing col x
          if(score > bestScore) bestScore = score; // keep track of best possible score so far.
        }

      return bestScore;
    }

    public:

    int solve(const Position &P) 
    {
      nodeCount = 0;
      return negamax(P);
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
int main() {

  Solver solver;
  
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
      int score = solver.solve(P);
      unsigned long long end_time = getTimeMicrosec();
      std::cout << line << " " << score << " " << solver.getNodeCount() << " " << (end_time - start_time);
    }
    std::cout << std::endl;
  }
}


