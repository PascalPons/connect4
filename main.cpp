/*
 * This file is part of Connect4 Game Solver <http://connect4.gamesolver.org>
 * Copyright (C) 2017-2019 Pascal Pons <contact@gamesolver.org>
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

#include "Solver.hpp"
#include <iostream>

using namespace GameSolver::Connect4;

/**
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
int main(int argc, char** argv) {
  Solver solver;
  bool weak = false;
  bool analyze = false;

  std::string opening_book = "7x6.book";
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      if(argv[i][1] == 'w') weak = true; // parameter -w: use weak solver
      else if(argv[i][1] == 'b') { // paramater -b: define an alternative opening book
        if(++i < argc) opening_book = std::string(argv[i]);
      }
      else if(argv[i][1] == 'a') { // paramater -a: make an analysis of all possible moves
        analyze = true;
      }
    }
  }
  solver.loadBook(opening_book);

  std::string line;

  for(int l = 1; std::getline(std::cin, line); l++) {
    Position P;
    if(P.play(line) != line.size()) {
      std::cerr << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << std::endl;
    } else {
      std::cout << line;
      if(analyze) {
        std::vector<int> scores = solver.analyze(P, weak);
        for(int i = 0; i < Position::WIDTH; i++) std::cout << " " << scores[i];
      }
      else {
        int score = solver.solve(P, weak);
        std::cout << " " << score;
      }
      std::cout << std::endl;
    }
  }
}
