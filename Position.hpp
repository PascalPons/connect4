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

#ifndef POSITION_HPP
#define POSITION_HPP

#include <string>
#include <cstdint>
#include <cassert>

namespace GameSolver {
namespace Connect4 {
/**
 * A class storing a Connect 4 position.
 * Functions are relative to the current player to play.
 * Position containing alignment are not supported by this class.
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
  
/**
 * Generate a bitmask containing one for the bottom slot of each colum
 * must be defined outside of the class definition to be available at compile time for bottom_mask
 */


class Position {
 public:
  static constexpr int WIDTH = 7;  // width of the board
  static constexpr int HEIGHT = 6; // height of the board

  // Board size is 64bits or 128 bits depending on WIDTH and HEIGHT
  using position_t = typename std::conditional < WIDTH * (HEIGHT + 1) <= 64, uint64_t, __int128>::type;
  // __int128 is a g++ non portable type. Use the following line limited to 64bits board for C++ compatibility
  // using position_t = uint64_t;

  static constexpr int MIN_SCORE = -(WIDTH*HEIGHT) / 2 + 3;
  static constexpr int MAX_SCORE = (WIDTH * HEIGHT + 1) / 2 - 3;

  static_assert(WIDTH < 10, "Board's width must be less than 10");
  static_assert(WIDTH * (HEIGHT + 1) <= sizeof(position_t)*8, "Board does not fit into position_t bitmask");

  /**
   * Plays a possible move given by its bitmap representation
   *
   * @param move: a possible move given by its bitmap representation
   *        only one bit of the bitmap should be set to 1
   *        the move should be a valid possible move for the current player
   */
  void play(position_t move) {
    current_position ^= mask;
    mask |= move;
    moves++;
  }

  /*
   * Plays a sequence of successive played columns, mainly used to initilize a board.
   * @param seq: a sequence of digits corresponding to the 1-based index of the column played.
   *
   * @return number of played moves. Processing will stop at first invalid move that can be:
   *           - invalid character (non digit, or digit >= WIDTH)
   *           - playing a colum the is already full
   *           - playing a column that makes an alignment (we only solve non).
   *         Caller can check if the move sequence was valid by comparing the number of
   *         processed moves to the length of the sequence.
   */
  unsigned int play(const std::string &seq) {
    for(unsigned int i = 0; i < seq.size(); i++) {
      int col = seq[i] - '1';
      if(col < 0 || col >= Position::WIDTH || !canPlay(col) || isWinningMove(col)) return i; // invalid move
      playCol(col);
    }
    return seq.size();
  }

  /**
   * return true if current player can win next move
   */
  bool canWinNext() const {
    return winning_position() & possible();
  }


  /**
   * @return number of moves played from the beginning of the game.
   */
  int nbMoves() const {
    return moves;
  }

  /**
   * @return a compact representation of a position on WIDTH*(HEIGHT+1) bits.
   */
  position_t key() const {
    return current_position + mask;
  }

  /**
  * Build a symetric base 3 key. Two symetric positions will have the same key.
  *
  * This key is a base 3 representation of the sequence of played moves column per column,
  * from bottom to top. The 3 digits are top_of_colum(0), current_player(1), opponent(2).
  *
  * example: game "45" where player one played colum 4, then player two played column 5
  * has a representation in base 3 digits : 0 0 0 1 0 2 0 0 0 or : 3*3^3 + 1*3^5
  *
  * The symetric key is the mimimum key of the two keys built iterating columns from left to righ or right to left.
  *
  * as the last digit is always 0, we omit it and a base 3 key
  * uses N = (nbMoves + nbColums - 1) base 3 digits or N*log2(3) bits.
  */
  uint64_t key3() const {
    uint64_t key_forward = 0;
    for(int i = 0; i < Position::WIDTH; i++) partialKey3(key_forward, i);  // compute key in increasing order of columns

    uint64_t key_reverse = 0;
    for(int i = Position::WIDTH; i--;) partialKey3(key_reverse, i);  // compute key in decreasing order of columns

    return key_forward < key_reverse ? key_forward / 3 : key_reverse / 3; // take the smallest key and divide per 3 as the last base3 digit is always 0
  }

  /**
   * Return a bitmap of all the possible next moves the do not lose in one turn.
   * A losing move is a move leaving the possibility for the opponent to win directly.
   *
   * Warning this function is intended to test position where you cannot win in one turn
   * If you have a winning move, this function can miss it and prefer to prevent the opponent
   * to make an alignment.
   */
  position_t possibleNonLosingMoves() const {
    assert(!canWinNext());
    position_t possible_mask = possible();
    position_t opponent_win = opponent_winning_position();
    position_t forced_moves = possible_mask & opponent_win;
    if(forced_moves) {
      if(forced_moves & (forced_moves - 1)) // check if there is more than one forced move
        return 0;                           // the opponnent has two winning moves and you cannot stop him
      else possible_mask = forced_moves;    // enforce to play the single forced move
    }
    return possible_mask & ~(opponent_win >> 1);  // avoid to play below an opponent winning spot
  }

  /**
   * Score a possible move.
   *
   * @param move, a possible move given in a bitmap format.
   *
   * The score we are using is the number of winning spots
   * the current player has after playing the move.
   */
  int moveScore(position_t move) const {
    return popcount(compute_winning_position(current_position | move, mask));
  }

  /**
   * Default constructor, build an empty position.
   */
  Position() : current_position{0}, mask{0}, moves{0} {}

  /**
   * Indicates whether a column is playable.
   * @param col: 0-based index of column to play
   * @return true if the column is playable, false if the column is already full.
   */
  bool canPlay(int col) const {
    return (mask & top_mask_col(col)) == 0;
  }

  /**
   * Plays a playable column.
   * This function should not be called on a non-playable column or a column making an alignment.
   *
   * @param col: 0-based index of a playable column.
   */
  void playCol(int col) {
    play((mask + bottom_mask_col(col)) & column_mask(col));
  }

  /**
   * Indicates whether the current player wins by playing a given column.
   * This function should never be called on a non-playable column.
   * @param col: 0-based index of a playable column.
   * @return true if current player makes an alignment by playing the corresponding column col.
   */
  bool isWinningMove(int col) const {
    return winning_position() & possible() & column_mask(col);
  }

 private:
  position_t current_position; // bitmap of the current_player stones
  position_t mask;             // bitmap of all the already palyed spots
  unsigned int moves;        // number of moves played since the beinning of the game.

  /**
    * Compute a partial base 3 key for a given column
    */
  void partialKey3(uint64_t &key, int col) const {
    for(position_t pos = UINT64_C(1) << (col * (Position::HEIGHT + 1)); pos & mask; pos <<= 1) {
      key *= 3;
      if(pos & current_position) key += 1;
      else key += 2;
    }
    key *= 3;
  }

  /**
   * Return a bitmask of the possible winning positions for the current player
   */
  position_t winning_position() const {
    return compute_winning_position(current_position, mask);
  }

  /**
   * Return a bitmask of the possible winning positions for the opponent
   */
  position_t opponent_winning_position() const {
    return compute_winning_position(current_position ^ mask, mask);
  }

  /**
   * Bitmap of the next possible valid moves for the current player
   * Including losing moves.
   */
  position_t possible() const {
    return (mask + bottom_mask) & board_mask;
  }

  /**
   * counts number of bit set to one in a 64bits integer
   */
  static unsigned int popcount(position_t m) {
    unsigned int c = 0;
    for(c = 0; m; c++) m &= m - 1;
    return c;
  }

  /**
   * @parmam position, a bitmap of the player to evaluate the winning pos
   * @param mask, a mask of the already played spots
   *
   * @return a bitmap of all the winning free spots making an alignment
   */
  static position_t compute_winning_position(position_t position, position_t mask) {
    // vertical;
    position_t r = (position << 1) & (position << 2) & (position << 3);

    //horizontal
    position_t p = (position << (HEIGHT + 1)) & (position << 2 * (HEIGHT + 1));
    r |= p & (position << 3 * (HEIGHT + 1));
    r |= p & (position >> (HEIGHT + 1));
    p = (position >> (HEIGHT + 1)) & (position >> 2 * (HEIGHT + 1));
    r |= p & (position << (HEIGHT + 1));
    r |= p & (position >> 3 * (HEIGHT + 1));

    //diagonal 1
    p = (position << HEIGHT) & (position << 2 * HEIGHT);
    r |= p & (position << 3 * HEIGHT);
    r |= p & (position >> HEIGHT);
    p = (position >> HEIGHT) & (position >> 2 * HEIGHT);
    r |= p & (position << HEIGHT);
    r |= p & (position >> 3 * HEIGHT);

    //diagonal 2
    p = (position << (HEIGHT + 2)) & (position << 2 * (HEIGHT + 2));
    r |= p & (position << 3 * (HEIGHT + 2));
    r |= p & (position >> (HEIGHT + 2));
    p = (position >> (HEIGHT + 2)) & (position >> 2 * (HEIGHT + 2));
    r |= p & (position << (HEIGHT + 2));
    r |= p & (position >> 3 * (HEIGHT + 2));

    return r & (board_mask ^ mask);
  }

  // Static bitmaps
  template<int width, int height> struct bottom {static constexpr position_t mask = bottom<width-1, height>::mask | position_t(1) << (width - 1) * (height + 1);};
  template <int height> struct bottom<0, height> {static constexpr position_t mask = 0;};

  static constexpr position_t bottom_mask = bottom<WIDTH, HEIGHT>::mask;
  static constexpr position_t board_mask = bottom_mask * ((1LL << HEIGHT) - 1);

  // return a bitmask containg a single 1 corresponding to the top cel of a given column
  static constexpr position_t top_mask_col(int col) {
    return UINT64_C(1) << ((HEIGHT - 1) + col * (HEIGHT + 1));
  }

  // return a bitmask containg a single 1 corresponding to the bottom cell of a given column
  static constexpr position_t bottom_mask_col(int col) {
    return UINT64_C(1) << col * (HEIGHT + 1);
  }

 public:
  // return a bitmask 1 on all the cells of a given column
  static constexpr position_t column_mask(int col) {
    return ((UINT64_C(1) << HEIGHT) - 1) << col * (HEIGHT + 1);
  }
};

} // namespace Connect4
} // namespace GameSolver
#endif
