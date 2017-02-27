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
#include <cstdint>

namespace GameSolver { namespace Connect4 {
  /** 
   * A class storing a Connect 4 position.
   * Functions are relative to the current player to play.
   * Position containing aligment are not supported by this class.
   *
   * A binary bitboard representationis used.
   * Each column is encoded on HEIGH+1 bits.
	 * 
   * Example of bit order to encode for a 7x6 board
	 * .  .  .  .  .  .  .
	 * 5 12 19 26 33 40 47
	 * 4 11 18 25 32 39 46
	 * 3 10 17 24 31 38 45
	 * 2  9 16 23 30 37 44
	 * 1  8 15 22 29 36 43
	 * 0  7 14 21 28 35 42 
	 * 
   * Position is stored as
   * - a bitboard "mask" with 1 on any color stones
   * - a bitboard "current_player" with 1 on stones of current player
   *
   * "current_player" bitboard can be transformed into a compact and non ambiguous key
   * by adding an extra bit on top of the last non empty cell of each column.
   * This allow to identify all the empty cells whithout needing "mask" bitboard
   *
	 * current_player "x" = 1, opponent "o" = 0
	 * board     position  mask      key       bottom
	 *           0000000   0000000   0000000   0000000
	 * .......   0000000   0000000   0001000   0000000
	 * ...o...   0000000   0001000   0010000   0000000
	 * ..xx...   0011000   0011000   0011000   0000000
	 * ..ox...   0001000   0011000   0001100   0000000
	 * ..oox..   0000100   0011100   0000110   0000000
	 * ..oxxo.   0001100   0011110   1101101   1111111
	 *
	 * current_player "o" = 1, opponent "x" = 0
	 * board     position  mask      key       bottom
	 *           0000000   0000000   0001000   0000000
	 * ...x...   0000000   0001000   0000000   0000000
	 * ...o...   0001000   0001000   0011000   0000000
	 * ..xx...   0000000   0011000   0000000   0000000
	 * ..ox...   0010000   0011000   0010100   0000000
	 * ..oox..   0011000   0011100   0011010   0000000
	 * ..oxxo.   0010010   0011110   1110011   1111111
	 *
	 * key is an unique representation of a board key = position + mask + bottom
	 * in practice, as bottom is constant, key = position + mask is also a 
   * non-ambigous representation of the position.
	 */
  class Position {
    public:

      static const int WIDTH = 7;  // width of the board
      static const int HEIGHT = 6; // height of the board
      static_assert(WIDTH < 10, "Board's width must be less than 10");
      static_assert(WIDTH*(HEIGHT+1) <= 64, "Board does not fit in 64bits bitboard");

      /**
       * Indicates whether a column is playable.
       * @param col: 0-based index of column to play
       * @return true if the column is playable, false if the column is already full.
       */
      bool canPlay(int col) const 
      {
        return (mask & top_mask(col)) == 0;
      }

      /**
       * Plays a playable column.
       * This function should not be called on a non-playable column or a column making an alignment.
       *
       * @param col: 0-based index of a playable column.
       */
      void play(int col) 
      {
        current_position ^= mask;
        mask |= mask + bottom_mask(col);
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
        uint64_t pos = current_position; 
        pos |= (mask + bottom_mask(col)) & column_mask(col);
        return alignment(pos);
      }

      /**    
       * @return number of moves played from the beginning of the game.
       */
      unsigned int nbMoves() const
      {
        return moves;
      }

      /**    
       * @return a compact representation of a position on WIDTH*(HEIGHT+1) bits.
       */
      uint64_t key() const 
      {
        return current_position + mask;
      }

      /**
       * Default constructor, build an empty position.
       */
      Position() : current_position{0}, mask{0}, moves{0} {}



    private:
      uint64_t current_position;
      uint64_t mask;
      unsigned int moves; // number of moves played since the beinning of the game.

      /**
       * Test an alignment for current player (identified by one in the bitboard pos)
       * @param a bitboard position of a player's cells.
       * @return true if the player has a 4-alignment.
       */
      static bool alignment(uint64_t pos) {
        // horizontal 
        uint64_t m = pos & (pos >> (HEIGHT+1));
        if(m & (m >> (2*(HEIGHT+1)))) return true;

        // diagonal 1
        m = pos & (pos >> HEIGHT);
        if(m & (m >> (2*HEIGHT))) return true;

        // diagonal 2 
        m = pos & (pos >> (HEIGHT+2));
        if(m & (m >> (2*(HEIGHT+2)))) return true;

        // vertical;
        m = pos & (pos >> 1);
        if(m & (m >> 2)) return true;

        return false;
      }

      // return a bitmask containg a single 1 corresponding to the top cel of a given column
      static uint64_t top_mask(int col) {
        return (UINT64_C(1) << (HEIGHT - 1)) << col*(HEIGHT+1);
      }

      // return a bitmask containg a single 1 corresponding to the bottom cell of a given column
      static uint64_t bottom_mask(int col) {
        return UINT64_C(1) << col*(HEIGHT+1);
      }

      // return a bitmask 1 on all the cells of a given column
      static uint64_t column_mask(int col) {
        return ((UINT64_C(1) << HEIGHT)-1) << col*(HEIGHT+1);
      }

  };

}} // end namespaces

#endif
