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

#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include <cstring>

namespace GameSolver {
namespace Connect4 {

/**
 * util functions to compute next prime at compile time
 */
constexpr uint64_t med(uint64_t min, uint64_t max) {
  return (min + max) / 2;
}
/**
 * tells if an integer n has a a divisor between min (inclusive) and max (exclusive)
 */
constexpr bool has_factor(uint64_t n, uint64_t min, uint64_t max) {
  return min * min > n ? false : // do not search for factor above sqrt(n)
         min + 1 >= max ? n % min == 0 :
         has_factor(n, min, med(min, max)) || has_factor(n, med(min, max), max);
}

// return next prime number greater or equal to n.
// n must be >= 2
constexpr uint64_t next_prime(uint64_t n) {
  return has_factor(n, 2, n) ? next_prime(n + 1) : n;
}

// log2(1) = 0; log2(2) = 1; log2(3) = 1; log2(4) = 2; log2(8) = 3
constexpr unsigned int log2(unsigned int n) {
  return n <= 1 ? 0 : log2(n / 2) + 1;
}

/**
 * Abstrac interface for the Transposition Table get function
 */
template<class key_t, class value_t>
class TableGetter {
 private:
  virtual void* getKeys() = 0;
  virtual void* getValues() = 0;
  virtual size_t getSize() = 0;
  virtual int getKeySize() = 0;
  virtual int getValueSize() = 0;

 public:
  virtual value_t get(key_t key) const = 0;
  virtual ~TableGetter() {};

 friend class OpeningBook;
};

// uint_t<S> is a template type providing an unsigned int able to fit interger of S bits.
// uint_t<8> = uint8_t and uint_t<9> = uint_16t
template<int S> using uint_t =
  typename std::conditional < S <= 8, uint_least8_t,
  typename std::conditional < S <= 16, uint_least16_t,
  typename std::conditional<S <= 32, uint_least32_t,
  uint_least64_t>::type>::type >::type;

/**
 * Transposition Table is a simple hash map with fixed storage size.
 * In case of collision we keep the last entry and overide the previous one.
 * We keep only part of the key to reduce storage, but no error is possible thanks to Chinese theorem.
 *
 * The number of stored entries is a power of two that is defined at compile time.
 * We also define size of the entries and keys to allow optimization at compile time.
 *
 * key_size:   number of bits of the key
 * value_size: number of bits of the value
 * log_size:   base 2 log of the size of the Transposition Table.
 *             The table will contain 2^log_size elements
 */
template<class partial_key_t, class key_t, class value_t, int log_size>
class TranspositionTable : public TableGetter<key_t, value_t> {
 private:
  static const size_t size = next_prime(1 << log_size); // size of the transition table. Have to be odd to be prime with 2^sizeof(key_t)
  partial_key_t *K;     // Array to store truncated version of keys;
  value_t *V;   // Array to store values;

  void* getKeys()    override {return K;}
  void* getValues()  override {return V;}
  size_t getSize()   override {return size;}
  int getKeySize()   override {return sizeof(partial_key_t);}
  int getValueSize() override {return sizeof(value_t);}

  size_t index(key_t key) const {
    return key % size;
  }

 public:
  TranspositionTable() {
    K = new partial_key_t[size];
    V = new value_t[size];
    reset();
  }

  ~TranspositionTable() {
    delete[] K;
    delete[] V;
  }

  /**
   * Empty the Transition Table.
   */
  void reset() { // fill everything with 0, because 0 value means missing data
    memset(K, 0, size * sizeof(partial_key_t));
    memset(V, 0, size * sizeof(value_t));
  }

  /**
   * Store a value for a given key
   * @param key: must be less than key_size bits.
   * @param value: must be less than value_size bits. null (0) value is used to encode missing data
   */
  void put(key_t key, value_t value) {
    size_t pos = index(key);
    K[pos] = key; // key is possibly trucated as key_t is possibly less than key_size bits.
    V[pos] = value;
  }

  /**
   * Get the value of a key
   * @param key: must be less than key_size bits.
   * @return value_size bits value associated with the key if present, 0 otherwise.
   */
  value_t get(key_t key) const override {
    size_t pos = index(key);
    if(K[pos] == (partial_key_t)key) return V[pos]; // need to cast to key_t because key may be truncated due to size of key_t
    else return 0;
  }
};

} // namespace Connect4
} // namespace GameSolver
#endif
