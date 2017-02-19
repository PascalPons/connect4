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

#ifndef POSITION_HPP
#define POSITION_HPP

#include <string>

namespace GameSolver { namespace Connect4 {

  /** 
   * A class storing a Connect 4 position.
   * Functions are relative to the current player to play.
   * Position containing aligment are not supported by this class.
   */
  class Position {
    public:

      static const int WIDTH = 7;  // width of the board
      static const int HEIGHT = 6; // height of the board
      static_assert(WIDTH < 10, "Board's width must be less than 10");

      /**
       * Indicates whether a column is playable.
       * @param col: 0-based index of column to play
       * @return true if the column is playable, false if the column is already full.
       */
      bool canPlay(int col) const 
      {
        return height[col] < HEIGHT;
      }

      /**
       * Plays a playable column.
       * This function should not be called on a non-playable column or a column making an alignment.
       *
       * @param col: 0-based index of a playable column.
       */
      void play(int col) 
      {
        board[col][height[col]] = 1 + moves%2;
        height[col]++;
        moves++;
      }

      /*
       * Plays a sequence of successive played columns, mainly used to initilize a board.
       * @param seq: a sequence of digits corresponding to the 1-based index of the column played.
       *
       * @return number of played moves. Processing will stop at first invalid move that can be:
       *           - invalid character (non digit, or digit >= WIDTH)
       *           - playing a colum the is already full
       *           - playing a column that makes an aligment (we only solve non).
       *         Caller can check if the move sequence was valid by comparing the number of 
       *         processed moves to the length of the sequence.
       */
      unsigned int play(std::string seq) 
      {
        for(unsigned int i = 0; i < seq.size(); i++) {
          int col = seq[i] - '1'; 
          if(col < 0 || col >= Position::WIDTH || !canPlay(col) || isWinningMove(col)) return i; // invalid move
          play(col);
        }
        return seq.size();
      }

      /**
       * Indicates whether the current player wins by playing a given column.
       * This function should never be called on a non-playable column.
       * @param col: 0-based index of a playable column.
       * @return true if current player makes an alignment by playing the corresponding column col.
       */
      bool isWinningMove(int col) const 
      {
        int current_player = 1 + moves%2;
        // check for vertical alignments
        if(height[col] >= 3 
            && board[col][height[col]-1] == current_player 
            && board[col][height[col]-2] == current_player 
            && board[col][height[col]-3] == current_player) 
          return true;

        for(int dy = -1; dy <=1; dy++) {    // Iterate on horizontal (dy = 0) or two diagonal directions (dy = -1 or dy = 1).
          int nb = 0;                       // counter of the number of stones of current player surronding the played stone in tested direction.
          for(int dx = -1; dx <=1; dx += 2) // count continuous stones of current player on the left, then right of the played column.
            for(int x = col+dx, y = height[col]+dx*dy; x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && board[x][y] == current_player; nb++) {
              x += dx;
              y += dx*dy;
            }
          if(nb >= 3) return true; // there is an aligment if at least 3 other stones of the current user 
                                   // are surronding the played stone in the tested direction.
        }
        return false;
      }

      /**    
       * @return number of moves played from the beginning of the game.
       */
      unsigned int nbMoves() const
      {
        return moves;
      }

      /*
       * Default constructor, build an empty position.
       */
      Position() : board{0}, height{0}, moves{0} {}



    private:
      int board[WIDTH][HEIGHT]; // 0 if cell is empty, 1 for first player and 2 for second player.
      int height[WIDTH];        // number of stones per column
      unsigned int moves;       // number of moves played since the beinning of the game.
  };
  
}} // end namespaces

#endif
