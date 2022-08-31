//     BlackCore is a UCI Chess engine
//     Copyright (c) 2022 SzilBalazs
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef BLACKCORE_EVAL_H
#define BLACKCORE_EVAL_H

#include "constants.h"
#include "position.h"

Score eval(const Position &pos);

struct Value {
    Score mg = 0;
    Score eg = 0;

    inline Value operator+(Value a) const { return {mg + a.mg, eg + a.eg}; }

    inline Value operator-(Value a) const { return {mg - a.mg, eg - a.eg}; }

    inline Value operator*(int a) const { return {mg * a, eg * a}; }

    inline Value &operator+=(Value a) {
        mg += a.mg;
        eg += a.eg;
        return *this;
    }

    inline Value &operator+=(Score a) {
        mg += a;
        eg += a;
        return *this;
    }
};

#ifdef TUNE

extern Value PIECE_VALUES[6];

extern Score TEMPO_SCORE;

extern Value PAWN_PASSED_BONUS;
extern Value PAWN_DOUBLE_PENALTY;
extern Value PAWN_ISOLATED_PENALTY;

extern Value KNIGHT_MOBILITY;

extern Value BISHOP_ATTACK_BONUS;

extern Value ROOK_MOBILITY;
extern Value ROOK_TRAPPED;
extern Value ROOK_OPEN_BONUS;
extern Value ROOK_HALF_BONUS;

extern Value KING_UNSAFE;
extern Value KING_SHIELD_1;
extern Value KING_SHIELD_2;

#else

constexpr Value PIECE_VALUES[6] = {{0,    0},
                                   {98,   136},
                                   {394,  378},
                                   {450,  530},
                                   {647,  821},
                                   {1300, 1700}};


constexpr Score TEMPO_SCORE = 10;

constexpr Value PAWN_PASSED_BONUS = {28, 58};
constexpr Value PAWN_DOUBLE_PENALTY = {-4, -21};
constexpr Value PAWN_ISOLATED_PENALTY = {-13, -28};

constexpr Value KNIGHT_MOBILITY = {11, 10};

constexpr Value BISHOP_ATTACK_BONUS = {15, 5};

constexpr Value ROOK_MOBILITY = {7, 2};
constexpr Value ROOK_TRAPPED = {-70, -25};
constexpr Value ROOK_OPEN_BONUS = {34, 19};
constexpr Value ROOK_HALF_BONUS = {17, 21};

constexpr Value KING_UNSAFE = {-70, 0};
constexpr Value KING_SHIELD_1 = {25, 0};
constexpr Value KING_SHIELD_2 = {15, 0};

#endif

constexpr Value bPawnTable[64] = {
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {20,  50},
        {30,  55},
        {30,  60},
        {30,  60},
        {30,  60},
        {30,  60},
        {30,  55},
        {20,  50},
        {5,   20},
        {10,  25},
        {10,  30},
        {10,  30},
        {10,  30},
        {10,  30},
        {10,  25},
        {5,   20},
        {0,   15},
        {0,   20},
        {0,   20},
        {20,  20},
        {20,  20},
        {0,   20},
        {0,   20},
        {0,   15},
        {0,   5},
        {0,   10},
        {0,   10},
        {30,  10},
        {30,  10},
        {0,   10},
        {0,   10},
        {0,   5},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {-20, 0},
        {-20, 0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0}};

constexpr Value wPawnTable[64] = {
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {-20, 0},
        {-20, 0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   5},
        {0,   10},
        {0,   10},
        {30,  10},
        {30,  10},
        {0,   10},
        {0,   10},
        {0,   5},
        {0,   15},
        {0,   20},
        {0,   20},
        {20,  20},
        {20,  20},
        {0,   20},
        {0,   20},
        {0,   15},
        {5,   20},
        {10,  25},
        {10,  30},
        {10,  30},
        {10,  30},
        {10,  30},
        {10,  25},
        {5,   20},
        {20,  50},
        {30,  55},
        {30,  60},
        {30,  60},
        {30,  60},
        {30,  60},
        {30,  55},
        {20,  50},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0},
        {0,   0}};

constexpr Score egKingTable[64] = {-50, -40, -30, -30, -30, -30, -40, -50,
                                   -40, -20, 5, 5, 5, 5, -20, -40,
                                   -30, 5, 10, 15, 15, 10, 5, -30,
                                   -30, 5, 15, 20, 20, 15, 5, -30,
                                   -30, 5, 15, 20, 20, 15, 5, -30,
                                   -30, 5, 10, 15, 15, 10, 5, -30,
                                   -40, -20, 5, 5, 5, 5, -20, -40,
                                   -50, -40, -30, -30, -30, -30, -40, -50};

#endif //BLACKCORE_EVAL_H
