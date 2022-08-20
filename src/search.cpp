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

#include "search.h"
#include "timeman.h"
#include "tt.h"
#include "eval.h"
#include "uci.h"

Ply selectiveDepth = 0;

Score quiescence(Position &pos, Score alpha, Score beta, Ply ply) {

    if (shouldEnd()) return UNKNOWN_SCORE;

    if (ply > selectiveDepth)
        selectiveDepth = ply;

    Score staticEval = eval(pos);

    if (staticEval >= beta) {
        return beta;
    }

    if (staticEval > alpha) {
        alpha = staticEval;
    }

    MoveList moves = {pos, ply, true};

    while (!moves.empty()) {

        Move m = moves.nextMove();

        // Delta pruning
        if (m.isPromo() * PIECE_VALUES[QUEEN].mg + PIECE_VALUES[pos.pieceAt(m.getTo()).type].mg +
            staticEval + DELTA_MARGIN < alpha)
            continue;

        pos.makeMove(m);

        Score score = -quiescence(pos, -beta, -alpha, ply + 1);

        pos.undoMove(m);

        if (shouldEnd()) return UNKNOWN_SCORE;

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

Score search(Position &pos, SearchState *state, Depth depth, Score alpha, Score beta, Ply ply) {

    if (shouldEnd()) return UNKNOWN_SCORE;

    if (pos.getMove50() >= 4 && ply > 0 && pos.isRepetition()) return DRAW_VALUE;

    Score ttScore = ttProbe(pos.getHash(), depth, alpha, beta);
    if (ttScore != UNKNOWN_SCORE) return ttScore;

    if (depth <= 0) return quiescence(pos, alpha, beta, ply);

    Color color = pos.getSideToMove();

    MoveList moves = {pos, ply, false};
    bool inCheck = bool(getAttackers(pos, pos.pieces<KING>(color).lsb()));
    if (moves.count == 0) {
        if (inCheck) {
            return -MATE_VALUE + ply;
        } else {
            return DRAW_VALUE;
        }
    }

    bool pvNode = beta - alpha > 1;

    Score staticEval = state->eval = eval(pos);

    // Razoring
    if (depth == 1 && !pvNode && !inCheck && staticEval + RAZOR_MARGIN < alpha) {
        return quiescence(pos, alpha, beta, ply);
    }

    // Reverse futility pruning
    if (depth <= RFP_DEPTH && !inCheck && staticEval - RFP_DEPTH_MULTIPLIER * (int)depth >= beta && std::abs(beta) < MATE_VALUE - 100)
        return beta;

    // Null move pruning
    if (!pvNode && ply > 0 && !inCheck && !(state-1)->move.isNull() && depth >= NULL_MOVE_DEPTH && staticEval >= beta) {
        // We don't want to make a null move in a Zugzwang position
        if (pos.pieces<KNIGHT>(color) | pos.pieces<BISHOP>(color) | pos.pieces<ROOK>(color) | pos.pieces<QUEEN>(color)) {
            state->move = Move();
            pos.makeNullMove();
            Score score = -search(pos, state+1, depth - NULL_MOVE_REDUCTION, -beta, -beta + 1, ply + 1);
            pos.undoNullMove();

            if (score >= beta) {
                if (std::abs(score) > MATE_VALUE - 100) return beta;
                return score;
            }
        }
    }

    Move bestMove;
    EntryFlag ttFlag = ALPHA;
    bool searchPv = true;

    while (!moves.empty()) {

        Move m = moves.nextMove();
        state->move = m;

        Score score;

        pos.makeMove(m);

        if (searchPv)
            score = -search(pos, state+1, depth - 1, -beta, -alpha, ply + 1);
        else {
            score = -search(pos, state+1, depth - 1, -alpha - 1, -alpha, ply + 1);

            if (score > alpha && score < beta) {
                score = -search(pos, state+1, depth - 1, -beta, -alpha, ply + 1);
            }
        }

        pos.undoMove(m);

        if (shouldEnd()) return UNKNOWN_SCORE;

        if (score >= beta) {

            if (m.isQuiet()) {
                recordKillerMove(m, ply);
                recordHHMove(m, color, depth);
            }

            ttSave(pos.getHash(), depth, beta, BETA, m);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = m;
            ttFlag = EXACT;
        }

        searchPv = false;
    }

    ttSave(pos.getHash(), depth, alpha, ttFlag, bestMove);

    return alpha;
}

std::string getPvLine(Position &pos) {
    Move m = getHashMove(pos.getHash());
    if (!pos.isRepetition() && m) {
        pos.makeMove(m);
        std::string str = m.str() + " " + getPvLine(pos);
        pos.undoMove(m);
        return str;
    } else {
        return "";
    }
}

Score searchRoot(Position &pos, Depth depth, bool uci) {

    clearTables();
    selectiveDepth = 0;
    SearchState stateStack[400];

    Score score = search(pos, stateStack+1, depth, -INF_SCORE, INF_SCORE, 0);

    if (score == UNKNOWN_SCORE) return UNKNOWN_SCORE;

    std::string pvLine = getPvLine(pos);
    if (uci) {
        Score absScore = std::abs(score);
        int mateDepth = MATE_VALUE - absScore;
        std::string scoreStr = "cp " + std::to_string(score);

        if (mateDepth <= 64) {
            int matePly;
            // We are giving the mate
            if (score > 0) {
                matePly = mateDepth / 2 + 1;

            } else {
                matePly = -(mateDepth / 2);
            }
            scoreStr = "mate " + std::to_string(matePly);
        }

        out("info", "depth", depth, "seldepth", selectiveDepth, "nodes", nodeCount, "score", scoreStr, "time",
            getSearchTime(), "nps", getNps(), "pv", pvLine);
    }


    return score;
}

void iterativeDeepening(Position pos, Depth depth, bool uci) {
    Move bestMove;

    for (Depth currDepth = 1; currDepth <= depth; currDepth++) {
        Score score = searchRoot(pos, currDepth, uci);
        if (score == UNKNOWN_SCORE) break;
        bestMove = getHashMove(pos.getHash());
    }

    globalAge++;

    if (uci)
        out("bestmove", bestMove);
}