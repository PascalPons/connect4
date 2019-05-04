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

#ifndef MOVE_SORTER_HPP
#define MOVE_SORTER_HPP

#include "Position.hpp"

namespace GameSolver {
namespace Connect4 {

/**
 * This class helps sorting the next moves
 *
 * You have to add moves first with their score
 * then you can get them back in decreasing score
 *
 * This class implement an insertion sort that is in practice very
 * efficient for small number of move to sort (max is Position::WIDTH)
 * and also efficient if the move are pushed in approximatively increasing
 * order which can be acheived by using a simpler column ordering heuristic.
 */
class MoveSorter {
 public:

  /**
   * Add a move in the container with its score.
   * You cannot add more than Position::WIDTH moves
   */
  void add(const Position::position_t move, const int score) {
    int pos = size++;
    for(; pos && entries[pos - 1].score > score; --pos) entries[pos] = entries[pos - 1];
    entries[pos].move = move;
    entries[pos].score = score;
  }

  /**
   * Get next move
   * @return next remaining move with max score and remove it from the container.
   * If no more move is available return 0
   */
  Position::position_t getNext() {
    if(size)
      return entries[--size].move;
    else
      return 0;
  }

  /**
   * reset (empty) the container
   */
  void reset() {
    size = 0;
  }

  /**
   * Build an empty container
   */
  MoveSorter(): size{0} {
  }

 private:
  // number of stored moves
  unsigned int size;

  // Contains size moves with their score ordered by score
  struct {
    Position::position_t move;
    int score;
  } entries[Position::WIDTH];
};

} // namespace Connect4
} // namespace GameSolver
#endif
